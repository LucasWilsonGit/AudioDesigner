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

struct in_addr;
struct in6_addr;
struct sockaddr;
struct sockaddr_in;
struct sockaddr_in6;
//16byte aligned 128 byte buffer based on RFC 4393 assuming max 128byte sockaddr type
struct sockaddr_storage;

union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
};
typedef union epoll_data epoll_data_t;
struct epoll_event {
    uint32_t events;
    epoll_data_t data;
};

namespace Net {
    //fdecl socket.hpp class socket
    class socket;
    class address_ipv4;
    class address_ipv6;
    class end_point;

#ifdef _WIN32 
    using epoll_handle_t = void*;

    #define POLLPRI = 0x400;
    
    #define POLLRDNORM = 0x100;
    #define POLLRDBAND = 0x200;
    #define POLLIN = POLLRDNORM | POLLRDBAND;

    #define POLLWRNORM = 0x10;
    #define POLLWRBAND = 0x20;
    #define POLLOUT = POLLWRNORM | POLLWRBAND;

    #define POLLERR = 0x1;
    #define POLLHUP = 0x2;
    #define POLLNVAL = 0x4;

    #define POLLMSG = POLLIN;
    
#else
    using epoll_handle_t = int;

    #define POLLIN = 0x1;
    #define POLLPRI = 0x2;
    #define POLLOUT = 0x4;
    #define POLLERR = 0x8;
    #define POLLHUP = 0x10;
    #define POLLNVAL = 0x20;

    #define POLLRDNORM = 0x40;
    #define POLLRDBAND = 0x80;
    #define POLLWRNORM = 0x100;
    #define POLLWRBAND = 0x200;

    #define POLLMSG = 0x400;
    #define POLLREMOVE = 0x1000;
    #define POLLRDHUP = 0x2000;

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

    constexpr std::array<std::byte, 4> to_bytes(::in_addr const& value);
    constexpr std::array<std::byte, 16> to_bytes(::in6_addr const& value);

    //inits WSA on windows, should do nothing in linux.
    void init();

    //cleans up WSA on Windows
    void cleanup();

    //uses inet_pton
    std::array<std::byte, 4> parse_ipv4(std::string const& addr);
    //uses to_bytes<T>(T);
    std::array<std::byte, 4> parse_ipv4(::in_addr const& addr);

    //uses inet_pton
    std::array<std::byte, 16> parse_ipv6(std::string const& addr);
    //uses some possibly unsafe pointer voodoo. Alignment and lifetime rules should not be violated though so I think it ought to be standards compliant.
    std::array<std::byte, 16> parse_ipv6(::in6_addr const& addr);

    ::in_addr get_addr4(address_ipv4 const& addr);
    ::in6_addr get_addr6(address_ipv6 const& addr);

    //get an end_point from a sockaddr
    end_point parse_end_point(::sockaddr const& addr);

    //ugly interface due to out reference, but it's for internal use except for people who want to do something fancy
    ::sockaddr_storage get_end_point(end_point const& ep);

    //create an unbound socket
    socket_t socket(int family, int type, int protocol);

    //binds the passed socket to the passed endpoint
    void bind(socket_t sock, end_point const& endpoint);

    //listen for up to num_clients incoming connections
    void listen(socket_t sock, int num_clients);

    //blocks until sock receives a connection request, then accepts the connection and returns the connecting socket
    std::pair<socket_t, end_point> accept(socket_t sock);

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

    void send();
    void sendto();
    void sendmsg();
    void shutdown();
    void write();
}