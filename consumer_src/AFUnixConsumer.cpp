#include <cstdlib>
#include <sys/socket.h>
#include <errno.h>          // strerror(errno)
#include <unistd.h>         // unlink, close
#include <cstring>          // c_str()

#include "AFUnixConsumer.hpp"
#include "transmission_header.hpp"
#include "simple_logger.hpp"

using namespace std;

AFUnixConsumer::AFUnixConsumer(string socketPath, string logLevel) : socketPath(socketPath), logLevel(logLevel) {
}

AFUnixConsumer::~AFUnixConsumer() {
    // can fail if not initialized, but will simply return an error code
    close(server_fd);
    close(client_fd);
}

int AFUnixConsumer::BindSocket() {
    // setup connection
    server_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (server_fd < 0) {
        Logger::error("socket init error: " + string(strerror(errno)));
        return SOCKET_ERROR;
    }

    this->addr = {};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, this->socketPath.c_str(), sizeof(addr.sun_path) - 1);
    unlink(this->socketPath.c_str()); // deletes old socket path if it exists

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        Logger::error("socket bind error: " + string(strerror(errno)));
        return SOCKET_ERROR;
    }

    return SOCKET_SUCCESS;
}

int AFUnixConsumer::ListenOnSocket() {
    if (listen(server_fd, 1) < 0) {
        Logger::error("socket bind error: " + string(strerror(errno)));
        return SOCKET_ERROR;
    }

    client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) {
        Logger::error("socket accept error: " + string(strerror(errno)));
        return SOCKET_ERROR;
    }

    return SOCKET_SUCCESS;
}

/* 
    * Note: this directly returns the recv error values, not my custom macros,
    * so that we can also get size
    */
ssize_t AFUnixConsumer::RecvSocket(Payload_IMU_t* buffer) {
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(Payload_IMU_t), 0);

    if (bytes_received > 0) {
        Logger::info("Server received: " + to_string(bytes_received));
        if (bytes_received != sizeof(Payload_IMU_t)) {
            // error reading, wrong number of bytes
            return -1;
        }
    } else if (bytes_received == 0) {
        // rare, signifies broken connection
        Logger::error("Client closed the connection.");
        return 0;
    } else {
        // common, includes timeouts and other trivial "errors"
        Logger::error("recv failed: " + string(strerror(errno)));
        if (errno == EPIPE || errno == ECONNRESET) {
            Logger::error("Broken pipe or connection reset.");
            return 0;
        }
    }

    return bytes_received;
}
