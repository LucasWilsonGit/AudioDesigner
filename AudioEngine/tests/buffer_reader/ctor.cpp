#include "AudioEngine/dsp.hpp"

#include "make_test_buffer.hpp"

int main() {
    char const* msg = "Hello, world! This is a test message\nThe quick brown fox, jumped over the lazy dog.\r\n\r\nLorem ipsum! dolor _312sit amet.é";
    auto [buff, len] = make_test_buffer<char>(msg);

    AudioEngine::buffer_reader<char, 16> reader(buff.get(), len);
}