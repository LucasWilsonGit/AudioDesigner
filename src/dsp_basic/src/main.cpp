#include "AudioEngine/sockapi.hpp"
#include "AudioEngine/address.hpp"

#include <winsock2.h>

#include <iostream>

#include <windows.h>

#include <SDKDDKVer.h>
#include <winternl.h>
#pragma comment(lib, "ntdll.lib")

#define AFD_POLL_RECEIVE           0x0001
#define AFD_POLL_RECEIVE_EXPEDITED 0x0002
#define AFD_POLL_SEND              0x0004
#define AFD_POLL_DISCONNECT        0x0008
#define AFD_POLL_ABORT             0x0010
#define AFD_POLL_LOCAL_CLOSE       0x0020
#define AFD_POLL_ACCEPT            0x0080
#define AFD_POLL_CONNECT_FAIL      0x0100

#define IOCTL_AFD_POLL 0x00012024


#include <limits>

typedef struct _AFD_POLL_HANDLE_INFO {
  HANDLE Handle;
  ULONG Events;
  NTSTATUS Status;
} AFD_POLL_HANDLE_INFO, *PAFD_POLL_HANDLE_INFO;

typedef struct _AFD_POLL_INFO {
  LARGE_INTEGER Timeout;
  ULONG NumberOfHandles;
  ULONG Exclusive;
  AFD_POLL_HANDLE_INFO Handles[1];
} AFD_POLL_INFO, *PAFD_POLL_INFO;


struct CompletionHandler
{
    OVERLAPPED ovl;

    uint64_t marker = 0xFEEDF00D;

};

void DisplayError(DWORD NTStatusMessage)
{
   LPVOID lpMessageBuffer;
   HMODULE Hand = LoadLibrary("NTDLL.DLL");

   FormatMessage( 
       FORMAT_MESSAGE_ALLOCATE_BUFFER | 
       FORMAT_MESSAGE_FROM_SYSTEM | 
       FORMAT_MESSAGE_FROM_HMODULE,
       Hand, 
       NTStatusMessage,  
       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
       (LPTSTR) &lpMessageBuffer,  
       0,  
       NULL );

   // Now display the string.
   std::cout << lpMessageBuffer << "\n";

   // Free the buffer allocated by the system.
   LocalFree( lpMessageBuffer ); 
   FreeLibrary(Hand);
}

int main() {
    Net::socket_t sock = 0;

    //If this fails then we might as well just exit.
    Net::init();

    try {
        Net::address_ipv4 addr{"127.0.0.1"};
        Net::end_point ep(addr, 10681);

        Net::socket_t sock = Net::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        u_long mode = 1;
        ::ioctlsocket(sock, FIONBIO, &mode);
        //Net::ioctl(sock, FIONBIO, &mode);

        Net::bind(sock, ep);
        Net::end_point ep2 = Net::get_sock_name(sock);
        std::cout << "My sock name is " << ep2 << "\n";

        Net::listen(sock, 1); 
        Sleep(INFINITE);

        std::vector<Net::pollfd> fds;

        Net::pollfd entry;
        entry.sock = sock;
        entry.events = POLLIN;

        fds.push_back(entry);

        int res = Net::poll(fds, -1);
        std::cout << "Net::poll returned " << res << "\n";

        if (fds[0].revents & POLLRDNORM) {
            std::cout << "Ok we got incoming data\n";

            auto [client_sock, client_ep] = Net::accept(sock);

            std::cout << "Accepted incoming connection from " << client_ep << "\n";
        }
    }
    catch (Net::net_error const& e) {
        std::cout << e.what() << "\n";
        if (sock)
            Net::close(sock);
    }

    Net::cleanup();

    return 0;
}