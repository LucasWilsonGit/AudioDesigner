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

    Net::socket_t sock = Net::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    try {
        //setup socket binding
        Net::bind(sock, ep);

        auto ep2 = Net::get_sock_name(sock); //only works when we have a binding, else should error

        auto peerlport = ep2.port;
        auto peerrport = ep.port;

        auto peerladdr = std::get<Net::address_ipv4>(ep2.address).number();
        auto peerraddr = std::get<Net::address_ipv4>(ep.address).number();

        std::cout << "peerlport " << peerlport << " peerrport " << peerrport << " peerladdr " << peerladdr << " peerraddr " << peerraddr << "\n";

        bool res = (ep == ep2);
        Net::close(sock);
        return !res;
    }
    catch (Net::net_error const& e) {
        Net::close(sock);
        std::cout << e.what() << "\n";
        return 1;
    }

    return 0;
}