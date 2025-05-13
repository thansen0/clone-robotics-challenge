#include <cstring>      // for strerror, strncpy, strlen
#include <cerrno>       // for errno
#include <unistd.h>     // for close()
#include <sys/socket.h> // for socket(), connect(), send()

#include "AFUnixPublisher.hpp"

// client acts as publisher
AFUnixPublisher::AFUnixPublisher(string socketPath, string logLevel) : socketPath(socketPath), logLevel(logLevel) {
}

AFUnixPublisher::~AFUnixPublisher() {
    close(sock_fd);
}

int AFUnixPublisher::ConnectSocket() {
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (sock_fd < 0) {
        Logger::error("socket failed: " + string(strerror(errno)));
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        Logger::error("connect failed: " + string(strerror(errno)));
        return 1;
    }

    return 0;
}

int AFUnixPublisher::SendOnSocket(const string message) {
    if (send(sock_fd, message.c_str(), strlen(message.c_str()), 0) < 0) {
        Logger::error("socket send error: " + string(strerror(errno)));
        return 1;
    }

    return 0;
}

int AFUnixPublisher::SendOnSocket(const Payload_IMU_t* data) {
    if (send(sock_fd, (const void*)data, sizeof(*data), 0) < 0) {
        Logger::error("socket send error: " + string(strerror(errno)));
        return 1;
    }
    Logger::info("Sent " + to_string(sizeof(*data)) + " bytes");

    return 0;
}