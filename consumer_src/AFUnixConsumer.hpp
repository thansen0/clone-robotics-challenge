#ifndef AFUNIXCONSUMER_HPP
#define AFUNIXCONSUMER_HPP

#include <string>
#include <cstring>          // c_str()
#include <sys/un.h>         // sockaddr_un
#include "transmission_header.hpp"

using namespace std;

// server acts as consumer
class AFUnixConsumer {
private:
    string socketPath;
    string logLevel;

    int server_fd, client_fd;
    sockaddr_un addr;

public:
    AFUnixConsumer(string socketPath, string logLevel);
    ~AFUnixConsumer();

    int BindSocket();
    int ListenOnSocket();

    /* 
     * Note: this directly returns the recv error values, not my custom macros,
     * so that we can also get size
     */
    ssize_t RecvSocket(Payload_IMU_t* buffer);
};

#endif // AFUNIXCONSUMER_HPP