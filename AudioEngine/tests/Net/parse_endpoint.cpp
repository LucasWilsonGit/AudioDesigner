#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>

int main() {
    Net::init();

    in_addr addr;
    addr.S_un.S_addr = 0x7BFE7BFE;

    sockaddr_in ipv4 {
        .sin_family = AF_INET,
        .sin_port = ::htons(1234),
        .sin_addr = addr
    };

    auto ep = Net::parse_end_point(reinterpret_cast<sockaddr const&>(ipv4));
    std::string sep = ep;
    if (sep.find("254.123.254.123:1234") == std::string::npos)
        return -1;

    return 0;
}