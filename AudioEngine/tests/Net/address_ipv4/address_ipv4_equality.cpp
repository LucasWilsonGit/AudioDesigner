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
    auto ipv4_2 = Net::address_ipv4("254.124.254.124");
    auto ipv4_3 = Net::address_ipv4("254.123.254.123");

    if (ipv4 == ipv4_2)
        return -1;
    
    if (ipv4 != ipv4_3)
        return -1;

    return 0;
}