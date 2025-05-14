#ifndef AFUNIXPUBLISHER_HPP
#define AFUNIXPUBLISHER_HPP

#include <string>
#include <sys/un.h>
#include "simple_logger.hpp"
#include "transmission_header.hpp"   // for Payload_IMU_t

class AFUnixPublisher {
private:
    string      socketPath;
    string      logLevel;
    int         sock_fd = -1;
    sockaddr_un addr;

public:
    AFUnixPublisher(const string socketPath, const string logLevel);
    ~AFUnixPublisher();

    int ConnectSocket();
    int SendOnSocket(const string message);
    int SendOnSocket(const Payload_IMU_t* data);
    int ResetSocket();
};

#endif // AFUNIXPUBLISHER_HPP
