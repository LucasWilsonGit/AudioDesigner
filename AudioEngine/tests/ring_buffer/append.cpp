#include <iostream>

#include "AudioEngine/core.hpp"
#include "AudioEngine/ring_buffer.hpp"

int main() {
    AudioEngine::ring_buffer<uint8_t, 8> buff;
    buff.append(3);
    buff.append(5);

    std::cout << "buff size: " << buff.size() << " back: " << (uint32_t)buff.back() << "\n";
    if (buff.size() != 2)
        return 1;
    
    if (buff.back() != 5)
        return 1;
    
    return 0;
}