#pragma once

#include "buffer.hpp"
#include <ios>
#include <span>
#include <string.h> //memcpy_s
#include <type_traits>
namespace AudioEngine {
    template <dsp_buffer BufferT>
    class buffer_writer {
        BufferT& m_buffer;
        std::streampos m_pos;
        static constexpr size_t Alignment = BufferT::Alignment;

        using ValueType = typename BufferT::ValueType;
    public:
        buffer_writer(BufferT& buff)
        :   m_buffer(buff),
            m_pos(0)
        {}

        buffer_writer<BufferT>& operator<<(ValueType const& v) {
            m_buffer.get(m_pos++) = v;

            return *this;
        }

        buffer_writer<BufferT>& operator<<(std::span<ValueType> const& v) {
            ValueType *data = m_buffer.data();

            m_buffer.store(m_pos, v);

            return *this;
        }

        [[nodiscard]] operator bool() const noexcept {
            return m_pos < m_buffer.size();
        }
    };  

    template <dsp_buffer BufferT>
    class buffer_reader {
        BufferT const& m_buffer;
        std::streampos m_pos;

        using ValueType = typename BufferT::ValueType;
    public:
        buffer_reader(BufferT const& buff)
        :   m_buffer(buff),
            m_pos(0)
        {}

        buffer_reader<BufferT>& operator>>(ValueType& dst) {
            dst = m_buffer.get(m_pos++);
            return *this;
        }

        buffer_reader<BufferT>& operator>>(std::span<ValueType>& dst) {
            ValueType *data = m_buffer.data();
            std::span<ValueType> dataspan = std::span<ValueType>(data, m_buffer.size());

            int64_t headroom = m_buffer.size() - m_pos;

            int64_t left = std::min(headroom, (int64_t)dst.size());
            auto leftspan = dataspan.subspan(m_pos, left);
            
            //copy left into dst
            if (dst.size_bytes() < leftspan.size_bytes()) [[unlikely]]
                throw Memory::memory_error(format("Copying {} into buffer of size {} bytes\n", leftspan.size_bytes(), dst.size_bytes()));
            std::memcpy(dst.data(), leftspan.data(), leftspan.size_bytes());

            int64_t right = (int64_t)dst.size() - left;
            if (right > 0) {
                auto rightspan = dataspan.subspan(0, right);
                std::memcpy(&dst[left], rightspan.data(), rightspan.size_bytes());
            }

            //std::cout << format("Read pcm into dst {} samples m_pos {} headroom {} left {} right {}\n", dst.size(), (int64_t)m_pos, headroom, left, right);

            m_pos = ((int64_t)m_pos + dst.size()) % m_buffer.size(); //optimization: constrain size to power of 2
            return *this;
        }

        [[nodiscard]] operator bool() const noexcept {
            return true;
        }
    };
}