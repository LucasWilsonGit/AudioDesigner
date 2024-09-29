#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>

int main() {
    Net::init();
    
    std::array<std::byte, 4> validation = {std::byte(254), std::byte(123), std::byte(254), std::byte(123)};

    //0x2d4f70cb
    auto ipv4 = Net::address_ipv4("254.123.254.123");
    
    for (int i = 0; i < 4; i++) {
        if ( static_cast<uint8_t>(validation[i]) != static_cast<uint8_t>(ipv4.bytes()[i]) )
            return -1;
    }

    return 0;
}