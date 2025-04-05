#pragma once

#include "core.hpp"
#include <miniaudio.h>

namespace AudioEngine {
    template <class Deallocator, class T>
    concept has_deallocate = requires(Deallocator alloc, T* ptr, size_t size) {
        { alloc.deallocate(ptr, size) };
    };

    bool ma_call(ma_result res) {
        if (res == MA_SUCCESS)
            return true;

        throw AudioEngine::dsp_error(
            format("miniaudio call failed with reason: {}\n", ma_result_description(res))
        );
    }

    template <class MaType, auto Dtor, class Deallocator_t = std::default_delete<MaType>>
    class ma_wrapper {
        MaType *m_value;
        Deallocator_t m_deallocator; 

    public:
        ma_wrapper(MaType *val)
        :   m_value(val),
            m_deallocator(std::default_delete<MaType>{})
        {}

        ma_wrapper(MaType *val, Deallocator_t&& deallocator)
        :   m_value(val),
            m_deallocator(std::move(deallocator))
        {}

        ~ma_wrapper() {
            if ((bool)m_value) {
                Dtor(m_value);

                if constexpr (has_deallocate<Deallocator_t, MaType>)
                    m_deallocator.deallocate(m_value, 1);
                else
                    m_deallocator(m_value);

            }
        }

        operator bool() const noexcept {
            return (bool)m_value;
        }

        operator MaType&() {
            return *m_value;
        }

        operator MaType*() {
            return m_value;
        }

        MaType& operator*() {
            return *m_value;
        }
    };
}