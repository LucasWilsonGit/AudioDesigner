#pragma once

#include <cstdint>
#include <variant>
#include <concepts>
#include <array>
#include <type_traits>
#include <string>
#include <utility>
#include <optional>
#include <vector>
#include <bitset>


#ifdef _WIN32
    using epoll_handle_t = void*;
#else
    using epooll_handle_t = int;
#endif


#include "core.hpp"

using sa_family_t = uint16_t;

#ifdef _MSC_VER
    typedef struct in_addr in_addr;
    typedef struct in6_addr in6_addr;
    typedef struct sockaddr sockaddr;
    typedef struct sockaddr_in sockaddr_in;
    typedef struct sockaddr_in6 sockaddr_in6;
    typedef struct sockaddr_storage sockaddr_storage;
#else
    struct in_addr;
    struct in6_addr;
    struct sockaddr;
    struct sockaddr_in;
    struct sockaddr_in6;
    //16byte aligned 128 byte buffer based on RFC 4393 assuming max 128byte sockaddr type
    struct alignas(16) sockaddr_storage;
#endif

namespace Net {
    //fdecl socket.hpp class socket
    struct socket;
    struct address_ipv4;
    struct address_ipv6;
    struct end_point;

#ifdef _WIN32 
    using epoll_handle_t = void*;

    constexpr uint64_t POLLPRI = 0x400;
    
    constexpr uint64_t POLLRDNORM = 0x100;
    constexpr uint64_t POLLRDBAND = 0x200;
    constexpr uint64_t POLLIN = POLLRDNORM | POLLRDBAND;

    constexpr uint64_t POLLWRNORM = 0x10;
    constexpr uint64_t POLLWRBAND = 0x20;
    constexpr uint64_t POLLOUT = POLLWRNORM | POLLWRBAND;

    constexpr uint64_t POLLERR = 0x1;
    constexpr uint64_t POLLHUP = 0x2;
    constexpr uint64_t POLLNVAL = 0x4;

    constexpr uint64_t POLLMSG = POLLIN;

    using message_header_t = void;

#else
    using epoll_handle_t = int;

    constexpr uint64_t POLLIN = 0x1;
    constexpr uint64_t POLLPRI = 0x2;
    constexpr uint64_t POLLOUT = 0x4;
    constexpr uint64_t POLLERR = 0x8;
    constexpr uint64_t POLLHUP = 0x10;
    constexpr uint64_t POLLNVAL = 0x20;

    constexpr uint64_t POLLRDNORM = 0x40;
    constexpr uint64_t POLLRDBAND = 0x80;
    constexpr uint64_t POLLWRNORM = 0x100;
    constexpr uint64_t POLLWRBAND = 0x200;

    constexpr uint64_t POLLMSG = 0x400;
    constexpr uint64_t POLLREMOVE = 0x1000;
    constexpr uint64_t POLLRDHUP = 0x2000;

    using message_header = void;

#endif

    struct pollfd {
        Net::socket_t sock;
        short events;
        short revents;
    };

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

    constexpr std::array<std::byte, 4> to_bytes(in_addr const& value);
    constexpr std::array<std::byte, 16> to_bytes(in6_addr const& value);

    //inits WSA on windows, should do nothing in linux.
    void init();

    //cleans up WSA on Windows
    void cleanup();

    //uses inet_pton
    std::array<std::byte, 4> parse_ipv4(std::string const& addr);
    //uses to_bytes<T>(T);
    std::array<std::byte, 4> parse_ipv4(in_addr const& addr);

    //uses inet_pton
    std::array<std::byte, 16> parse_ipv6(std::string const& addr);
    //uses some possibly unsafe pointer voodoo. Alignment and lifetime rules should not be violated though so I think it ought to be standards compliant.
    std::array<std::byte, 16> parse_ipv6(in6_addr const& addr);

    in_addr get_addr4(address_ipv4 const& addr);
    in6_addr get_addr6(address_ipv6 const& addr);

    //get an end_point from a sockaddr
    end_point parse_end_point(::sockaddr const& addr);                                  //Tested

    //get a socketaddr from an end point
    std::pair<::sockaddr_storage, size_t> get_end_point(end_point const& ep);           //Tested

    //create an unbound socket
    socket_t socket(int family, int type, int protocol);                                //Tested

    //binds the passed socket to the passed endpoint
    void bind(socket_t sock, end_point const& endpoint);                                //Tested

    //listen for up to num_clients incoming connections
    void listen(socket_t sock, int num_clients);                                        //Tested

    //accept the first incoming connection
    std::pair<socket_t, end_point> accept(socket_t sock);                               //Tested, depends connect

    //closes the passed socket
    void close(socket_t sock);                                                          //Tested   

    //opens a connection to the peer
    void connect(socket_t sock, end_point const& endpoint);                             //Tested by case accept

    //get the addr, port of the peer
    end_point get_peer_name(socket_t sock);                                             //Tested, depends bind & connect

    //gets the addr, port of the local binding.
    end_point get_sock_name(socket_t sock);                                             //Tested, depends bind

    //configure socket
    void set_sock_opt(socket_t sock, int level, int optname, char* optval, int optlen);

    template <size_t N>
    void set_sock_opt(socket_t sock, int level, int optname, std::array<std::byte, N> const& opt) {
        set_sock_opt(sock, level, optname, reinterpret_cast<char*>(opt.data()), static_cast<int>(N));
    }

    template <class T>
    void set_sock_opt(socket_t sock, int level, int optname, T const& opt) {
        set_sock_opt(sock, level, optname, reinterpret_cast<char*>(&opt), static_cast<int>(sizeof(opt)));
    }

    //check socket configuration
    void get_sock_opt(socket_t sock, int level, int optname, char* optval, int *optlen);

    template <class T>
    void get_sock_opt(socket_t sock, int level, int optname, T& out_opt) {
        int len = sizeof(T);
        get_sock_opt(sock, level, optname, reinterpret_cast<char*>(&out_opt), &len);
    }

#ifndef WIN32
    using iocmdtype_t = uint32_t;
#else
    using iocmdtype_t = long;
#endif
    //a bit convoluted but you need to add specializations for all the ts you want to call this with, to this header. Then compilation units will not emit linker errors when using this function.
    template <class... Ts>
    void ioctl(socket_t sock, iocmdtype_t op, Ts const&... ts);

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

    //select is not supported because of it's hardcoded FD_SET limit and security vulnerabilities

    //returns the number of populated pollfds, they will be sparse in the entries vector, it is modified in place so iterate it and bit test the revents
    int poll(std::vector<pollfd>& entries, int timeout);

    epoll_handle_t epoll_create(int size);
    epoll_handle_t epoll_create1(int flags);
    int epoll_ctl(epoll_handle_t ep, int op, socket_t sock, struct epoll_event* event);
    int epoll_wait(epoll_handle_t ep, struct epoll_event *events, int maxevents, int timeout);
    int epoll_close(epoll_handle_t ep);

    int send(socket_t sock, uint8_t const* buf, int size, int flags);

    template <size_t N>
    int send(socket_t sock, std::array<std::byte, N> data, int flags) {
        return send(sock, data.data(), N, flags);
    }

    int sendto(socket_t sock, uint8_t const* buf, int size, int flags, end_point const& dst);

    template <size_t N>
    int sendto(socket_t sock, std::array<std::byte, N> data, int flags, end_point const& dst) {
        return sendto(sock, data.data(), data.size(), flags, dst); //calls out to non-templated definition
    }
    
    int sendmsg(socket_t sock, message_header_t *header, int flags);

    void shutdown(socket_t sock, int how);

    int write(socket_t sock, void * buf, size_t len);

    template <size_t N>
    int write(socket_t sock, std::array<std::byte, N> data) {
        return write(sock, data.data(), N);
    }

    template <class T>
    int write(socket_t sock, T const& data) {
        return write(sock, &data, sizeof(T));
    }
}