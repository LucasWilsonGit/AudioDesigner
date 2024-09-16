#include "AudioEngine/sockapi.hpp"
#include "AudioEngine/socket.hpp"

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>

//being very explicit about namespaces just to avoid any future headaches
namespace AudioEngine {

    

    std::string errno_string(int err) {
        LPSTR buf;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, 0, buf, 0, nullptr);
        std::string str(buf);
        LocalFree(buf);
        return str; 
    }

    void WSAcall(int ret) {
        if (ret != 0) {
            throw net_error("Failed to initialize WSA: " + errno_string(WSAGetLastError()));
        }
    }

    void init() {
        int result;
        WSADATA wsa_data;

        WSAcall(WSAStartup(MAKEWORD(2,2), &wsa_data));
    }

    std::array<std::byte, 4> parse_ipv4(std::string const& addr) {
        std::array<std::byte, 4> buffer alignas(::in_addr); //literally only the ipv4 address component
        if (::inet_pton(AF_INET, addr.c_str(), reinterpret_cast<void*>(&buffer)) != 1)
            throw net_error("Address " + addr + " is not a valid ipv4 address.");

        return buffer;
    }

    std::array<std::byte, 4> parse_ipv4(::in_addr const& value) {
        return to_bytes(value.S_un.S_addr);
    }

    std::array<std::byte, 16> parse_ipv6(std::string const& addr) {
        std::array<std::byte, 16> buffer alignas(::in6_addr);
        if (::inet_pton(AF_INET6, addr.c_str(), reinterpret_cast<void*>(&buffer)) != 1)
            throw net_error("Address " + addr + " is not a valid ipv6 address.");

        return buffer;
    }

    std::array<std::byte, 16> parse_ipv6(::in6_addr const& value) {
        std::array<std::byte, 16> buffer;
        
        //not 16 byte aligned in value so no SIMD intrinsic 
        uint64_t* qwords = (uint64_t*)value.u.Byte;
        (uint64_t&)(buffer.data()[0]) = qwords[0];
        (uint64_t&)(buffer.data()[8]) = qwords[1];

        return buffer;
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

    AudioEngine::socket accept(AudioEngine::socket const& sock) {
        ::sockaddr out_addr;
        int out_len;
        socket_t client_sock = ::accept(sock.platform_handle(), &out_addr, &out_len);

        end_point peer = parse_end_point(out_addr);
    }
}