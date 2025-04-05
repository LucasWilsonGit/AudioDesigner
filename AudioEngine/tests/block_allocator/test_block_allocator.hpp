#pragma once
#include "AudioEngine/core.hpp"
#include "AudioEngine/block_allocator.hpp"

template <size_t M>
bool test_marker_array(std::array<uint32_t, M> const& left, std::array<uint32_t, M> const& right) {
    return std::equal(left.cbegin(), left.cend(), right.cbegin());
}

/**
 * @name   test_block_allocator
 * @brief  Encapsulates the AudioEngine::block_allocator<T, N> with a function to compare the internal marker array to validate the allocated buffer state against some predefined scenarios
 */
template <class T, size_t N>
class test_block_allocator : public AudioEngine::block_allocator<T, N> {
public:
    test_block_allocator() = delete;
    using AudioEngine::block_allocator<T, N>::block_allocator;

    template <class U>
    struct rebind {
        using other = test_block_allocator<U, N>;
    };

    bool test_marker_array(std::array<uint32_t, N> const& expected) const {
        auto storage = this->get_storage();
        if (!storage)
            throw std::runtime_error("Storage not initialized!");
        
        return ::test_marker_array(storage->markers, expected);
    }

    void log_markers() const {
        auto storage = this->get_storage();
        if (!storage)
            throw std::runtime_error("Storage not initialized!");
        
        for (size_t i = 0; i < N; i++) {
            std::cout << static_cast<int>(storage->markers[i]) << " ";
        }
        std::cout << std::endl;
    }
};
