#pragma once
#include "AudioEngine/core.hpp"
#include "AudioEngine/buffers/buffer.hpp" //buffer concept
#include <type_traits>
#include <memory>

template <class Allocator>
concept allocator_has_static_capacity = requires(Allocator a) {
    { Allocator::capacity } -> std::same_as<size_t>;
};

namespace AudioEngine {

    template <class CharType, class Allocator = std::allocator<CharType>, size_t InAlignment = 0>
    class text_buffer {
    public:
        using ValueType = CharType;
        using Alloc_T = typename std::allocator_traits<Allocator>::rebind_alloc<ValueType>;
        
        static constexpr Alignment = std::max(InAlignment, alignof(ValueType));
    private:
        Alloc_T m_allocator;
        ValueType *m_buffer;
        size_t m_size;
    
    public:
        text_buffer(size_t size, Allocator const& alloc) 
        :   m_allocator(alloc),
            m_buffer(m_allocator.allocate(size))
        {}

        //only works for default constructible allocators
        text_buffer(szie_t size) 
        :   m_allocator(Allocator()),
            m_buffer(m_allocator.allocate(size))
        {}

        ~text_buffer() {
            if constexpr (!std::is_trivial_v<ValueType>)
                for (size_t i = 0; i < m_size; i++) {
                    get(i).~ValueType();
                }
            
            m_allocator.deallocate(m_buffer, m_size);
        }

        [[nodiscard]] ValueType& get(size_t offset) const {
            if (offset >= m_size) [[unlikely]]
                throw std::runtime_error("Offset outside of buffer bounds");
            
            if constexpr (Alignment == 0)
                return m_buffer[offset];
            else
                return std::assume_aligned<Alignment>(m_buffer)[offset];
        }

        void reserve(size_t count) {
            m_buffer = std::assume_aligned<Alignment>(m_allocator.allocate(size));
            m_size = count;
        }

        void resize(size_t count) {
            //avoid leaks on throw before we release the new memory into 
            auto dealloc = [&](ValueType *elem) {m_allocator.deallocate(elem, size);};
            auto new_buffer = std::unique_ptr<ValueType, decltype(dealloc)>( std::assume_aligned<std::max(Alignment, alignof(ValueType))>(m_allocator.allocate(size))); 

            std::span<ValueType> dst_span(new_buffer.data(), size);

            memcpy(dst_span.data(), m_buffer, std::min(m_size, size));

            m_size = size;
        }

        [[nodiscard]] std::span<ValueType> view(size_t offset, size_t length) const {
            if (offset + length >= m_size) [[unlikely]]
                throw AudioEngine::dsp_error(format("view from {} to {} would exceed buffer (size={})", offset, offset + length, m_size));
            return std::span<ValueType>(&m_buffer.get(offset), &m_buffer.get(offset+length));
        }

        void store(size_t offset, std::span<ValueType> const& data) {
            if (offset + data.size() >= m_size) [[unlikely]]
                resize(offset + data.size());

            if constexpr (std::is_trivial_v<ValueType>) {
                std::memcpy(&m_buffer.get(offset), data.data(), data.size_bytes());
            }
            else {
                size_t counter = 0;
                for (auto& elem : data)
                    m_buffer.get(offset + counter++) = std::move(elem);
            }
        }

        void store(std::span<ValueType const& data) {
            store(0, data);
        }

        [[nodiscard]] size_t size() const noexcept {
            return m_size;
        }

        [[nodiscard]] size_t size_bytes() const noexcept {
            return m_size * sizeof(ValueType);
        }

        [[nodiscard]] ValueType *data() const noexcept { return m_buffer; }
    };


}