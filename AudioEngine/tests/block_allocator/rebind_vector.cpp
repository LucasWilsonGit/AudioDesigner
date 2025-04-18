#include "AudioEngine/core.hpp"
#include "AudioEngine/block_allocator.hpp"
#include "test_block_allocator.hpp"

#include <vector>
#include <array>

int main() {

    struct s32b {
        uint64_t data[4];
    };

    try {

        alignas(32) s32b *buf = new s32b[64];
        test_block_allocator<s32b, 64> allocator(buf);
        auto container = std::vector<uint64_t, decltype(allocator)::rebind<uint64_t>::other>(5, decltype(allocator)::rebind<uint64_t>::other(allocator));

        std::array<uint32_t, 64> validate0 = {
            2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        };
        allocator.log_markers();
        if (!allocator.test_marker_array(validate0))
            throw std::runtime_error("(1/2) Managed buffer did not match expected state!");
        
        container.reserve(10);
        container.reserve(20);

        std::array<uint32_t, 64> validate1 = {
            0,0,0,0,0,6,5,4,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        };

        allocator.log_markers();
        if (!allocator.test_marker_array(validate1))
            throw std::runtime_error("(2/2) Managed buffer did not match expected state!");
        

        return 0;
    }
    catch (AudioEngine::dsp_error const& e) {
        std::cout << "Error: " << e.what() << "\n";
        return 1;
    }
}