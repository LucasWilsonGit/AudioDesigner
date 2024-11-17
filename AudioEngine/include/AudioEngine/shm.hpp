#pragma once
/**
 * @name    shm.hpp
 * @author  Lucas Wilson
 * @date    07/10/2024
 * @brief   Provides a platform independent interface for mapping named memory
 */
#ifdef _WIN32

    using map_handle_t = void*;
#else
    #include <sys/mman.h>

    using map_handle_t = void*;
#endif

#include <memory>
#include <concepts>
#include <string>
#include <array>

#include "AudioEngine/core.hpp"

//todo: implement some allocator ontop of the preallocated memory
//potential improvements: Reserve but do not commit pages

namespace Memory {

    constexpr size_t pagesize_2MB = 1024 * 1024 * 2;
    constexpr size_t pagesize_4MB = 1024 * 1024 * 4; 

    template <size_t PageSize>
    struct page {
        std::byte bytes[PageSize];
    };

    struct mapping {
        std::string const name;
        map_handle_t handle;
        size_t size;
        void* data;
    };

    enum class shm_size : uint64_t {
        KILOBYTE = 1,
        MEGABYTEx256 = 2,
        MEGABYTEx512 = 3,
        GIGABYTE = 4,
        GIGABYTEx10 = 5
    };

    template<shm_size size>
    struct shm_size_getter {
    };

#define DEF_SHM_SIZE(sizeenum, val) \
template<>                          \
struct shm_size_getter<sizeenum> {  \
    static constexpr size_t value = val;\
}                                   

    DEF_SHM_SIZE(shm_size::KILOBYTE, 1024ull);
    DEF_SHM_SIZE(shm_size::MEGABYTEx256, 256*1024*1024ull);
    DEF_SHM_SIZE(shm_size::MEGABYTEx512, 512*1024*1024ull);
    DEF_SHM_SIZE(shm_size::GIGABYTE, 1024*1024*1024ull);
    DEF_SHM_SIZE(shm_size::GIGABYTEx10, 10*1024*1024*1024ull);

    template <class T>
    concept mmap_impl_interface = requires(T t) {
        { t.init() };
        { t.create("foo", 0LL, 0U) } -> std::same_as<mapping>;
        { t.release(mapping{"foo", 0LL, 0U}) };
        { t.data(mapping{"foo", 0LL, 0U}) } -> std::same_as<void*>;
        { T::page_size };
    };

    std::string make_platform_name(std::string const& name);

    /**
     * @brief               static method for making the file mapping
     * @todo                NUMA support
     * @param name          Should be the platform-specific name of the memory mapping
     * @param size          size of memory to map, must be a multiple of `get_page_size`
     * @param access_flag   one of the page access flags such as `PAGE_READONLY`, `PAGE_READWRITE`, `PAGE_EXECUTE_READ`, etc...
     */
    mapping make_mapping(std::string const& name, size_t size, uint32_t access_flag);

    template <mmap_impl_interface MapInterface, shm_size shm_size_t>
    class _shm {
    private:
        using api = MapInterface;

        mapping m_mapping;

    public:
        static constexpr size_t page_size = api::page_size;

        _shm(std::string name, uint32_t access_flags) :
            m_mapping(api::create(name, size(), access_flags))
        {}

        ~_shm() {
            api::release(m_mapping);
        }

        std::string_view const name() const noexcept { return m_mapping.name; };
        size_t size() noexcept { return shm_size_getter<shm_size_t>::value; };

        void* data() noexcept { return m_mapping.data; }
        void* get_page(size_t page_idx) noexcept { 
            if (page_idx >= (size() / MapInterface::page_size) ) [[unlikely]]  {
                throw std::out_of_range("Attempt to get_page(size_t page_idx) outside of shm bounds");
            }
            return reinterpret_cast<page<MapInterface::page_size> *>(m_mapping.data)[page_idx]; 
        }


    protected:
        map_handle_t handle() noexcept { return m_mapping.handle; };
        map_handle_t chandle() const noexcept { return m_mapping.handle(); };

    private:
    };


#ifdef _WIN32
    /**
     * @brief Implements a global init (e.g enable privileges on Windows)
     * Also implements some basic methods for making the mapping, releasing it and getting the data address.
     */
    template <size_t PageSize = 1024 * 1024 * 2> //assume 2MB large pages, they may be 4MB on some windows platforms.
    struct win32mmapapi {
        static constexpr size_t page_size = PageSize;
        
        static bool init();
        static mapping create(std::string const& name, size_t size, uint32_t flags);
        static void release(mapping const&);
        static void* data(mapping const&) noexcept;
    };

    template <shm_size size>
    using shm2mb = _shm<win32mmapapi<1024 * 1024 * 2>, size>;

    template <shm_size size>
    using shm4mb = _shm<win32mmapapi<1024 * 1024 * 4>, size>;
#else

#endif
}