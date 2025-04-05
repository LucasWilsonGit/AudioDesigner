#include <iostream>

#include "AudioEngine/address.hpp"
#include "AudioEngine/sockapi.hpp"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <string>
#include <cstring>
#include <thread>
#include <future>

int main() {
    Net::init();
    
    auto ep = Net::end_point(Net::address_ipv4("127.0.0.1"), 1234);

    Net::socket_t sock = INVALID_SOCKET;

    //should not except
    try {
        //setup socket binding
        sock = Net::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        Net::bind(sock, ep);

        Net::listen(sock, 5);

        
        Net::socket_t sock2 = Net::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        Net::connect(sock2, ep); //this will chuck an exception if the socket is not listening, should return cos the lsiten will let it

        auto [sock2_client, sock2_client_ep] = Net::accept(sock);
        std::cout << "Got socket join from " << sock2_client_ep << "\n";

        auto peername = Net::get_peer_name(sock2_client);
        std::cout << "Peer socket " << peername << "\n";

        auto peerlport = sock2_client_ep.port;
        auto peerrport = peername.port;

        auto peerladdr = std::get<Net::address_ipv4>(sock2_client_ep.address).number();
        auto peerraddr = std::get<Net::address_ipv4>(peername.address).number();

        std::cout << "peerlport " << peerlport << " peerrport " << peerrport << " peerladdr " << peerladdr << " peerraddr " << peerraddr << "\n";

        bool res = (sock2_client_ep.port == peername.port && std::get<Net::address_ipv4>(sock2_client_ep.address).number() == std::get<Net::address_ipv4>(peername.address).number());
        Net::close(sock);
        return !res;
    }
    catch (Net::net_error const& e) {
        if (sock != INVALID_SOCKET)
            Net::close(sock);
        std::cout << e.what() << "\n";
        return 1;
    }

}