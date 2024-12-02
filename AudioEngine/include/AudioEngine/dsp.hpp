#pragma once

#define _ISOC11_SOURCE

#include <array>
#include <tuple>
#include <fstream>
#include <cstdlib>

#include "core.hpp"
#include "address.hpp"
#include "sockapi.hpp"
#include "shm.hpp"
#include "block_allocator.hpp"
#include "monitoring.hpp"
#include "buffer_reader.hpp"
#include "config.hpp"


namespace AudioEngine {

    template <class PluginType>
    concept dsp_plugin = requires (PluginType t) {

        typename PluginType::cfg_parser_types;
        PluginType::shm_size_t;

        {t.init(&t)};
        {t.idle(&t)};
    };

    template <dsp_plugin Plugin>    
    class _dsp {
    public:
        using cfg_parser_types = tuple_combine_t<typename Plugin::cfg_parser_types, AudioEngine::base_cfg_parsers>;
        using shm_size_t = Plugin::shm_size_t;

    private:

    protected:
        friend Plugin;

        struct dsp_state {
            Memory::shm2mb< Memory::shm_size::MEGABYTEx256 > shm;
            AudioEngine::Monitoring::probe_service monitoring_service;

        };

        dsp_state m_state;

    public:
        _dsp() {

        }
        ~_dsp() = default;

        
    protected:
    private:
    };
}