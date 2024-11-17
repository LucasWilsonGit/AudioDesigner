#include "AudioEngine/core.hpp"
#include "AudioEngine/block_allocator.hpp"
#include "test_block_allocator.hpp"

#include <vector>

int main() {

    try {
        std::array<uint8_t, 64> validate0 = {
            2,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        };
        std::array<uint8_t, 64> validate1 = {
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        };

        if (test_marker_array(validate0, validate1))
            throw AudioEngine::dsp_error("Validator passed what should have been a fail case");    

        if (!test_marker_array(validate0, validate0))
            throw AudioEngine::dsp_error("Validator failed to pass what should have been a pass case");   

        return 0;
    }
    catch (AudioEngine::dsp_error const& e) {
        std::cout << "Error: " << e.what() << "\n";
        return 1;
    }

    return 1;
}