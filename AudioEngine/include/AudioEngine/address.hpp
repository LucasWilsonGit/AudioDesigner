#pragma once

#include <string>
#include <array>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <regex>
#include <variant>

#include "AudioEngine/sockapi.hpp"

namespace AudioEngine {
    using port_t = uint16_t;
    constexpr port_t PORT_ANY = 0;

    enum class IPVERSION {
        IPV4 = 0,
        IPV6 = 1,
        UNKNOWN = 2
    };

    struct address_ipv4 {
    //data
    private:
        std::array<std::byte, 4> m_bytes;

    public:
        explicit address_ipv4(uint32_t bytes) {
            uint8_t i = 0;
            for (uint8_t *p_bytes = reinterpret_cast<uint8_t*>(&bytes); i < 4; p_bytes++, i++) {
                m_bytes[i] = std::byte{*p_bytes};    
            }
        }
        explicit address_ipv4(std::array<std::byte, 4> bytes) : m_bytes(std::move(bytes)) {}
        address_ipv4(std::string const& addr) : m_bytes(parse_ipv4(addr)) {}
        address_ipv4(::in_addr const& addr) : m_bytes(parse_ipv4(addr)) {}
        
        bool is_multicast() const noexcept {
            uint8_t byte = std::to_integer<uint8_t>(m_bytes[0]);
            return  byte >= 224 && byte < 240;
        }

        std::string display_string() const {
            std::string result;
            for (auto& byte : m_bytes) {
                uint16_t num = std::to_integer<uint16_t>(byte);
                result += (result.size() == 0 ? "" : ".") + std::to_string(num);
            }
            return result;
        }

        std::array<std::byte, 4> bytes() const {
            return m_bytes;
        }

    protected:
    private:
    };

    class address_ipv6 {
    private:
        std::array<std::byte, 16> m_bytes;

    public:
        explicit address_ipv6(std::array<std::byte, 16> bytes) : m_bytes(std::move(bytes)) {}
        address_ipv6(std::string const& addr) : m_bytes(parse_ipv6(addr)) {}
        address_ipv6(::in6_addr const& addr) : m_bytes(parse_ipv6(addr)) {}

        bool is_multicast() const noexcept {
            return std::to_integer<uint8_t>(m_bytes[0]) == 0xff;
        }

    protected:
    private:
    };

    class end_point {
    private:        
        std::variant<address_ipv4, address_ipv6> m_address;
        port_t m_port;

    public:
        end_point(std::variant<address_ipv4, address_ipv6> const& addr, port_t port = PORT_ANY) noexcept :
            m_address(addr),
            m_port(port)
        {}


    };
}