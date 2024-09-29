#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <string>

int main() {
    Net::init();

    in6_addr addr {
        {
            0x20, 0x01, 0x0d, 0xb8, 
            0x85, 0xa3, 0x00, 0x00, 
            0x00, 0x00, 0x8a, 0x2e, 
            0x03, 0x70, 0x73, 0x34 
        }
    };
    
    Net::address_ipv6 ipv6(addr);

    if (ipv6.display_string().find("2001:db8:85a3::8a2e:370:7334") == std::string::npos)
        return -1;
    
    return 0;
}