#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>

int main() {
    Net::init();
    
    std::array<std::byte, 16> bytes = {
        std::byte{0x20}, std::byte{0x01},  // 2001
        std::byte{0x0d}, std::byte{0xb8},  // 0db8
        std::byte{0x85}, std::byte{0xa3},  // 85a3
        std::byte{0x00}, std::byte{0x00},  // 0000
        std::byte{0x00}, std::byte{0x00},  // 0000
        std::byte{0x8a}, std::byte{0x2e},  // 8a2e
        std::byte{0x03}, std::byte{0x70},  // 0370
        std::byte{0x73}, std::byte{0x34}   // 7334
    };
    auto ipv6 = Net::address_ipv6(bytes);

    std::cout << "ipv6: " << ipv6.display_string() << "\n";

    if (ipv6.display_string().find("2001:db8:85a3::8a2e:370:7334") == std::string::npos)
        return -1;

    return 0;
}