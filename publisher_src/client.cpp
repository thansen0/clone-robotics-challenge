#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <sys/un.h>         // sockaddr_un
#include <errno.h>          // strerror(errno)
#include <unistd.h>         // close

#include <cxxopts.hpp>
#include "transmission_header.hpp"
#include "simple_logger.hpp"

using namespace std;

// server global variables
static string socketPath;
static string logLevel;
static long long frequencyHz;

// client acts as publisher
class AFUnixPublisher {
private:
    string socketPath;
    string logLevel;

    int sock_fd;
    sockaddr_un addr;

public:
    AFUnixPublisher(string socketPath, string logLevel) : socketPath(socketPath), logLevel(logLevel) {
    }

    ~AFUnixPublisher() {
        close(sock_fd);
    }

    int ConnectSocket() {
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

    int SendOnSocket(const string message) {
        if (send(sock_fd, message.c_str(), strlen(message.c_str()), 0) < 0) {
            Logger::error("socket send error: " + string(strerror(errno)));
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
        ("frequency-hz", "Publish frequency in Hz", cxxopts::value<long long>()->default_value("1"))
        ("h,help", "Print help");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    // assign static global variables
    socketPath = result["socket-path"].as<std::string>();
    logLevel = result["log-level"].as<std::string>();
    frequencyHz = result["frequency-hz"].as<long long>();

    // set logger level, NOTE: all incorrect logger levels are
    // defaulted to INFO
    Logger::setLevel(logLevel);

    Logger::info("Socket path: " + socketPath);
    Logger::info("Log level: " + logLevel);
    Logger::info("Frequency Hz: " + to_string(frequencyHz));

    // setup frequency/wait until 
    auto next_wake = chrono::steady_clock::now();

    if (frequencyHz <= 0) {
        Logger::error("frequency-hz must be greater than 0");
        return -1;
    }
    long long ms_period = static_cast<long long>(1000.0 / frequencyHz);
    const chrono::milliseconds wait_period(ms_period);

    // create connection 
    AFUnixPublisher afup(socketPath, logLevel);
    afup.ConnectSocket();

    int status;

    while (true) {
        // must set wake time first for accurate timing
        next_wake += wait_period;
        
        status = afup.SendOnSocket("Hello from client!");
        if (status != 0) {
            // connection broken, exit
            break;
        }

        // sleep until next_wait time
        this_thread::sleep_until(next_wake);
    }

    return 0;
}
