#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>

int main() {
    Net::init();
    
    auto ep = Net::end_point(Net::address_ipv4("127.0.0.1"), 1234);
    auto ep2 = Net::end_point(Net::address_ipv4("127.0.0.2"), 1234);
    auto ep3 = Net::end_point(Net::address_ipv4("127.0.0.1"), 1234);
    auto ep4 = Net::end_point(Net::address_ipv4("127.0.0.1"), 1235);

    if (ep == ep2)
        return -1;

    if (ep != ep3)
        return -1;
    
    if (ep == ep4)
        return -1;

    return 0;
}