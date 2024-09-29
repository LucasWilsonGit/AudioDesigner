#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>

int main() {
    Net::init();
    
    auto ipv6 = Net::address_ipv6("2001:db8::1");
    std::cout << "ipv6 address: " << ipv6.display_string() << "\n";
    if (ipv6.display_string().find("2001:db8::1") == std::string::npos)
        return -1;

    return 0;
}