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

    //THIS SHOULD FAIL! WE WANT IT TO FAIL! 
    try {
        //setup socket binding
        volatile auto ep2 = Net::get_sock_name(sock); //only works when we have a binding, else should error
        (void)ep2;
        return 1;
    }
    catch (Net::net_error const& e) {
        Net::close(sock);
    }


    try {
        //setup socket binding
        sock = Net::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        Net::bind(sock, ep);

        volatile auto ep2 = Net::get_sock_name(sock); //only works when we have a binding, else should error
        (void)ep2;
        Net::close(sock);
    }
    catch (Net::net_error const& e) {
        Net::close(sock);
        std::cout << e.what() << "\n";
        return 1;
    }

    return 0;
}