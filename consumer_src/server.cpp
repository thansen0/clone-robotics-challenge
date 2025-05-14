//     ___ _                      __       _           _   _          
//    / __\ | ___  _ __   ___    /__\ ___ | |__   ___ | |_(_) ___ ___ 
//   / /  | |/ _ \| '_ \ / _ \  / \/// _ \| '_ \ / _ \| __| |/ __/ __|
//  / /___| | (_) | | | |  __/ / _  \ (_) | |_) | (_) | |_| | (__\__ \
//  \____/|_|\___/|_| |_|\___| \/ \_/\___/|_.__/ \___/ \__|_|\___|___/
//                                                                  
//  Author: Thomas Hansen
//  Version: 0.0.1
//

#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/un.h>         // sockaddr_un
#include <errno.h>          // strerror(errno)
#include <unistd.h>         // unlink, close
#include <cstring>          // c_str()

#include <cxxopts.hpp>
#include "AFUnixConsumer.hpp"
#include "transmission_header.hpp"
#include "simple_logger.hpp"

using namespace std;

// server global variables
static string socketPath;
static string logLevel;
static long long timeoutMs;

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

    // sanity check, ignore upper bound
    if (timeoutMs < 0) {
        Logger::info("Timeout ms cannot be less than 0");
        return -1;
    }

    // create server
    AFUnixConsumer afuc(socketPath, logLevel);
    afuc.BindSocket();

    if (SOCKET_SUCCESS != afuc.ListenOnSocket()) {
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

        // take abs incase timeout is negative
        if (abs(time_diff) > timeoutMs) {
            Logger::error("Timeout exceeded, exiting");
            break;
        }
    }

    return 0;
}
