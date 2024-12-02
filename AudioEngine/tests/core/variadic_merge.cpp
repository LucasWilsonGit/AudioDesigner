#include "AudioEngine/core.hpp"
#include <cstdint>

int main() {
    using namespace AudioEngine;

    {   //check that we can add a type
        
        using test_type = variadic_merge_impl<std::tuple<int, float>, char>::type;
        if constexpr (!std::is_same_v<test_type, std::tuple<int, float, char>>) {
            return 1;
        }
    }

    {   //something a bit more complicated
        
        using test_type = variadic_merge_impl<std::tuple<int, float, char, uint32_t const>, float, char, uint64_t>::type;
        if constexpr (!std::is_same_v<test_type, std::tuple<int, float, char, uint32_t const, uint64_t>>)
            return 1;
    }



    return 0;
}