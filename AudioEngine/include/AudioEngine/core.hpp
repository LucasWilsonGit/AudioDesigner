#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace Net {
#ifdef _WIN32
    using socket_t = unsigned long long;
#else
    using socket_t = int;
#endif


    class net_error : public std::runtime_error {
    private:
        std::string m_err;

    public:
        net_error(std::string&& err) : m_err(std::move(err)), std::runtime_error(err.c_str()) {}

        char const* what() const noexcept { return m_err.c_str(); }
    };

}