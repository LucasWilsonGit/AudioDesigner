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
    addr.S_un.S_addr = htonl(0xFE7BFE7B);

    sockaddr_in ipv4 {
        .sin_family = AF_INET,
        .sin_port = ::htons(1234),
        .sin_addr = addr,
        .sin_zero = {0}
    };

    auto ep = Net::parse_end_point(reinterpret_cast<sockaddr const&>(ipv4));
    std::string sep = ep;
    std::cout << "sep: " << sep << "!\n";
    std::cout << std::hex << "0x" << sep.find("254.123.254.123:1234") << std::dec << "\n";
    if (sep.find("254.123.254.123:1234") == std::string::npos)
        return -1;

    return 0;
}