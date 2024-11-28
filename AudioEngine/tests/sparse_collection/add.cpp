#include <iostream>
#include <sstream>

#include "AudioEngine/core.hpp"
#include "AudioEngine/sparse_collection.hpp"
#include "AudioEngine/block_allocator.hpp"

template <class Alloc_T>
std::string serialize(AudioEngine::sparse_collection<uint64_t, Alloc_T>& collection) {
    
    std::stringstream ss;
    for (auto& elem : collection.values()) {
        ss << elem.value;
    }

    return ss.str();
}

int main() {
    struct alignas(16) s16 {
        char b[16];
    };

    auto aligned_allocator = AudioEngine::block_allocator<s16, 1024>(new s16[1024]);
    AudioEngine::sparse_collection<uint64_t, decltype(aligned_allocator)> data(aligned_allocator);

    auto _ = data.add(1);
    auto t = data.add(2);
    _=data.add(3);
    _=data.add(4);

    (void)t;
    
    if (serialize(data) != "1234") {
        throw std::runtime_error("sparse_collection storage did not match expected value (1234)");
    }
    
    return 0;
}