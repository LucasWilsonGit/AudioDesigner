#include "AudioEngine/core.hpp"
#include "AudioEngine/block_allocator.hpp"
#include "test_block_allocator.hpp"

#include <vector>

int main() {

    try {

        int64_t *buf = new int64_t[64];
        test_block_allocator<int64_t, 64> allocator(buf);
        auto container = std::vector<int64_t, decltype(allocator)>(8, allocator);

        std::array<uint8_t, 64> validate0 = {
            9,8,7,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        };
        allocator.log_markers();
        if (!allocator.test_marker_array(validate0))
            throw std::runtime_error("Managed buffer did not match expected state!");

        for (int64_t i = 0; i < 17; i++) {
            container.push_back(i);
            allocator.log_markers();
        }

        std::array<uint8_t, 64> validate1 = {
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,
            0,0,0,0,0
        };
        allocator.log_markers();
        if (!allocator.test_marker_array(validate1))
            throw std::runtime_error("Managed buffer did not match expected state!");

        return 0;
    }
    catch (AudioEngine::dsp_error const& e) {
        std::cout << "Error: " << e.what() << "\n";
        return 1;
    }

    return 1;
}