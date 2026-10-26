#include "PlatformAdapter.hpp"

#include <windows.h>
#include <bcrypt.h>
#include <ws2tcpip.h>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <system_error>

namespace folio::platform {
namespace {

std::string windowsError(const char* operation, DWORD error = GetLastError());

std::wstring toWide(const std::string& value) {
    if (value.empty()) return {};
    const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                         value.data(), static_cast<int>(value.size()),
                                         nullptr, 0);
    if (size <= 0) throw std::system_error(
        static_cast<int>(GetLastError()), std::system_category(), "UTF-8 conversion failed");
    std::wstring result(static_cast<std::size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                        static_cast<int>(value.size()), result.data(), size);
    return result;
}

std::wstring quoteArgument(const std::wstring& argument) {
    std::wstring quoted = L"\"";
    std::size_t backslashes = 0;
    for (wchar_t character : argument) {
        if (character == L'\\') {
            ++backslashes;
        } else if (character == L'"') {
            quoted.append(backslashes * 2 + 1, L'\\');
            quoted.push_back(character);
            backslashes = 0;
        } else {
            quoted.append(backslashes, L'\\');
            backslashes = 0;
            quoted.push_back(character);
        }
    }
    quoted.append(backslashes * 2, L'\\');
    quoted.push_back(L'"');
    return quoted;
}

std::wstring findPythonExecutable() {
    const DWORD required = GetEnvironmentVariableW(L"PATH", nullptr, 0);
    if (required == 0) throw std::runtime_error(windowsError("reading PATH"));
    std::vector<wchar_t> pathBuffer(required);
    const DWORD length = GetEnvironmentVariableW(L"PATH", pathBuffer.data(), required);
    if (length == 0 || length >= required) {
        throw std::runtime_error(windowsError("reading PATH"));
    }

    const std::wstring path(pathBuffer.data(), length);
    std::size_t start = 0;
    while (start <= path.size()) {
        const std::size_t separator = path.find(L';', start);
        const std::wstring entry = path.substr(
            start, separator == std::wstring::npos ? std::wstring::npos : separator - start);
        if (!entry.empty() && std::filesystem::path(entry).is_absolute()) {
            const auto candidate = std::filesystem::path(entry) / L"python.exe";
            std::error_code error;
            if (std::filesystem::is_regular_file(candidate, error)) {
                return candidate.wstring();
            }
        }
        if (separator == std::wstring::npos) break;
        start = separator + 1;
    }
    throw std::runtime_error("python.exe was not found in absolute PATH entries");
}

std::string windowsError(const char* operation, DWORD error) {
    return std::string(operation) + " failed (Windows error " +
           std::to_string(error) + ")";
}

} // namespace

bool initializeNetworking(std::string& error) {
    WSADATA data{};
    const int result = WSAStartup(MAKEWORD(2, 2), &data);
    if (result == 0) return true;
    error = windowsError("WSAStartup", static_cast<DWORD>(result));
    return false;
}

SocketHandle createLoopbackServer(std::uint16_t port, std::string& error) {
    const SocketHandle server = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == invalidSocket) {
        error = windowsError("socket", static_cast<DWORD>(WSAGetLastError()));
        return invalidSocket;
    }
    BOOL reuse = TRUE;
    ::setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);
    if (::bind(server, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR ||
        ::listen(server, 8) == SOCKET_ERROR) {
        error = windowsError("bind/listen", static_cast<DWORD>(WSAGetLastError()));
        ::closesocket(server);
        return invalidSocket;
    }
    return server;
}

SocketHandle acceptConnection(SocketHandle server) {
    return ::accept(server, nullptr, nullptr);
}

std::ptrdiff_t readSocket(SocketHandle socket, char* buffer, std::size_t size) {
    const int boundedSize = static_cast<int>(
        std::min(size, static_cast<std::size_t>(std::numeric_limits<int>::max())));
    return ::recv(socket, buffer, boundedSize, 0);
}

std::ptrdiff_t writeSocket(SocketHandle socket, const char* data, std::size_t size) {
    const int boundedSize = static_cast<int>(
        std::min(size, static_cast<std::size_t>(std::numeric_limits<int>::max())));
    return ::send(socket, data, boundedSize, 0);
}

int waitForSocket(SocketHandle socket, bool waitForWrite, unsigned int timeoutMilliseconds) {
    fd_set readSet;
    fd_set writeSet;
    fd_set errorSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&errorSet);
    FD_SET(socket, waitForWrite ? &writeSet : &readSet);
    FD_SET(socket, &errorSet);

    timeval timeout{};
    timeout.tv_sec = static_cast<long>(timeoutMilliseconds / 1000);
    timeout.tv_usec = static_cast<long>((timeoutMilliseconds % 1000) * 1000);
    const int result = ::select(0, &readSet, &writeSet, &errorSet, &timeout);
    if (result == SOCKET_ERROR || FD_ISSET(socket, &errorSet)) return -1;
    return result == 0 ? 0 : 1;
}

bool socketOperationInterrupted() {
    return WSAGetLastError() == WSAEINTR;
}

void setSocketTimeouts(SocketHandle socket, unsigned int seconds) {
    const DWORD timeout = seconds * 1000;
    ::setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO,
                 reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    ::setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO,
                 reinterpret_cast<const char*>(&timeout), sizeof(timeout));
}

void closeSocket(SocketHandle socket) {
    if (socket != invalidSocket) ::closesocket(socket);
}

std::string createSecureTemporaryFile(const std::string& prefix,
                                      const std::filesystem::path& targetDirectory) {
    std::wstring directory;
    if (targetDirectory.empty()) {
        wchar_t buffer[MAX_PATH + 1]{};
        const DWORD length = GetTempPathW(MAX_PATH, buffer);
        if (length == 0 || length > MAX_PATH) {
            throw std::runtime_error(windowsError("GetTempPathW"));
        }
        directory.assign(buffer, length);
    } else {
        directory = toWide(targetDirectory.u8string());
        if (!directory.empty() && directory.back() != L'\\' && directory.back() != L'/') {
            directory.push_back(L'\\');
        }
    }
    std::string random;
    for (unsigned attempt = 0; attempt < 32; ++attempt) {
        if (!secureRandomBytes(16, random)) {
            throw std::runtime_error(windowsError("BCryptGenRandom"));
        }
        static constexpr wchar_t hex[] = L"0123456789abcdef";
        std::wstring name = toWide(prefix);
        for (unsigned char byte : random) {
            name.push_back(hex[byte >> 4]);
            name.push_back(hex[byte & 0x0f]);
        }
        const std::wstring fullPath = directory + name;
        HANDLE file = CreateFileW(fullPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                                  nullptr, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, nullptr);
        if (file != INVALID_HANDLE_VALUE) {
            CloseHandle(file);
            const int utf8Size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                fullPath.data(), static_cast<int>(fullPath.size()), nullptr, 0, nullptr, nullptr);
            if (utf8Size <= 0) {
                DeleteFileW(fullPath.c_str());
                throw std::runtime_error(windowsError("WideCharToMultiByte"));
            }
            std::string result(static_cast<std::size_t>(utf8Size), '\0');
            WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, fullPath.data(),
                static_cast<int>(fullPath.size()), result.data(), utf8Size, nullptr, nullptr);
            return result;
        }
        if (GetLastError() != ERROR_FILE_EXISTS) {
            throw std::runtime_error(windowsError("CreateFileW"));
        }
    }
    throw std::runtime_error("cannot create a unique secure temporary file");
}

void replaceFileAtomically(const std::string& temporary,
                           const std::string& destination) {
    if (!MoveFileExW(toWide(temporary).c_str(), toWide(destination).c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        throw std::runtime_error(windowsError("MoveFileExW"));
    }
}

std::filesystem::path executablePath() {
    std::wstring path(32768, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, path.data(),
                                             static_cast<DWORD>(path.size()));
    if (length == 0 || length >= path.size()) {
        throw std::runtime_error(windowsError("GetModuleFileNameW"));
    }
    path.resize(length);
    return std::filesystem::path(path);
}

int runPython(const std::string& script, const std::vector<std::string>& arguments,
              bool suppressOutput) {
    std::vector<std::wstring> values;
    values.reserve(arguments.size() + 2);
    values.push_back(findPythonExecutable());
    values.push_back(toWide(script));
    for (const auto& argument : arguments) values.push_back(toWide(argument));

    std::wstring commandLine;
    for (const auto& value : values) {
        if (!commandLine.empty()) commandLine.push_back(L' ');
        commandLine += quoteArgument(value);
    }
    std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
    mutableCommand.push_back(L'\0');

    STARTUPINFOEXW startup{};
    startup.StartupInfo.cb = sizeof(startup);
    HANDLE nullHandle = INVALID_HANDLE_VALUE;
    std::vector<unsigned char> attributeStorage;
    bool attributesInitialized = false;
    if (suppressOutput) {
        SECURITY_ATTRIBUTES securityAttributes{};
        securityAttributes.nLength = sizeof(securityAttributes);
        securityAttributes.bInheritHandle = TRUE;
        nullHandle = CreateFileW(L"NUL", GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 &securityAttributes, OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL, nullptr);
        if (nullHandle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error(windowsError("opening NUL"));
        }
        SIZE_T attributeSize = 0;
        InitializeProcThreadAttributeList(nullptr, 1, 0, &attributeSize);
        attributeStorage.resize(attributeSize);
        startup.lpAttributeList = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(
            attributeStorage.data());
        if (!InitializeProcThreadAttributeList(startup.lpAttributeList, 1, 0,
                                               &attributeSize)) {
            const DWORD error = GetLastError();
            CloseHandle(nullHandle);
            throw std::runtime_error(windowsError("InitializeProcThreadAttributeList", error));
        }
        attributesInitialized = true;
        HANDLE inheritedHandles[] = {nullHandle};
        if (!UpdateProcThreadAttribute(startup.lpAttributeList, 0,
                                       PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                                       inheritedHandles, sizeof(inheritedHandles),
                                       nullptr, nullptr)) {
            const DWORD error = GetLastError();
            DeleteProcThreadAttributeList(startup.lpAttributeList);
            CloseHandle(nullHandle);
            throw std::runtime_error(windowsError("UpdateProcThreadAttribute", error));
        }
        startup.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
        startup.StartupInfo.hStdInput = nullHandle;
        startup.StartupInfo.hStdOutput = nullHandle;
        startup.StartupInfo.hStdError = nullHandle;
    }

    PROCESS_INFORMATION process{};
    const DWORD creationFlags = CREATE_NO_WINDOW |
        (suppressOutput ? EXTENDED_STARTUPINFO_PRESENT : 0);
    const BOOL created = CreateProcessW(values.front().c_str(), mutableCommand.data(),
                                        nullptr, nullptr,
                                        suppressOutput, creationFlags, nullptr, nullptr,
                                        &startup.StartupInfo, &process);
    const DWORD createError = created ? ERROR_SUCCESS : GetLastError();
    if (attributesInitialized) DeleteProcThreadAttributeList(startup.lpAttributeList);
    if (nullHandle != INVALID_HANDLE_VALUE) CloseHandle(nullHandle);
    if (!created) throw std::runtime_error(windowsError("CreateProcessW", createError));

    if (WaitForSingleObject(process.hProcess, INFINITE) != WAIT_OBJECT_0) {
        const DWORD error = GetLastError();
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        throw std::runtime_error(windowsError("WaitForSingleObject", error));
    }
    DWORD exitCode = 1;
    const BOOL readExitCode = GetExitCodeProcess(process.hProcess, &exitCode);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    if (!readExitCode) throw std::runtime_error(windowsError("GetExitCodeProcess"));
    return static_cast<int>(exitCode);
}

bool secureRandomBytes(std::size_t size, std::string& output) {
    if (size > std::numeric_limits<ULONG>::max()) return false;
    output.assign(size, '\0');
    const NTSTATUS result = BCryptGenRandom(nullptr,
        reinterpret_cast<PUCHAR>(output.data()), static_cast<ULONG>(size),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (result >= 0) return true;
    output.clear();
    return false;
}

void clearEnvironmentVariable(const std::string& name) {
    _putenv_s(name.c_str(), "");
}

bool checkFilePermissions(const std::string&, unsigned int) {
    return true;
}

} // namespace folio::platform
