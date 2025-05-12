#ifndef SIMPLE_LOGGER_HPP
#define SIMPLE_LOGGER_HPP

#include <iostream>
#include <string>
#include <mutex>

using namespace std;

class Logger {
public:
    enum class Level {
        NONE = 0,
        ERROR = 1,
        INFO = 2,
    };

    static void setLevel(string newLevel) {
        if (newLevel == "NONE") {
            getInstance().logLevel = Level::NONE;
        } else if (newLevel == "ERROR") {
            getInstance().logLevel = Level::ERROR;
        } else {
            getInstance().logLevel = Level::INFO;
        }
    }

    static void info(const std::string& message) {
        if (getInstance().logLevel >= Level::INFO) {
            std::lock_guard<std::mutex> lock(getInstance().mtx);
            std::cout << "[INFO] " << message << std::endl;
        }
    }

    static void error(const std::string& message) {
        if (getInstance().logLevel >= Level::ERROR) {
            std::lock_guard<std::mutex> lock(getInstance().mtx);
            std::cerr << "[ERROR] " << message << std::endl;
        }
    }

private:
    Logger() : logLevel(Level::INFO) {}

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    Level logLevel;
    std::mutex mtx;
};

#endif // SIMPLE_LOGGER_HPP