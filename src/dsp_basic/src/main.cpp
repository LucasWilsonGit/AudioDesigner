#include "AudioEngine/socket.hpp"

#include <iostream>

int main() {

    AudioEngine::address_ipv4 addr{"10.203.12.220"};
    std::cout << addr.display_string() << "\n";

    return 0;
}