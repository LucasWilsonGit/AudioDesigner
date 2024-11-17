#include <iostream>
#include "AudioEngine/core.hpp"
#include "AudioEngine/ring_buffer.hpp"

template <class T, size_t N, class _allocTy>
void log_buff(AudioEngine::ring_buffer<T, N, _allocTy>& buff) {
    std::string s;

    buff.for_each([&](auto const& elem) { s += std::to_string(elem) + " ";});
    std::cout << "collection: uint32_t[" << N << "] has elements " << buff.size() << ": " << s << "\n";
}

int main() {
    AudioEngine::ring_buffer<uint32_t, 8> buff;
    buff.append(3);
    buff.append(5);

    log_buff(buff);

    std::cout << "front: " << buff.front() << " back: " << buff.back() << "\n";

    buff.pop_back();

    log_buff(buff);

    std::cout << "front: " << buff.front() << " back: " << buff.back() << "\n";
    
    if (buff.size() != 1)
        return 1;

    if (buff.back() != 3)
        return 1;
    
    return 0;
}