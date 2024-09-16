#pragma once

#include <cstdint>
#include <variant>
#include <concepts>
#include <array>
#include <type_traits>
#include <string>
#include <utility>
#include <optional>

struct in_addr;
struct in6_addr;
struct sockaddr;

namespace AudioEngine {
    //fdecl socket.hpp class socket
    class socket;
    class address_ipv4;
    class address_ipv6;
    class end_point;

#ifdef _WIN32
    using socket_t = uintptr_t;
#else
    using socket_t = int;
#endif
    template <std::integral T>
    constexpr std::array<std::byte, sizeof(T)> to_bytes(T value) {
        using ValueType = std::conditional_t<   
                            std::is_const_v<std::remove_reference_t<T>>,
                                std::remove_const_t<T>, 
                                T>;

        std::array<std::byte, sizeof(T)> buffer alignas(T);
        *reinterpret_cast<ValueType*>(buffer.data()) = value;
        return buffer;
    }

    constexpr std::array<std::byte, 4> to_bytes(::in_addr const& value);
    constexpr std::array<std::byte, 16> to_bytes(::in6_addr const& value);

    //inits WSA on windows, should do nothing in linux.
    void init();

    //uses inet_pton
    std::array<std::byte, 4> parse_ipv4(std::string const& addr);
    //uses to_bytes<T>(T);
    std::array<std::byte, 4> parse_ipv4(::in_addr const& addr);

    //uses inet_pton
    std::array<std::byte, 16> parse_ipv6(std::string const& addr);
    //uses some possibly unsafe pointer voodoo. Alignment and lifetime rules should not be violated though so I think it ought to be standards compliant.
    std::array<std::byte, 16> parse_ipv6(::in6_addr const& addr);

    //get an end_point from a sockaddr
    end_point parse_end_point(::sockaddr const& addr);

    //blocks until sock receives a connection request, then accepts the connection and returns the connecting socket
    std::pair<socket_t, end_point> accept(socket_t sock);

    //binds the passed socket to the passed endpoint
    void bind(socket_t sock, end_point const& endpoint);

    //closes the passed socket
    void close(socket_t sock);

    //opens a connection to the peer
    void connect(socket_t sock, end_point const& endpoint);

    //get the addr, port of the peer
    end_point get_peer_name(socket_t sock);

    //gets the addr, port of the local binding.
    end_point get_sock_name(socket_t sock);

    //configure socket
    void set_sock_opt(socket_t sock, int level, int optname, void* optval, int optlen);

#ifndef WIN32
    using iocmdtype_t = uint32_t;
#else
    using iocmdtype_t = long;
#endif
    //a bit convoluted but you need to add specializations for all the ts you want to call this with, to this header. Then compilation units will not emit linker errors when using this function.
    template <class... Ts>
    void ioctl(socket_t sock, iocmdtype_t op, Ts const&... ts);

    //listen for up to num_clients incoming connections
    void listen(socket_t sock, size_t num_clients);

    //read count_bytes bytes of data into buffer. Please prefer recv, this is included for compatibility.
    void read(socket_t sock, void* buffer, size_t count_bytes);

    //recv buffer bytes from socket
    void recv(socket_t sock, void* buffer, size_t buffer_size);

    //recv data from a specific endpoint (UDP only)
    void recv_from(socket_t sock, end_point const& source, void* buffer);

    /**
     * Restricts you to using WSA provider sockets only on Windows. Is fine on Linux. Maybe just don't use this unless you really want to get message header info or do some obscure scatter IO optimization.
     * This cannot be used for QoS guarantees on Windows because they just up and outright disabled IP_TOS in favour of their GQOS API. You will need to write platform specific windowos code to support QOS. 
     * TODO: implement this
     **/
    std::optional<end_point> recv_msg(socket_t sock);
}