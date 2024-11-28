#include <iostream>
#include <chrono>
#include <numbers>
#include <cmath>

void generate_sin_wave(int16_t *buffer, size_t sample_count) {
    int16_t *abuffer = std::assume_aligned<32>(buffer);

    for (size_t i = 0; i < sample_count; i++) {
        float ts = i/48000;
        abuffer[i] = (int16_t)(std::numeric_limits<int16_t>::max() * std::sin(ts * std::numbers::pi));
    }
}

void generate_sin_wave2(int16_t *buffer, size_t sample_count) {
    for (size_t i = 0; i < sample_count; i++) {
        float ts = i/48000;
        buffer[i] = (int16_t)(std::numeric_limits<int16_t>::max() * std::sin(ts * std::numbers::pi));
    }
}

int main() {
    auto start_t = std::chrono::steady_clock::now(); 
    for (int i = 0; i < 1'000'000; i++) {
        int16_t *buf = new int16_t[480];
        generate_sin_wave(buf, 480);
        delete[] buf;
    }
    auto end_t = std::chrono::steady_clock::now();
    auto elapsed = end_t - start_t;
    std::cout << "aligned test " << ( elapsed.count() / 1'000'000 ) << " ns elapsed\n";

    start_t = std::chrono::steady_clock::now(); 
    for (int i = 0; i < 1'000'000; i++) {
        int16_t *buf = new int16_t[480];
        generate_sin_wave2(buf, 480);
        delete[] buf;
    }
    end_t = std::chrono::steady_clock::now();
    elapsed = end_t - start_t;
    std::cout << "unaligned test " << ( elapsed.count() / 1'000'000 ) << " ns elapsed\n";
}