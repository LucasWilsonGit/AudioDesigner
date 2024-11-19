#include "AudioEngine/sockapi.hpp"
#include "AudioEngine/address.hpp"
#include "AudioEngine/shm.hpp"
#include "AudioEngine/monitoring.hpp"
#include "AudioEngine/dsp.hpp"

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
        decltype(service)::probe_handle_t probe_handle = service.add_probe(pd);

        std::cout << "probe is: " << *(uint64_t*)&probe_handle << "\n";

        if (!service.send_probe_value("SETUP_PROBE", 12ll))
            throw std::runtime_error("Failed to send probe value");
        
        auto& datas = service.get_probe_data(probe_handle);
        std::cout << datas.back().value << "\n";

        char currentDir[MAX_PATH];
        DWORD result;
        if ( (result = GetCurrentDirectoryA(MAX_PATH, currentDir)) != 0 )
            std::cout << "CWD: " << currentDir << "\n";
        else
            std::cout << "Failed to read CWD\n";

        AudioEngine::dsp_cfg_parser< AudioEngine::dsp_cfg_bool_parser_impl > parser(std::string(currentDir) + "/conf.cfg");
        
        for (auto& e : parser.get_config_fields()) {
            std::cout << e.first << "\n";
        }
    }
    catch (Net::net_error const& e) {
        std::cout << e.what() << "\n";
    }

    Net::cleanup();

    return 0;
}