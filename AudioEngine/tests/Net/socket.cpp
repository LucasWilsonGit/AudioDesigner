#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>

int main() {
    Net::init();

    try {
        if (Net::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) == INVALID_SOCKET)
            return -1;

        if (Net::socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP) == INVALID_SOCKET)
            return -1;

        if (Net::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) == INVALID_SOCKET)
            return -1;
        
        if (Net::socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP) == INVALID_SOCKET)
            return -1;
    }
    catch (Net::net_error const& e) {
        std::cout << e.what() << "\n";
        return 1;
    }

    return 0;
}