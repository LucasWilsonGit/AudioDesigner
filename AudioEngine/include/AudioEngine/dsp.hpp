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
        typename PluginType::cfg_char_t;

        PluginType::shm_size_t;

        {t.init(&t)};
        {t.idle(&t)};
    };

    struct dsp_state {
        Memory::_shm< Memory::win32mmapapi<Memory::pagesize_2MB>, Memory::shm_size::MEGABYTEx256 > shm_storage;
        AudioEngine::Monitoring::probe_service monitoring_service;
    };

    template <dsp_plugin Plugin>    
    class _dsp {
    public:
        using cfg_parser_types = tuple_combine_t<typename Plugin::cfg_parser_types, AudioEngine::base_cfg_parsers>;
        using shm_size_t = Plugin::shm_size_t;
        using cfg_t = typename dsp_cfg_parser_from_parsers<typename Plugin::cfg_char_t, cfg_parser_types>::dsp_cfg_t;
        using cfg_parser_t = dsp_cfg_parser_from_parsers<typename Plugin::cfg_char_t, cfg_parser_types>::dsp_cfg_parser_t ;
    private:
        
        cfg_t load_config(std::string_view const& path) const {
            cfg_parser_t parser(path);
        }

    protected:
        friend Plugin;

        dsp_state m_state;

    public:
        _dsp() {

        }
        ~_dsp() {}

        
    protected:
    private:
    };
}