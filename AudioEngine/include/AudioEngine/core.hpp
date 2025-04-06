#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <concepts>
#include <format>
#include <cmath>
#include <iostream>
#include <memory>

#include "template_magic.hpp"

/**
 * @brief A non-owning pointer to some externally managed resource which may be deallocated whilst this weak handle is held
 */
template <class T>
using nonowning_ptr = T*; 

template<class T>
concept stringlike = requires (T t) {
    std::string(t);
};

template <class... Ts>
std::string format(stringlike auto fmtstr, Ts&&... args) {
    return std::vformat(fmtstr, std::make_format_args(args...));
}

template <class T>
auto ceil_div(T const& l, T const& r) {
    return (l + r-1) / r;
}

/*
Unused after I didn't enforce is_power_of_two on containers sizes
constexpr bool is_power_of_two(size_t n) {
    return (n!=0) && ((n & (n-1)) == 0);
}
*/

// Helper to create overloads
template<class... Ts> struct overloads : Ts... { using Ts::operator()...; };
// Deduction guide for overloads
template<class... Ts> overloads(Ts...) -> overloads<Ts...>;

template <auto *funcptr_inst>
struct function_ptr_traits {};

template <class ReturnType, class... ArgTypes, auto (*funcptr_inst)(ArgTypes...) -> ReturnType>
struct function_ptr_traits<funcptr_inst> {
    using arg_types = std::tuple<ArgTypes...>;
    using return_type = ReturnType;
};


namespace AudioEngine {
    //base error type for AudioEngine
    class dsp_error : public std::runtime_error {
    private:
        std::string m_err;

    public:
        dsp_error(std::string&& err) : std::runtime_error(err.c_str()), m_err(std::move(err)) {}

        char const* what() const noexcept { return m_err.c_str(); }
    };
}

#define REGISTER_AUDIOENGINE_ERROR(x, y)                                \
class x : public y {                                                    \
public:                                                                 \
    x(std::string&& err) : y(std::move(err)) {}                         \
}

namespace Net {
#ifdef _WIN32
    using socket_t = unsigned long long;
#else
    using socket_t = int;
#endif

    REGISTER_AUDIOENGINE_ERROR(net_error, AudioEngine::dsp_error); 
    REGISTER_AUDIOENGINE_ERROR(would_block_error, net_error);
    
}

namespace Memory {
    REGISTER_AUDIOENGINE_ERROR(memory_error, AudioEngine::dsp_error);

    /**
     * @brief Gets the platform specific error messages, e.g FormatMessage on Windows platforms
     */
    class memory_platform_error : public memory_error {
    public:
        memory_platform_error(std::string&& err) : memory_error(err + " " + get_error_msg()) {}
    private:
        std::string get_error_msg(); //platform specific
    };
}

namespace AudioEngine {
    REGISTER_AUDIOENGINE_ERROR(cfg_parse_error, AudioEngine::dsp_error);

    struct alignas(16) s16 {
        uint64_t _a[2];
    };





    

    
}

