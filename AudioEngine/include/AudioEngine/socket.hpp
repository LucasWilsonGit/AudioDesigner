#pragma once

#include <variant>
#include <optional>
#include "address.hpp"



namespace AudioEngine {
    class net_error : public std::runtime_error {
    private:
        std::string m_err;

    public:
        net_error(std::string&& err) : m_err(std::move(err)), std::runtime_error(err.c_str()) {}

        char const* what() { return m_err.c_str(); }
    };

    class socket {
        private:
            socket_t m_handle;
            std::optional<end_point> m_endpoint;

        public:

            socket_t platform_handle() const noexcept;
        protected:
        private:
    };
}