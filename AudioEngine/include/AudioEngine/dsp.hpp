#pragma once

#include <array>

#include "core.hpp"
#include "address.hpp"
#include "sockapi.hpp"
#include "shm.hpp"
#include "monitoring.hpp"

namespace AudioEngine {

    template <class T>
    concept dsp_plugin = requires (T t) {
        {t.init()};
    };

    template <dsp_plugin plugin>    
    class dsp {
    private:
        struct dsp_state {
            probe_service service;
        };

        dsp_state m_state;

    public:
        dsp() = default;
        ~dsp() = default;

        
    protected:
    private:
    };
}