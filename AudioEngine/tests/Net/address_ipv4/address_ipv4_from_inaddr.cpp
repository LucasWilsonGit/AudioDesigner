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

    auto ip = Net::address_ipv4(addr);
    if (ip.number() != 0xfe7bfe7b)
        return -1;

    return 0;
}