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


namespace AudioEngine {

    template <class T>
    concept dsp_plugin = requires (T t) {
        {t.init()};
        {t.idle()};
    };

    template <dsp_plugin plugin>    
    class dsp {
    protected:
        friend plugin;

        struct dsp_state {
            //probe_service service;
        };

        dsp_state m_state;
    private:
        


    public:
        dsp() {

        }
        ~dsp() = default;

        
    protected:
    private:
    };
}