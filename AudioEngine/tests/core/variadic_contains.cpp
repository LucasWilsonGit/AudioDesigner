#include "AudioEngine/core.hpp"

#include <string>

int main() {
    using namespace AudioEngine;
    
    //ensure that it does validate when it is present
    if constexpr (!variadic_contains_v<int, float, int, short>) {
        return 1;
    }

    //check that it should not validate when not present
    if constexpr (variadic_contains_v<int, bool>) {
        return 1;
    }

    //double check, this is the main use case
    if constexpr (variadic_contains_v<uint32_t, uint64_t, float, short, double, bool, std::tuple<int>, std::string>) {
        return 1;
    }

    //try with some cv-qualifiers, should not match cv to noncv types
    if constexpr (variadic_contains_v<volatile uint32_t, uint32_t>) 
        return 1;

    //see with reference types fail case
    if constexpr (variadic_contains_v<int&&, int&, float, double, char const>)
        return 1;
    
    //see with reference types pass case
    if constexpr (!variadic_contains_v<int&&, float, std::string, int&&>)
        return 1;

    return 0;
}