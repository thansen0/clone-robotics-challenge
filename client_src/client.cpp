#include <iostream>
#include <cxxopts.hpp>
#include "transmission_header.hpp"

using namespace std;

// server global variables
static string socketPath;
static string logLevel;
static int timeoutMs;

int main(int argc, char* argv[]) {
    cout << "Running client!" << endl;

    cxxopts::Options options(argv[0], "Consumer Client");

    options.add_options()
        ("socket-path", "Path to socket", cxxopts::value<std::string>())
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
}
