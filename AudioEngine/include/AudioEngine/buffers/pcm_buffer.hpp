#pragma once

#include <memory>
#include <optional>

namespace AudioEngine {
    template <class SampleType, class Allocator = std::allocator<SampleType>, size_t InAlignment = 0>
    class pcm_buffer {
    public:
        static constexpr size_t Alignment = std::max(InAlignment, alignof(SampleType));
        using Alloc_T = typename std::allocator_traits<Allocator>::template rebind_alloc<SampleType>;
        using ValueType = SampleType;

        
    private:
        Alloc_T m_allocator;
        SampleType *m_buffer;
        uint8_t m_channels;
        size_t m_frame_count;
        size_t m_size;
        
    public:

        pcm_buffer(uint8_t channels, size_t frame_count, Allocator const& alloc)
        :   m_allocator(alloc),
            m_buffer(m_allocator.allocate(channels * frame_count)),
            m_channels(channels),
            m_frame_count(frame_count),
            m_size(m_channels * m_frame_count)
        {}

        ~pcm_buffer() {
            if constexpr (!std::is_trivially_destructible_v<ValueType>)
                for (size_t i = 0; i < size(); i++) {
                    get(i).~ValueType();
                }

            m_allocator.deallocate(m_buffer, m_size);
        }

        [[nodiscard]] ValueType& get(size_t offset) const {
            if (offset >= m_size) [[unlikely]]
                throw std::runtime_error("Offset outside of buffer");
            
            if constexpr (Alignment == 0)
                return m_buffer[offset];
            else
                return std::assume_aligned<Alignment>(m_buffer)[offset];
        }

        [[nodiscard]] std::span<ValueType> view(size_t offset, size_t length) const {
            if ((offset + length) >= m_size) [[unlikely]]
                throw AudioEngine::dsp_error(format("view from {} to {} would exceed buffer ({} samples)", offset, offset + length, m_size));
            return std::span<ValueType>(&m_buffer.get(offset), &m_buffer.get(offset+length));
        }

        void store(size_t offset, std::span<ValueType> const& data) {
            if (offset + data.size() >= m_size) [[unlikely]]
                throw std::out_of_range("offset + data.size() >= m_size");


            if constexpr (std::is_trivial_v<ValueType>) {
                std::memcpy(&m_buffer.get(offset), data.data(), data.size_bytes());
            }
            else {
                size_t counter = 0;
                for (auto& elem : data) {
                    m_buffer.get(offset + counter++) = std::move(elem);
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

        [[nodiscard]] SampleType *data() const noexcept { return m_buffer; }
    };
}