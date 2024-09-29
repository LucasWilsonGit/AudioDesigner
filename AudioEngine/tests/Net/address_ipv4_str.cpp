#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>

bool byte_test(std::byte const& byte, uint8_t value) {
    std::cout << "byte_test " << (uint16_t)byte << ":" << (uint16_t)value << "\n"; 
    return (uint8_t)byte == value;
}

int main() {
    Net::init();
    
    auto ipv4 = Net::address_ipv4("254.123.254.123");
    auto bytes = ipv4.bytes();
    uint8_t validate[] = {254, 123,  254, 123};

    bool res = true;
    int i = 0;
    for (auto& byte : bytes) {
        res &= byte_test(byte, validate[i++]);
    }

    if (!res)
        return -1;

    return 0;
}