#pragma once

#include "core.hpp"
#include <cstring>
#include <stdexcept>

namespace AudioEngine {

    constexpr bool is_power_of_two(size_t n) {
        return (n!=0) && ((n & (n-1)) == 0);
    }

    template <class T, size_t N, class alloc_t = std::allocator<T>>
    class ring_buffer{
        static_assert(is_power_of_two(N), "ring_buffer size must be a power of two");

        alloc_t m_alloc;
        T *m_data;
        size_t m_start = 0;
        size_t m_count = 0;

    public:

        ring_buffer() : m_alloc(alloc_t()) {
            m_data = m_alloc.allocate(N);
        }
        ring_buffer(alloc_t const& allocator) : m_alloc(allocator) {
            m_data = m_alloc.allocate(N);
        }

        void append(T&& value) noexcept {
            if (m_count < N) {
                m_data[wrap_index(m_start + m_count)] = std::forward<T>(value);
                m_count++;
            }
            else {
                m_data[m_start] = std::forward<T>(value);
                m_start = wrap_index(m_start+1);
            }
        }

        void pop_front() {
            if (m_count == 0) 
                throw dsp_error("Attemt to remove element from empty ring buffer");
            
            m_start = wrap_index(m_start+1);
            m_count--;
        }

        void pop_back() {
            if (m_count == 0)
                throw dsp_error("Attempt to remove element from empty ring buffer");
            
            m_count--;
        }

        size_t size() const noexcept {
            return m_count;
        }

        T& front() {
            if (m_count == 0)
                throw dsp_error("Attempt to access element of empty ring buffer");
            
            return m_data[m_start];
        }

        T& back() {
            if (m_count == 0)
                throw dsp_error("Attempt to access element of empty ring buffer");
            
            return m_data[wrap_index(m_start + m_count - 1)];
        }

        T& at(size_t index) const {
            if (index >= m_count) {
                throw std::out_of_range("Index out of range");
            }

            return m_data[wrap_index(m_start + index)];
        }

        T* const& collection() const noexcept {
            return m_data;
        }

        size_t capacity() const noexcept {
            return N;
        }

        template <class Callable> 
        void for_each(Callable c) {
            auto [left, right] = get_wrap_counts();

            auto visit_set = [&](size_t begin, size_t length) {
                if (length == 0)
                    throw std::runtime_error("trying to iterate 0 length range");

                for (size_t offs = begin; offs < length; offs++) {
                    auto& elem = m_data[begin + offs];
                    c(elem);
                }
            };

            visit_set(m_start, left);
            if (right)
                visit_set(0, right.value());
        }



    protected:
        size_t wrap_index(size_t index) const {
            return index & (N-1);
        }

        /**
         * Returns [left, right]
         * left: Number of elements from m
         */
        std::pair<size_t, std::optional<size_t>> get_wrap_counts() {
            if (m_count == 0) {
                throw std::runtime_error("Attempt to find wrapped chunk lengths on an empty ringbuffer");
            }
            
            size_t end_index = m_start + m_count;

            if (end_index <= N) {
                return {m_count, std::nullopt};
            }
            else {
                size_t pre_wrap = capacity() - m_start;
                size_t post_wrap = m_count - pre_wrap;

                return {pre_wrap, post_wrap};
            }
        }
    };
}