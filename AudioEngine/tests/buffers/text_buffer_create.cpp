#include <iostream>

#include "gtest/gtest.h"

#include "AudioEngine/core.hpp"
#include "AudioEngine/buffers/text_buffer.hpp"
#include "AudioEngine/block_allocator.hpp"

class test_allocator : public AudioEngine::block_allocator<char, 4096> {
private:
    size_t m_allocated = 0;
public:
    test_allocator(void *prealloced_pool) : AudioEngine::block_allocator<char, 4096>(prealloced_pool)
    {}

    [[nodiscard]] char *allocate(size_t count) {
        m_allocated += count;
        return AudioEngine::block_allocator<char, 4096>::allocate(count);
    }

    void deallocate(char *elem, size_t count) {
        m_allocated -= count;
        return AudioEngine::block_allocator<char, 4096>::deallocate(elem, count);
    }

    [[nodiscard]] size_t allocated() const noexcept {
        return m_allocated;
    }
};

class text_buffer_test : public testing::Test {
protected:
    alloc_test() {
        try {
            alloc.emplace(new char[4096]);
            buff.emplace(4096, alloc);
        }
        catch (...) {
            ex = std::current_exception();
        }
    }


    std::exception_ptr ex = nullptr;
    std::optional<test_allocator> alloc;
    std::optional<AudioEngine::text_buffer<char, test_allocator, 16>> buff;    
}


int main() {

    TEST_F(text_buffer_test, can_create) {
        ASSERT_EQ(alloc.has_value(), true) << "Failed to create allocator";
        ASSERT_EQ(buff.has_value(), true) << "Failed to create textbuffer";
        ASSERT_EQ(ex, nullptr) << format("Some exception occurred when setting up alloc/buff: {}", ex->what());
    }

    TEST(text_buffer_test, can_reserve) {
        ASSERT_EQ(ex, nullptr) << "Exception found in test fixture";
        
    }

    
    
    buff.reserve(4);

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