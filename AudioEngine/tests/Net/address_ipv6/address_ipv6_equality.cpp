#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>

int main() {
    Net::init();
    
    auto a = Net::address_ipv6("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
    auto b = Net::address_ipv6("2001:0db8:85a3:0000:0000:8a2e:0370:7335");
    auto c = Net::address_ipv6("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
    
    if (a == b)
        return -1;

    if (a != c)
        return -1;

    return 0;
}