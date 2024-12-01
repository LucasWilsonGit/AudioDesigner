#pragma once

#include "core.hpp"
#include "miniaudio/miniaudio.h"

namespace AudioEngine {
    bool ma_call(ma_result res) {
        if (res == MA_SUCCESS)
            return true;

        throw AudioEngine::dsp_error(
            format("miniaudio call failed with reason: {}\n", ma_result_description(res))
        );
        return false;
    }

    template <class MaType, auto Dtor>
    class ma_wrapper {
        std::unique_ptr<MaType> m_value;
        decltype(Dtor) m_dtor = Dtor;

    public:
        ma_wrapper(MaType *val)
        :   m_value(std::unique_ptr<MaType>(val))
        {}

        ~ma_wrapper() {
            if ((bool)m_value)
                m_dtor(m_value.get());
        }

        operator bool() const noexcept {
            return (bool)m_value;
        }

        operator MaType&() {
            return *m_value;
        }

        operator MaType*() {
            return m_value.get();
        }

        MaType& operator*() {
            return *m_value;
        }
    };
}