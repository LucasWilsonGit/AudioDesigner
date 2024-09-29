#include "AudioEngine/sockapi.hpp"
#include "AudioEngine/address.hpp"

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>
#include <functional>
#include <cstring>

namespace wepoll {
#include "jumbo.h" //from wepoll PUBLIC includes
}

#include <iostream>

//being very explicit about namespaces just to avoid any future headaches
namespace Net {

    std::string errno_string(int err) {
        char* buf = nullptr;
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
            nullptr, 
            err, 
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
            (LPSTR)&buf, 
            0,
            nullptr
        );
        std::string str(buf);
        LocalFree(buf);
        return str; 
    }

    void WSAcall(int ret) {
        if (ret != 0) {
            throw net_error("WSA error: " + errno_string(WSAGetLastError()));
        }
    }

    void init() {
        int result;
        WSADATA wsa_data;

        WSAcall(WSAStartup(MAKEWORD(2,2), &wsa_data));
    }

    void cleanup() {
        WSAcall(WSACleanup());
    }

    std::array<std::byte, 4> parse_ipv4(std::string const& addr) {
        std::array<std::byte, 4> buffer alignas(::in_addr); //only the ipv4 address component
        if (::inet_pton(AF_INET, addr.c_str(), reinterpret_cast<void*>(&buffer)) != 1)
            throw net_error("Address " + addr + " is not a valid ipv4 address.");

        return buffer;
    }

    std::array<std::byte, 4> parse_ipv4(::in_addr const& value) {
        return std::array<std::byte, 4> {
            static_cast<std::byte>(value.S_un.S_un_b.s_b1), 
            static_cast<std::byte>(value.S_un.S_un_b.s_b2), 
            static_cast<std::byte>(value.S_un.S_un_b.s_b3), 
            static_cast<std::byte>(value.S_un.S_un_b.s_b4)
        };
    }

    std::array<std::byte, 16> parse_ipv6(std::string const& addr) {
        ::in6_addr buffer;
        if (::inet_pton(AF_INET6, addr.c_str(), &buffer) != 1)
            throw net_error("Address " + addr + " is not a valid ipv6 address.");

        return parse_ipv6(buffer);
    }

    std::array<std::byte, 16> parse_ipv6(::in6_addr const& value) {
        std::array<std::byte, 16> buffer;

        //This could be wwrong due to endianness but I am just moving across byte arrays using a larger type, so it should be fine? Unit test will confirm.
        void* buff = (uint64_t*)value.u.Byte;
        std::memcpy(buffer.data(), buff, 16);

        return buffer;
    }

    in_addr get_addr4(address_ipv4 const& addr) {
        in_addr sockaddr;
        memcpy(&sockaddr.S_un.S_addr, addr.data(), 4);
        return sockaddr;
    }

    in6_addr get_addr6(address_ipv6 const& addr) {
        in6_addr sockaddr;
        memcpy(&sockaddr.u.Byte, addr.data(), 16);
        return sockaddr;
    }

    std::string address_ipv4::display_string() const {
        std::array<char, 16> buffer;
        in_addr addr = get_addr4(*this);
        
        if (::inet_ntop(AF_INET, &addr, buffer.data(), buffer.size()) == NULL)
            WSAcall(-1); //force error

        return buffer.data();
    }

    std::string address_ipv6::display_string() const {
        std::array<char, 46> buffer;
        in6_addr addr = get_addr6(*this);
        
        if (::inet_ntop(AF_INET6, &addr, buffer.data(), buffer.size()) == NULL)
            WSAcall(-1); //force error

        return buffer.data();
    }

    end_point parse_end_point(::sockaddr const& addr) {
        switch (addr.sa_family) {
            case AF_INET: {
                ::sockaddr_in const& addr_in = reinterpret_cast<::sockaddr_in const&>(addr);
                
                address_ipv4 address(addr_in.sin_addr.S_un.S_addr); //build ipv4 address from uint32_t S_addr
                port_t port = ::ntohs(addr_in.sin_port);  

                return end_point(address, port);
            break; }
            

            case AF_INET6: {
                //This feels like bad voodoo but references are non-slicing and the socket interface already assumes if AF_FAMILY is AF_INET6 that we can cast the pointed sockaddr* to a sockaddr_in6
                ::sockaddr_in6 const& addr_in = reinterpret_cast<::sockaddr_in6 const&>(addr);
                
                address_ipv6 address(addr_in.sin6_addr);
                port_t port = ::ntohs(addr_in.sin6_port);

                return end_point(address, port);
            break; }
            

            default: {
                throw net_error("Unsupported safamily: " + std::to_string(addr.sa_family));
                return end_point(address_ipv4(0), 0);
            break; }
        }
    }

    struct sockaddr_resolver{
        ::sockaddr_storage operator()(address_ipv4 const& addr, port_t port) {
            return this->resolve(addr.display_string(), std::to_string(port), AF_INET);
        }
        ::sockaddr_storage operator()(address_ipv6 const& addr, port_t port) {
            return this->resolve(addr.display_string(), std::to_string(port), AF_INET6);
        }

        ::sockaddr_storage resolve(std::string hostname, std::string port, int family) {
            ::sockaddr_storage ss;

            ::addrinfo options {
                .ai_flags = 0,
                .ai_family = AF_UNSPEC,
                .ai_socktype = 0,
                .ai_protocol = 0,
                .ai_addrlen = 0,
                .ai_canonname = NULL,
                .ai_addr = NULL,
                .ai_next = NULL
            };

            ::addrinfo* p_res = nullptr;
            WSAcall(::getaddrinfo(hostname.c_str(), port.c_str(), &options, &p_res));

            for ( ; p_res; p_res = p_res->ai_next ) {
                if (p_res->ai_family == family) {
                    ::std::memcpy(&ss, p_res->ai_addr, p_res->ai_addrlen);
                    goto cleanup;
                }
            }

            throw net_error("Address " + hostname + ":" + port + " could not be resolved.");

cleanup:
            ::freeaddrinfo(p_res);
            return ss;
        }

    };
    
    ::sockaddr_storage get_end_point(end_point const& ep) {
        sockaddr_resolver resolver{};
        auto callable = std::bind(resolver, std::placeholders::_1, ep.port);
        ::sockaddr_storage res = std::visit(callable, ep.address);
        return res;
    }

    ::sockaddr_storage end_point::get_sockaddr() const {
        return get_end_point(*this);
    }

    socket_t socket(int family, int type, int proto) {
        socket_t res = ::socket(family, type, proto);
        if (res == INVALID_SOCKET)
            WSAcall(-1); //kickstart the usual net error stuff
        return res; 
    }

    void bind(socket_t sock, end_point const& target) {
        int addrlen;
        ::sockaddr_storage ss = get_end_point(target);
        switch (ss.ss_family) {
            case AF_INET:
                addrlen = sizeof(sockaddr_in);
                break;
            case AF_INET6:
                addrlen = sizeof(sockaddr_in6);
                break;
            default:
                throw net_error("Unsupported AF_FAMILY");
        }
        WSAcall(::bind(sock, (SOCKADDR*)&ss, addrlen));
    }

    void listen(socket_t sock, int num_clients) {
        WSAcall(::listen(sock, num_clients));
    }

    std::pair<socket_t, end_point> accept(socket_t sock) {
        ::sockaddr out_addr;
        int out_len = sizeof(out_addr);
        socket_t client_sock = ::accept(sock, &out_addr, &out_len);
        if (client_sock == INVALID_SOCKET || client_sock == static_cast<Net::socket_t>(-1))
            WSAcall(SOCKET_ERROR);

        end_point peer = parse_end_point(out_addr);

        return {client_sock, peer};
    }

    void close(socket_t sock) {
        WSAcall(::closesocket(sock));
    }


    
    end_point get_peer_name(socket_t sock) {
        ::sockaddr_storage ss;
        int len = sizeof(ss);
        WSAcall(::getpeername(sock, (sockaddr*)&ss, &len));

        return parse_end_point(reinterpret_cast<sockaddr const&>(ss));
    }

    end_point get_sock_name(socket_t sock) {
        ::sockaddr_storage ss;
        int len = sizeof(ss);
        WSAcall(::getsockname(sock, (sockaddr*)&ss, &len));

        return parse_end_point(reinterpret_cast<sockaddr const&>(ss));
    }



    template <class... Ts>
    void ioctl(socket_t sock, iocmdtype_t cmd, Ts const&... args) {
        WSAcall(::ioctlsocket(sock, cmd, args...     ));
    }
    template void ioctl<u_long*>(socket_t, iocmdtype_t, u_long* const&);

    int poll(std::vector<pollfd>& entries, int timeout_ms) {
        static_assert(sizeof(pollfd) == sizeof(::pollfd), "internal pollfd definition does not match the size of the OS pollfd definition");

        int res = ::WSAPoll((WSAPOLLFD*)entries.data(), entries.size(), timeout_ms);
        if (res == -1) //-1 handling
            WSAcall(res);

        return res;
    }

    epoll_handle_t epoll_create(int size) {
        return wepoll::epoll_create(size);
    }

    epoll_handle_t epoll_create1(int flags) {
        return wepoll::epoll_create1(flags);
    }

    int epoll_ctl(epoll_handle_t ep, int op, socket_t sock, epoll_event* event) {
        return wepoll::epoll_ctl(ep, op, sock, (wepoll::epoll_event*)event);
    }

    int epoll_wait(epoll_handle_t ep, epoll_event *events, int maxevents, int timeout) {
        return wepoll::epoll_wait(ep, (wepoll::epoll_event*)events, maxevents, timeout);
    }

    int epoll_close(epoll_handle_t ep) {
        return wepoll::epoll_close(ep);
    }

}