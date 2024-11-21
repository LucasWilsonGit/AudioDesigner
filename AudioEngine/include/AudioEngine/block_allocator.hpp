#pragma once
#include <new>
#include <memory>
#include <cstring>
#include <iostream>
#include "core.hpp"


namespace AudioEngine {

    template <size_t M, size_t N>
    concept equal_size = (M == N);

    template <class T>
    struct base_alloc_traits {
        using value_type = T;
        using pointer = T*;
        using const_pointer = T const*;
        using reference = T&;
        using const_reference = T const&;
        using size_type = std::size_t;
        using different_type = std::ptrdiff_t;
    }; 

    template <size_t N>
    struct block_allocator_data_storage {
        alignas(32) std::array<uint8_t, N> markers {0};
        void* buf = nullptr; //N instances of T externally allocated, this just manages suballocations but does not own the memory

        size_t elem_size;
        uint8_t alignment;
        

        block_allocator_data_storage(uint8_t align, size_t esize)
        :   alignment(align), elem_size(esize)
        {
            markers.fill(0);
        }
    };

    template <class T, size_t N>
    class block_allocator : public base_alloc_traits<T> {
    public:
        using data_storage = block_allocator_data_storage<N>;
        static constexpr size_t capacity = N;
    private:

        bool is_ptr_aligned(void* ptr, size_t alignment) {
            if (alignment & (alignment - 1) != 0) {
                throw std::logic_error("Attempt to test ptr alignment to non power of two alignment parameter");
            }
            return (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0;
        }

        size_t find_contiguous_blocks_idx(size_t count) {
            for (size_t i = 0; i < N; i++) {
                auto& marker = m_p_storage->markers[i];
                if (marker == 0) {
                    for (size_t offs = 1; offs < count; offs++) {
                        if (m_p_storage->markers[i+offs] != 0) {
                            goto skip;
                        }
                    }
                    return i;
                }
                skip:
                (void)marker;
            }

            throw std::bad_alloc();
        }
    protected:
        template <class U, size_t M>
        friend class block_allocator;

        

        std::shared_ptr<data_storage> m_p_storage;

        std::shared_ptr<data_storage> const get_storage() const noexcept {
            return m_p_storage;
        }

    public:

        block_allocator(void* addr) {
            uintptr_t uaddr = reinterpret_cast<uintptr_t>(addr);
            if (!is_ptr_aligned(addr, alignof(T)))
                throw std::runtime_error("Attempt to initialize block allocator to incorrectly aligned memory");
                
            m_p_storage = std::make_shared<data_storage>(alignof(T), sizeof(T));
            m_p_storage->buf = reinterpret_cast<T*>(addr);
        }

        //copy constructor
        block_allocator(block_allocator const& other) noexcept : m_p_storage(other.m_p_storage) {}
        //copy assignment
        block_allocator& operator=(block_allocator const& other) noexcept { 
            m_p_storage = other.m_p_storage;
            return *this;
        }
        block_allocator& operator=(block_allocator&& other) noexcept {
            m_p_storage = std::move(other.m_p_storage);
            return *this;
        }

        template <class T2>
        constexpr bool operator==(block_allocator<T2, N> const& other) noexcept {
            return m_p_storage == other.m_p_storage;
        }


        template <class U>
        block_allocator(block_allocator<U, N> const& other) noexcept : m_p_storage(other.m_p_storage) {}

        template <typename U>
        struct rebind {
            using other = block_allocator<U, N>;
        };

        //returns uninitialized aligned memory for `count` instances of `T` 
        T* allocate(size_t count) {
            size_t desired_size = count * sizeof(T) + (alignof(T) - 1); //pad alignment
            size_t num_blocks = ceil_div(desired_size,  m_p_storage->elem_size); //calculate num of blocks to contain T aligned desired_sizr
            size_t start_idx = find_contiguous_blocks_idx(num_blocks);

            for (size_t i = 0; i < num_blocks; i++) {
                m_p_storage->markers[start_idx + i] = num_blocks - i;
            }

            return reinterpret_cast<T*>(&reinterpret_cast<std::byte*>(m_p_storage->buf)[start_idx * m_p_storage->elem_size]);
        }

        void deallocate(T* elem, size_t count) {
            if (!elem) [[unlikely]] {
                throw std::runtime_error("Attempt to deallocate nullptr");
            }
            else if (count == 0) [[unlikely]] {
                throw std::runtime_error("Attempt to deallocate 0 instances");
            }
            
            uintptr_t buff_start = reinterpret_cast<uintptr_t>(m_p_storage->buf);
            uintptr_t elem_addr = reinterpret_cast<uintptr_t>(elem);
            intptr_t offs = elem_addr - buff_start;

            if (elem_addr < buff_start || offs > static_cast<intptr_t>(N * m_p_storage->elem_size)) {
                throw std::out_of_range("Attempt to deallocate pointed to memory outside of block allocator managed bounds");
            }

            size_t block_idx = offs / m_p_storage->elem_size;
            size_t desired_size = count * sizeof(T) + (alignof(T) - 1); //pad alignment
            size_t num_blocks = ceil_div(desired_size,  m_p_storage->elem_size); //calculate num of blocks to contain T aligned desired_sizr

            if (block_idx + num_blocks >= N)
                throw std::runtime_error("deallocation would overrun buffer boundary");

            size_t real_blocks = m_p_storage->markers[block_idx];
            if (real_blocks != num_blocks)
                throw Memory::memory_error( format("block deallocation mismatch between tracked and calculated sizes: tracked {} calculated {}", real_blocks, num_blocks) );

            for (size_t i = 0; i < real_blocks; i++) {
                m_p_storage->markers[block_idx + i] = 0;
            }
        }
    };
}