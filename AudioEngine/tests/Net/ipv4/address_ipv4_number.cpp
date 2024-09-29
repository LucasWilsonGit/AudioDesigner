#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>

int main() {
    Net::init();

    //0xfe7bfe7b
    auto ipv4 = Net::address_ipv4("254.123.254.123");
    if (ipv4.number() != htonl(0xfe7bfe7b))
        return -1;

    return 0;
}