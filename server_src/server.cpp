#include <iostream>
#include <cxxopts.hpp>
#include "transmission_header.hpp"

using namespace std;

// server global variables
static string socketPath;
static string logLevel;
static int frequencyHz;

int main(int argc, char* argv[]) {
    cout << "Running server!" << endl;

    cxxopts::Options options(argv[0], "Publisher Server");

    options.add_options()
        ("socket-path", "Path to socket", cxxopts::value<std::string>())
        ("log-level", "Logging level", cxxopts::value<std::string>()->default_value("INFO"))
        ("frequency-hz", "Publish frequency in Hz", cxxopts::value<int>()->default_value("0"))
        ("h,help", "Print help");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    socketPath = result["socket-path"].as<std::string>();
    logLevel = result["log-level"].as<std::string>();
    frequencyHz = result["frequency-hz"].as<int>();

    // print variables in lieu of debug
    std::cout << "Socket path: " << socketPath << "\n"
              << "Log level: " << logLevel << "\n"
              << "Frequency Hz: " << frequencyHz << "\n";
}
