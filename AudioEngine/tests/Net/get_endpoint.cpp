#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>

int main() {
    Net::init();
    
    auto ep = Net::end_point(Net::address_ipv4("254.123.254.123"), 1234);

    auto [addrstorage, addrlen] = Net::get_end_point(ep);
    sockaddr_in const& ipv4 = reinterpret_cast<sockaddr_in const&>(addrstorage);

    if (ipv4.sin_family != AF_INET) 
        return -1;

    if (ipv4.sin_addr.S_un.S_addr != htonl(0xfe7bfe7b))
        return -1;

    if (ipv4.sin_port != htons(1234))
        return -1;

    return 0;
}