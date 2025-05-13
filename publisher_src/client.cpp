/*
 *     ___ _                      __       _           _   _          
 *    / __\ | ___  _ __   ___    /__\ ___ | |__   ___ | |_(_) ___ ___ 
 *   / /  | |/ _ \| '_ \ / _ \  / \/// _ \| '_ \ / _ \| __| |/ __/ __|
 *  / /___| | (_) | | | |  __/ / _  \ (_) | |_) | (_) | |_| | (__\__ \
 *  \____/|_|\___/|_| |_|\___| \/ \_/\___/|_.__/ \___/ \__|_|\___|___/
 *                                                                  
 *  Author: Thomas Hansen
 *  Version: 0.0.1
 */

#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <sys/un.h>         // sockaddr_un
#include <errno.h>          // strerror(errno)
#include <unistd.h>         // close
#include <random>           // mt19937_64

#include <cxxopts.hpp>
#include "transmission_header.hpp"
#include "simple_logger.hpp"
#include "AFUnixPublisher.hpp"

using namespace std;

// server global variables
static string socketPath;
static string logLevel;
static long long frequencyHz;

void cli_args(int argc, char* argv[]) {
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
        exit(0);
    }

    // assign static global variables
    socketPath = result["socket-path"].as<std::string>();
    logLevel = result["log-level"].as<std::string>();
    frequencyHz = result["frequency-hz"].as<long long>();
}

Payload_IMU_t calcRandPayload() {
    static mt19937_64 rng{ random_device{}() };

    // AI suggested distributions
    uniform_real_distribution<float> accDist(-4.0f, 4.0f);
    uniform_int_distribution<int32_t> gyroDist(-25000, 25000);
    uniform_real_distribution<float> magDist(-100.0f, 100.0f);

    // fake time "reading" was taken
    auto now_ms = static_cast<uint32_t>(
        chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count()
    );

    Payload_IMU_t payload;
    // accelerometer
    payload.xAcc = accDist(rng);
    payload.yAcc = accDist(rng);
    payload.zAcc = accDist(rng);
    payload.timestampAcc = now_ms;

    // gyro
    payload.xGyro = gyroDist(rng);
    payload.yGyro = gyroDist(rng);
    payload.zGyro = gyroDist(rng);
    payload.timestampGyro = now_ms;

    // magnetometer
    payload.xMag = magDist(rng);
    payload.yMag = magDist(rng);
    payload.zMag = magDist(rng);
    payload.timestampMag = now_ms;

    return payload;
}

int main(int argc, char* argv[]) {
    // read in command line arguments
    cli_args(argc, argv);
    

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
    Payload_IMU_t cur_data;

    while (true) {
        // must set wake time first for accurate timing
        next_wake += wait_period;

        // generate random IMU data for payload
        cur_data = calcRandPayload();
        
        status = afup.SendOnSocket(&cur_data);
        if (status != 0) {
            // connection broken, exit
            break;
        }

        // sleep until next_wait time
        this_thread::sleep_until(next_wake);
    }

    return 0;
}
