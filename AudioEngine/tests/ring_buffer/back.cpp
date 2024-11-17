#include <iostream>

#include "AudioEngine/core.hpp"
#include "AudioEngine/ring_buffer.hpp"

int main() {
    AudioEngine::ring_buffer<uint8_t, 8> buff;
    buff.append(3);
    
    if (buff.size() != 1)
        return 1;

    if (buff.back() != 3)
        return 1;
    
    return 0;
}