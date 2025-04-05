#pragma once

#include <memory>
#include <optional>
#include <span>

namespace AudioEngine {
    template <class SampleType, class Allocator = std::allocator<SampleType>>
    class pcm_buffer {
    public:
        using Alloc_T = typename std::allocator_traits<Allocator>::template rebind_alloc<SampleType>;
        using ValueType = SampleType;
    private:
        std::optional<Alloc_T> m_allocator;
        SampleType *m_buffer;
        uint8_t m_channels;
        size_t m_frame_count;
        size_t m_size;
        
    public:

        pcm_buffer(uint8_t channels, size_t frame_count, Alloc_T const& alloc)
        :   m_allocator(alloc),
            m_buffer(m_allocator->allocate(channels * frame_count)),
            m_channels(channels),
            m_frame_count(frame_count),
            m_size(m_channels * m_frame_count)
        {}

        ~pcm_buffer() {
            m_allocator->deallocate(m_buffer, m_size);
        }

        ValueType& get(size_t offset) const {
            if (offset >= m_size) [[unlikely]]
                throw std::runtime_error("Offset outside of buffer");
            return m_buffer[offset];
        }

        std::span<ValueType> view(size_t offset, size_t length) const {
            if ((offset + length) >= m_size)
                throw AudioEngine::dsp_error(format("view from {} to {} would exceed buffer ({} samples)", offset, offset + length, m_size));
            return std::span<ValueType>(m_buffer[offset], m_buffer[offset+length]);
        }

        void store(size_t offset, std::span<ValueType> const& data) {
            if constexpr (std::is_trivially_constructible_v<ValueType>) {
                std::memcpy(&m_buffer[offset], data.data(), data.size_bytes());
            }
            else {
                if (offset + data.size() >= m_size)
                    throw std::out_of_range("offset + data.size() >= m_size");

                size_t counter = 0;
                for (auto& elem : data) {
                    m_buffer[offset + counter] = std::move(elem);
                }
            }
        }

        void store(std::span<ValueType> const& data) {
            store(0, data);
        }

        [[nodiscard]] size_t size() const noexcept {
            return m_size;
        }

        [[nodiscard]] size_t size_bytes() const noexcept {
            return size() * sizeof(ValueType);
        }

        SampleType *data() const noexcept { return m_buffer; }
    };
}