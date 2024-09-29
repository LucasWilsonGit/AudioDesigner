#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>

int main() {
    Net::init();
    
    auto ipv6 = Net::address_ipv6("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
    if (ipv6.display_string().find("2001:db8:85a3::8a2e:370:7334") == std::string::npos)
        return -1;

    return 0;
}