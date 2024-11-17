#include "AudioEngine/sockapi.hpp"
#include "AudioEngine/address.hpp"
#include "AudioEngine/shm.hpp"
#include "AudioEngine/monitoring.hpp"

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <limits>

int main() {
    //If this fails then we might as well just exit.
    Net::init();

    try {
        Net::address_ipv4 addr{"127.0.0.1"};
        Net::end_point ep(addr, 10681);
        
        using shm_t = Memory::_shm<
            Memory::win32mmapapi<Memory::pagesize_2MB>, 
            Memory::shm_size::MEGABYTEx256
        >;

        shm_t shm("256mb_storage_dsp_basic", PAGE_READWRITE);
        AudioEngine::Monitoring::probe_service service(shm.data(), shm_t::page_size);
        
        AudioEngine::Monitoring::probe_description pd{.name = "SETUP_PROBE", .unit = "", .flags = 0};
        decltype(service)::probe_t *probe = service.add_probe(pd);

        if (!service.send_probe_value("SETUP_PROBE", 12ll))
            throw std::runtime_error("Failed to send probe value");

    }
    catch (Net::net_error const& e) {
        std::cout << e.what() << "\n";
    }

    Net::cleanup();

    return 0;
}