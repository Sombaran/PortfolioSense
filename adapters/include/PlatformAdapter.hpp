#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#endif

namespace folio::platform {

#ifdef _WIN32
using SocketHandle = SOCKET;
constexpr SocketHandle invalidSocket = INVALID_SOCKET;
#else
using SocketHandle = int;
constexpr SocketHandle invalidSocket = -1;
#endif

bool initializeNetworking(std::string& error);
SocketHandle createLoopbackServer(std::uint16_t port, std::string& error);
SocketHandle acceptConnection(SocketHandle server);
std::ptrdiff_t readSocket(SocketHandle socket, char* buffer, std::size_t size);
std::ptrdiff_t writeSocket(SocketHandle socket, const char* data, std::size_t size);
bool socketOperationInterrupted();
void setSocketTimeouts(SocketHandle socket, unsigned int seconds);
void closeSocket(SocketHandle socket);

std::string createSecureTemporaryFile(
    const std::string& prefix, const std::filesystem::path& directory = {});
void replaceFileAtomically(const std::string& temporary,
                           const std::string& destination);
std::filesystem::path executablePath();
int runPython(const std::string& script, const std::vector<std::string>& arguments,
              bool suppressOutput);
bool secureRandomBytes(std::size_t size, std::string& output);
void clearEnvironmentVariable(const std::string& name);
bool checkFilePermissions(const std::string& path, unsigned int expectedMode);

} // namespace folio::platform
