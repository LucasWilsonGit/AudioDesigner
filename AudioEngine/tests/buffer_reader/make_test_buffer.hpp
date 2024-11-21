#pragma once

#include <memory>
#include <string>
#include "AudioEngine/core.hpp"
#include "AudioEngine/block_allocator.hpp"

template <class CharT>
[[nodiscard]] std::pair<std::unique_ptr<CharT>, size_t> make_test_buffer(char const* msg) {
    std::string s(msg);

    //this would not be nececssary if std::aligned_alloc would work but GCC do not want to be standards compliant here, or something... I am not happy.
    auto alloc = AudioEngine::block_allocator<AudioEngine::s16, 4096>(new AudioEngine::s16[ 4096 ]);
    AudioEngine::block_allocator<char, 4096> ralloc(alloc); //now we can allocate chars on a 16byte alignment

    std::unique_ptr<char> file_buffer( ralloc.allocate(s.size()) );
    if (!file_buffer)
        throw Memory::memory_error( format("Failed to allocate {} bytes of {} byte alignment memory.", s.size(), 16) );
    
    std::memcpy(file_buffer.get(), s.data(), s.size());

    std::string_view s2(file_buffer.get(), s.size());
    if (s2.compare(s) != 0) {
        throw std::runtime_error("Expected buffer to contain the string I want to test");
    }

    return std::make_pair(std::move(file_buffer), s.size());
}