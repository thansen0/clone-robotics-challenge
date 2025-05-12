#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/un.h>         // sockaddr_un
#include <errno.h>          // strerror(errno)
#include <unistd.h>         // unlink, close
#include <cstring>          // c_str()

#include <cxxopts.hpp>
#include "transmission_header.hpp"

using namespace std;

// server global variables
static string socketPath;
static string logLevel;
static int timeoutMs;

// server acts as consumer
class AFUnixConsumer {
private:
    string socketPath;
    string logLevel;
    int timeoutMs;

    int server_fd, client_fd;
    sockaddr_un addr;

public:
    AFUnixConsumer(string socketPath, string logLevel, int timeoutMs) : socketPath(socketPath), logLevel(logLevel), timeoutMs(timeoutMs) {
    }

    ~AFUnixConsumer() {
        close(server_fd);
        close(client_fd);
    }

    int BindSocket() {
        // setup connection
        server_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
        if (server_fd < 0) {
            cerr << "socket init error: " << strerror(errno) << endl;
            return server_fd;
        }

        this->addr = {};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);
        unlink(socketPath.c_str()); // deletes old socket path if it exists

        if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            cerr << "socket bind error: " << strerror(errno) << endl;
            return (1);
        }

        return 0;
    }

    int ListenOnSocket() {
        if (listen(server_fd, 1) < 0) {
            cerr << "socket bind error: " << strerror(errno) << endl;
            return server_fd;
        }

        client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            cerr << "socket accept error: " << strerror(errno) << endl;
            return client_fd;
        }

        return 0;
    }

    ssize_t RecvSocket() {
        char buffer[128];
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            cout << "Server received: " << string(buffer, bytes_received) << endl;
        } else {
            cerr << "recv failed: " << bytes_received << ", " << strerror(errno) << endl;
        }

        return bytes_received;
    }
};

int main(int argc, char* argv[]) {
    // add server args
    cxxopts::Options options(argv[0], "Server");

    options.add_options()
        ("socket-path", "Path to socket", cxxopts::value<std::string>()->default_value("/tmp/dummy_socket"))
        ("log-level", "Logging level", cxxopts::value<std::string>()->default_value("INFO"))
        ("timeout-ms", "Timeout in ms", cxxopts::value<int>()->default_value("0"))
        ("h,help", "Print help");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    socketPath = result["socket-path"].as<std::string>();
    logLevel = result["log-level"].as<std::string>();
    timeoutMs = result["timeout-ms"].as<int>();

    // print variables in lieu of debug
    std::cout << "Socket path: " << socketPath << "\n"
              << "Log level: " << logLevel << "\n"
              << "Timeout ms: " << timeoutMs << "\n";

    // create server
    AFUnixConsumer afuc(socketPath, logLevel, timeoutMs);
    afuc.BindSocket();

    afuc.ListenOnSocket();

    afuc.RecvSocket();

    afuc.RecvSocket();

}
