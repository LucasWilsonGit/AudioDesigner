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
    auto ipv4 = Net::address_ipv4("254.123.254.123");
    std::cout << "Display string: " << ipv4.display_string() << "\n";
    if (ipv4.display_string().find("254.123.254.123") == std::string::npos)
        return -1;

    return 0;
}