#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>

bool byte_validate(Net::address_ipv6 const& addr, std::array<std::byte, 16> const& bytes) {
    std::cout << "bytes : addr" << "\n";
    for (size_t i = 0; i < bytes.size(); i+=2) {
        std::cout << std::hex << ntohs(reinterpret_cast<uint16_t const&>(bytes[i])) << " : " << ntohs(reinterpret_cast<uint16_t const&>(addr.bytes()[i])) << std::dec << "\n";
        if ( reinterpret_cast<uint16_t const&>(bytes[i]) != reinterpret_cast<uint16_t const&>(addr.bytes()[i]) )
        {   
            std::cout << "false\n";
            return false;
        }
    }
    std::cout << "\n";

    return true;
}

int main() {
    Net::init();
    
    std::array<std::byte, 16> bytes = {{
        std::byte{0x20}, std::byte{0x01},  // 2001
        std::byte{0x0d}, std::byte{0xb8},  // 0db8
        std::byte{0x85}, std::byte{0xa3},  // 85a3
        std::byte{0x00}, std::byte{0x00},  // 0000
        std::byte{0x00}, std::byte{0x00},  // 0000
        std::byte{0x8a}, std::byte{0x2e},  // 8a2e
        std::byte{0x03}, std::byte{0x70},  // 0370
        std::byte{0x73}, std::byte{0x34}   // 7334
    }};
    auto ipv6 = Net::address_ipv6("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
    
    //check longform pass
    if (!byte_validate(ipv6, bytes))
        return -1;

    bytes = {{
        std::byte{0x20}, std::byte{0x01},  // 2001
        std::byte{0x0d}, std::byte{0xb8},  // 0db8
        std::byte{0x00}, std::byte{0x00},  // 0000
        std::byte{0x00}, std::byte{0x00},  // 0000
        std::byte{0x00}, std::byte{0x00},  // 0000
        std::byte{0x00}, std::byte{0x00},  // 0000
        std::byte{0x00}, std::byte{0x00},  // 0000
        std::byte{0x00}, std::byte{0x01}   // 0001
    }};
    ipv6 = Net::address_ipv6("2001:db8::1");

    //check shortform pass
    if (!byte_validate(ipv6, bytes))
        return -1;

    //check fail case
    if (byte_validate(Net::address_ipv6("2001:db8:35fe::1"), bytes))
        return -1;

    return 0;
}