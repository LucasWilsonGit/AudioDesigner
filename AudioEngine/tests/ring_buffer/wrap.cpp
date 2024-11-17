#include <iostream>

#include "AudioEngine/core.hpp"
#include "AudioEngine/ring_buffer.hpp"

int main() {
    AudioEngine::ring_buffer<uint32_t, 4> buff;
    
    for (int i = 0; i < 5; i++) {
        buff.append(i);
        std::string s;
        buff.for_each([&](auto const& elem) -> void { s += std::to_string(elem) + " ";});
        std::cout << "collection " << "has elements " << buff.size() << ": " << s << "\n";
    }

    std::cout << "front: " << buff.front() << " back: " << buff.back() << "\n";
    
    if (buff.size() != 4)
        return 1;    

    if (buff.back() != 4)
        return 1;

    if (buff.front() != 1)
        return 1;

    buff.pop_front();

    if (buff.front() != 2)
        return 1;
    
    return 0;
}