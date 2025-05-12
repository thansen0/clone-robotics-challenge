#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/un.h>         // sockaddr_un
#include <errno.h>          // strerror(errno)
#include <unistd.h>         // close

#include <cxxopts.hpp>
#include "transmission_header.hpp"

using namespace std;

// server global variables
static string socketPath;
static string logLevel;
static int timeoutMs;

class AFUnixClient {
private:
    string socketPath;
    string logLevel;
    int timeoutMs;

    int sock_fd;
    sockaddr_un addr;

public:
    AFUnixClient(string socketPath, string logLevel, int timeoutMs) : socketPath(socketPath), logLevel(logLevel), timeoutMs(timeoutMs) {
    }

    ~AFUnixClient() {
        close(sock_fd);
    }

    int ConnectSocket() {
        sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
        if (sock_fd < 0) {
            cerr << "socket failed: " << strerror(errno) << endl;
            return 1;
        }

        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

        if (connect(sock_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            cerr << "connect failed: " << strerror(errno) << endl;
            return 1;
        }

        return 0;
    }

    int SendOnSocket(const string message) {
        if (send(sock_fd, message.c_str(), strlen(message.c_str()), 0) < 0) {
            cerr << "socket send error: " << strerror(errno) << endl;
            return 1;
        }

        return 0;
    }
};

int main(int argc, char* argv[]) {
    // add client args
    cxxopts::Options options(argv[0], "Client");

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

    // create connection 
    AFUnixClient afuc(socketPath, logLevel, timeoutMs);
    afuc.ConnectSocket();

    afuc.SendOnSocket("Hello from client!");

    afuc.SendOnSocket("Hello from client 2!");

}
