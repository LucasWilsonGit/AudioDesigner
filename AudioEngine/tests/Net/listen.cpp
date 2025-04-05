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

void client(Net::end_point const& dst) {
    Net::socket_t sock = Net::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    Net::connect(sock, dst); //this will chuck an exception if the socket is not listening
}

int main() {
    Net::init();
    
    auto ep = Net::end_point(Net::address_ipv4("127.0.0.1"), 1234);
    auto ep_fail = Net::end_point(Net::address_ipv4("127.0.0.1"), 1235);

    Net::socket_t sock = INVALID_SOCKET;

    //should not except
    try {
        //setup socket binding
        sock = Net::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        Net::bind(sock, ep);

        Net::listen(sock, 5);

        std::future<void> res = std::async(std::launch::async, client, ep);
        res.get(); //if it has an exception it will throw
        Net::close(sock);
    }
    catch (Net::net_error const& e) {
        if (sock != INVALID_SOCKET)
            Net::close(sock);
        std::cout << e.what() << "\n";
        return 1;
    }



    //should except
    try {
        //setup socket binding
        sock = Net::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        Net::bind(sock, ep_fail);

        std::future<void> res = std::async(std::launch::async, client, ep_fail);
        res.get(); //if it has an exception it will throw
        return 1;
    }
    catch (Net::net_error const& e) {
        if (sock != INVALID_SOCKET)
            Net::close(sock);
        std::cout << e.what() << "\n";
        return 0;
    }

}