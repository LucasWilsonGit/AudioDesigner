#pragma once

#include <string>
#include <array>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <regex>
#include <variant>
#include <compare>
#include <cstring>

#include "AudioEngine/sockapi.hpp"

namespace Net {
    // Concept to check if two types can be compared with ==
    template <typename T1, typename T2>
    concept equality_comparable = requires(T1 a, T2 b) {
        { a == b } -> std::convertible_to<bool>;
    };

    //fdecl to be friend of ipv4 ipv6
    class end_point;

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
        //tested
        explicit address_ipv4(uint32_t bytes) {                                                         //tested
            uint8_t i = 0;
            for (uint8_t *p_bytes = reinterpret_cast<uint8_t*>(&bytes); i < 4; p_bytes++, i++) {
                m_bytes[i] = std::byte{*p_bytes};    
            }
        }
        explicit address_ipv4(std::array<std::byte, 4> bytes) : m_bytes(std::move(bytes)) {}            //tested
        address_ipv4(std::string const& addr) : m_bytes(parse_ipv4(addr)) {}                            //tested
        address_ipv4(::in_addr const& addr) : m_bytes(parse_ipv4(addr)) {}                              //tested
        
        bool is_multicast() const noexcept {                                                            //TODO: write unit test
            uint8_t byte = std::to_integer<uint8_t>(m_bytes[0]);
            return  byte >= 224 && byte < 240;
        }

        std::string display_string() const;                                                             //tested        (uses inet_ntop)

        std::array<std::byte, 4> bytes() const noexcept {                                               //tested
            return m_bytes;
        }

        std::byte const* data() const noexcept {                                                        //tested by bytes()
            return m_bytes.data();
        }

        uint32_t number() const noexcept;

        bool operator==(address_ipv4 const& other) const noexcept {
            return std::memcmp(m_bytes.data(), other.peek_bytes().data(), m_bytes.size()) == 0;
        }

    protected:
        friend class end_point;

        std::array<std::byte, 4> const& peek_bytes() const noexcept {                                   //tested by bytes()
            return m_bytes;
        }

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

        std::string display_string() const; //defined by sockapi (I know, this is a bit weird but we need to use inet_ntop)

        std::array<std::byte, 16> bytes() const noexcept {                                              //tested
            return m_bytes;
        }

        std::byte const* data() const noexcept {                                                        //tested by bytes()
            return m_bytes.data();
        }

        bool operator==(address_ipv6 const& other) const noexcept {
            return std::memcmp(m_bytes.data(), other.peek_bytes().data(), m_bytes.size()) == 0;
        }

    protected:
        friend class end_point;

        std::array<std::byte, 16> const& peek_bytes() const noexcept {                                  //tested by bytes()
            return m_bytes;
        }
    private:
    };

    struct end_point {       
        std::variant<address_ipv4, address_ipv6> address;
        port_t port;

    public:
        end_point(std::variant<address_ipv4, address_ipv6> const& addr, port_t in_port = PORT_ANY) noexcept :
            address(addr),
            port(in_port)
        {}

        //gets a sockaddr_storage (struct is identical on windows and linux I think?) also returns the size of the struct because it's a very common use pattern to need to pass the size of a struct sockaddr* 
        //defined in sockapi_$PLATFORM.cpp
        std::pair<::sockaddr_storage, size_t> get_sockaddr() const;

        struct address_string_visitor {
            __forceinline std::string operator()(address_ipv4 const& addr) {
                return addr.display_string();
            }
            __forceinline std::string operator()(address_ipv6 const& addr) {
                return addr.display_string();
            }
        };

        operator std::string() const {
            return std::visit(address_string_visitor{}, address) + ":" + std::to_string(port);
        }
        friend std::ostream& operator<<(std::ostream& out, end_point const& obj) {
            out << static_cast<std::string>(obj);
            return out;
        }

        auto operator==(end_point const& other) const noexcept {
            if (port != other.port)
                return false;

            return std::visit(
                [](const auto& lhs, const auto& rhs) -> bool {
                    if constexpr (equality_comparable<decltype(lhs), decltype(rhs)>) 
                        return lhs == rhs;
                    else {
                        return false;
                    }
                }, 
                address, 
                other.address
            );
        }
    };
}