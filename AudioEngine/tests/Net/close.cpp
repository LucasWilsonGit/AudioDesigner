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

    //should not fail
    try {
        //setup socket binding
        Net::bind(sock, ep);

        auto ep2 = Net::get_sock_name(sock); //only works when we have a binding, else should error
        Net::close(sock);

        std::cout << "Pass [should not fail] case\n";
    }
    catch (Net::net_error const& e) {
        Net::close(sock);
        std::cout << e.what() << "\n";
        return 1;
    }

    //should fail
    try {
        //setup socket binding
        sock = Net::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        Net::close(sock);
        Net::bind(sock, ep); //bind after close should fail
        
        return 1;
    }
    catch (Net::net_error const& e) {
        std::cout << "Pass [should fail] case\n";
    }

    return 0;
}