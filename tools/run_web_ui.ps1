param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectRoot,
    [Parameter(Mandatory = $true)]
    [string]$Executable,
    [Parameter(Mandatory = $true)]
    [string]$LogFile,
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ApplicationArguments
)

$ErrorActionPreference = 'Stop'

function Import-PortfolioEnvironmentFile {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return
    }
    $environmentFile = Get-Item -LiteralPath $Path -Force
    if ($environmentFile.Attributes -band [IO.FileAttributes]::ReparsePoint) {
        throw "Refusing to read environment file through a reparse point: $Path"
    }

    $allowedNames = @(
        'UPSTOX_API_KEY',
        'UPSTOX_API_SECRET',
        'UPSTOX_ACCESS_TOKEN',
        'FOLIO_LOGIN_CODE'
    )
    $lineNumber = 0
    foreach ($line in [IO.File]::ReadAllLines($Path)) {
        $lineNumber++
        $trimmed = $line.Trim()
        if (-not $trimmed -or $trimmed.StartsWith('#')) {
            continue
        }

        if ($trimmed -notmatch '^(?:export\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.*)$') {
            throw "Invalid environment assignment in $Path at line $lineNumber."
        }

        $name = $Matches[1]
        if ($name -notin $allowedNames -or
            -not [string]::IsNullOrEmpty([Environment]::GetEnvironmentVariable($name))) {
            continue
        }

        $value = $Matches[2].Trim()
        if ($value.Length -ge 2 -and
            (($value[0] -eq "'" -and $value[$value.Length - 1] -eq "'") -or
             ($value[0] -eq '"' -and $value[$value.Length - 1] -eq '"'))) {
            $value = $value.Substring(1, $value.Length - 2)
        }
        [Environment]::SetEnvironmentVariable($name, $value, 'Process')
    }
}

function ConvertTo-WindowsArgument {
    param([string]$Value)

    $quoted = [Text.StringBuilder]::new()
    [void]$quoted.Append('"')
    $backslashes = 0
    foreach ($character in $Value.ToCharArray()) {
        if ($character -eq '\') {
            $backslashes++
        }
        elseif ($character -eq '"') {
            [void]$quoted.Append(('\' * (2 * $backslashes + 1)))
            [void]$quoted.Append('"')
            $backslashes = 0
        }
        else {
            if ($backslashes) {
                [void]$quoted.Append(('\' * $backslashes))
                $backslashes = 0
            }
            [void]$quoted.Append($character)
        }
    }
    if ($backslashes) {
        [void]$quoted.Append(('\' * (2 * $backslashes)))
    }
    [void]$quoted.Append('"')
    return $quoted.ToString()
}

try {
    $ProjectRoot = [IO.Path]::GetFullPath($ProjectRoot)
    if (-not [IO.Path]::IsPathRooted($Executable)) {
        $Executable = Join-Path $ProjectRoot $Executable
    }
    $Executable = [IO.Path]::GetFullPath($Executable)
    $LogFile = [IO.Path]::GetFullPath($LogFile)
    $userProfile = [Environment]::GetFolderPath([Environment+SpecialFolder]::UserProfile)
    Import-PortfolioEnvironmentFile (Join-Path $userProfile '.upstox.env')

    $missing = @('UPSTOX_API_KEY', 'UPSTOX_API_SECRET', 'UPSTOX_ACCESS_TOKEN') |
        Where-Object { [string]::IsNullOrEmpty([Environment]::GetEnvironmentVariable($_)) }
    if ($missing) {
        throw "--web requires UPSTOX_API_KEY, UPSTOX_API_SECRET, and UPSTOX_ACCESS_TOKEN. Set them in the environment or in $([IO.Path]::Combine($userProfile, '.upstox.env'))."
    }
    if (-not (Test-Path -LiteralPath $Executable -PathType Leaf)) {
        throw "Portfolio Health executable was not found: $Executable"
    }

    $startInfo = [Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $Executable
    $startInfo.WorkingDirectory = $ProjectRoot
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $false
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true
    $startInfo.Arguments = [string]::Join(
        ' ',
        @($ApplicationArguments | ForEach-Object { ConvertTo-WindowsArgument $_ })
    )

    $logWriter = [IO.StreamWriter]::new($LogFile, $true, [Text.UTF8Encoding]::new($false))
    $logWriter.AutoFlush = $true
    $syncWriter = [System.IO.TextWriter]::Synchronized($logWriter)
    $process = [Diagnostics.Process]::new()
    $process.StartInfo = $startInfo
    $process.add_OutputDataReceived({
        if ($null -ne $EventArgs.Data) { $syncWriter.WriteLine($EventArgs.Data) }
    }.GetNewClosure())
    $process.add_ErrorDataReceived({
        if ($null -ne $EventArgs.Data) { $syncWriter.WriteLine($EventArgs.Data) }
    }.GetNewClosure())

    if (-not $process.Start()) {
        throw 'Could not start Portfolio Health.'
    }
    $process.BeginOutputReadLine()
    $process.BeginErrorReadLine()

    $deadline = [DateTime]::UtcNow.AddSeconds(20)
    $ready = $false
    while (-not $ready -and -not $process.HasExited -and [DateTime]::UtcNow -lt $deadline) {
        $client = [Net.Sockets.TcpClient]::new()
        try {
            $client.Connect('127.0.0.1', 8080)
            $ready = $true
        }
        catch [Net.Sockets.SocketException] {
            Start-Sleep -Milliseconds 250
        }
        finally {
            $client.Dispose()
        }
    }

    if ($ready -and -not $process.HasExited) {
        try {
            Start-Process 'http://localhost:8080/'
        }
        catch {
            Write-Warning "The dashboard is running at http://localhost:8080/, but Windows could not open a browser. Open the URL manually."
        }
    }
    elseif (-not $process.HasExited) {
        $process.Kill()
        $process.WaitForExit()
        throw "The web server did not start on 127.0.0.1:8080 within 20 seconds. Check the web log: $LogFile"
    }

    $process.WaitForExit()
    $process.WaitForExit()
    exit $process.ExitCode
}
catch {
    Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
finally {
    if ($null -ne $logWriter) {
        $logWriter.Dispose()
    }
}
