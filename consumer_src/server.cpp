#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/un.h>         // sockaddr_un
#include <errno.h>          // strerror(errno)
#include <unistd.h>         // unlink, close
#include <cstring>          // c_str()

#include <cxxopts.hpp>
#include "transmission_header.hpp"
#include "simple_logger.hpp"

using namespace std;

// server global variables
static string socketPath;
static string logLevel;
static long long timeoutMs;

// server acts as consumer
class AFUnixConsumer {
private:
    string socketPath;
    string logLevel;

    int server_fd, client_fd;
    sockaddr_un addr;

public:
    AFUnixConsumer(string socketPath, string logLevel, int timeoutMs) : socketPath(socketPath), logLevel(logLevel) {
    }

    ~AFUnixConsumer() {
        close(server_fd);
        close(client_fd);
    }

    int BindSocket() {
        // setup connection
        server_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
        if (server_fd < 0) {
            Logger::error("socket init error: " + string(strerror(errno)));
            return server_fd;
        }

        this->addr = {};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);
        unlink(socketPath.c_str()); // deletes old socket path if it exists

        if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            Logger::error("socket bind error: " + string(strerror(errno)));
            return (1);
        }

        return 0;
    }

    int ListenOnSocket() {
        if (listen(server_fd, 1) < 0) {
            Logger::error("socket bind error: " + string(strerror(errno)));
            return server_fd;
        }

        client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            Logger::error("socket accept error: " + string(strerror(errno)));
            return client_fd;
        }

        return 0;
    }

    ssize_t RecvSocket(Payload_IMU_t* buffer) {
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(Payload_IMU_t), 0);

        if (bytes_received > 0) {
            Logger::info("Server received: " + to_string(bytes_received) + ", " + to_string(buffer->xAcc));
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
};

void cli_args(int argc, char* argv[]) {
    cxxopts::Options options(argv[0], "Server");

    options.add_options()
        ("socket-path", "Path to socket", cxxopts::value<std::string>()->default_value("/tmp/dummy_socket"))
        ("log-level", "Logging level", cxxopts::value<std::string>()->default_value("INFO"))
        ("timeout-ms", "Timeout in ms", cxxopts::value<long long>()->default_value("1000"))
        ("h,help", "Print help");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    // assign static global variables
    socketPath = result["socket-path"].as<std::string>();
    logLevel = result["log-level"].as<std::string>();
    timeoutMs = result["timeout-ms"].as<long long>();
}

int main(int argc, char* argv[]) {
    // set command line args
    cli_args(argc, argv);

    // set logger level, NOTE: all incorrect logger levels are
    // defaulted to INFO
    Logger::setLevel(logLevel);

    Logger::info("Socket path: " + socketPath);
    Logger::info("Log level: " + logLevel);
    Logger::info("Timeout ms: " + to_string(timeoutMs));

    // create server
    AFUnixConsumer afuc(socketPath, logLevel, timeoutMs);
    afuc.BindSocket();

    if (0 != afuc.ListenOnSocket()) {
        // error listening, exit program early.
        Logger::error("Error in ListenOnSocket, exiting.");
        exit(-1);
    }

    ssize_t status;
    bool has_read = true;
    chrono::_V2::steady_clock::time_point last_bad_read = chrono::steady_clock::now();
    Payload_IMU_t buffer;

    while (true) {
        // receive on socket
        status = afuc.RecvSocket(&buffer);
        if (status == 0) {
            Logger::error("Error in RecvSocket, exiting.");
            break;
        } else if (!has_read && status < 0) {
            // sets last bad read even if 
            last_bad_read = chrono::steady_clock::now();
            has_read = false;
        } else if (status > 0) {
            // success, read data
            has_read = true;

            // keep moving up on good reads
            last_bad_read = chrono::steady_clock::now();
        }

        // check if timeout has been exceeded
        long long time_diff = chrono::duration_cast<chrono::milliseconds>(
                chrono::_V2::steady_clock::now() - last_bad_read
        ).count();

        if (time_diff > timeoutMs) {
            Logger::error("Timeout exceeded, exiting");
            break;
        }
    }

    return 0;
}
