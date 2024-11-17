#include <iostream>

#include "AudioEngine/core.hpp"
#include "AudioEngine/ring_buffer.hpp"

#include <vector>
#include <functional>

template <class T, size_t N>
void setup_buff(AudioEngine::ring_buffer<T,N>& buff, int count) {
    for (int i = 0; i < count; i++) {
        buff.append(i);
    }
}

void visit_elem(uint32_t const& elem, std::vector<uint32_t>& checks) {
    if (checks[0] != elem)
        throw std::runtime_error("Mismatched element in iteration.");

    std::cout << "visited element " << elem << "\n";
    
    checks.erase(checks.begin());
}

int main() {
    AudioEngine::ring_buffer<uint32_t, 4> buff;
    setup_buff(buff, 5);

    AudioEngine::ring_buffer<uint32_t, 8> buff2;
    setup_buff(buff2, 6);

    if (buff.size() != 4)
        return 1;

    if (buff2.size() != 6)
        return 1;

    std::vector<uint32_t> checks = {1,2,3,4};
    buff.for_each(std::bind(visit_elem, std::placeholders::_1, std::ref(checks)));

    std::cout << "checks: " << checks.size() << "\n";
    for (auto& e : checks) {
        std::cout << e << " ";
    }
    std::cout << "\n";

    if (checks.size() > 0)
        return 1;

    checks = {0,1,2,3,4,5};
    buff2.for_each(std::bind(visit_elem, std::placeholders::_1, std::ref(checks)));

    std::cout << "checks: " << checks.size() << "\n";

    if (checks.size() > 0) {
        std::string s;
        for (auto& x : checks) {
            s += std::to_string(x) + " ";
        }
        std::cout << "Checks not empty\n elements: " << s << "\n";
        return 1;
    }

    
    return 0;
}