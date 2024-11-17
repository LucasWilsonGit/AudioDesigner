#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>

int main() {
    Net::init();
    
    //0x2d4f70cb
    auto ipv4 = Net::address_ipv4(std::array<std::byte, 4>{std::byte(45), std::byte(79), std::byte(112), std::byte(203)});

    if (ipv4.number() != 0x2d4f70cb) //windows stores numbers backwards, on linux it should be fine comparing to byte arrays as-is, do htonl to get order to match byte array
        return -1;

    return 0;
}