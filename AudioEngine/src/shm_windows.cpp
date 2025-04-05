#pragma warning(push, 0)

#include <iostream>
#include "AudioEngine/shm.hpp"
#include <windows.h>
#include <winnt.h>
#include <memoryapi.h>
#include <processthreadsapi.h>

#pragma warning(pop, 0)

namespace Memory {
    class temporary_handle {
    private:
        HANDLE m_handle;
    
    public:
        temporary_handle(HANDLE hnd) : m_handle(hnd) {}
        ~temporary_handle() { ::CloseHandle(m_handle); }

        HANDLE const& peek_handle() const noexcept { return m_handle; }
    };

    std::string memory_platform_error::get_error_msg() {
        auto err = GetLastError();
        if (err == 0)
            return "";

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

    size_t init_page_size() {
        SIZE_T size = GetLargePageMinimum();
        if (size == 0)
            throw memory_platform_error("System does not support Large Pages; GetLargePageMinimum returned 0");
        
        return size;
    }
    size_t get_page_size() {
        static size_t size=init_page_size(); //check this only once by making it static
        return size;
    }

    temporary_handle get_process_token() {
        HANDLE hnd{INVALID_HANDLE_VALUE};
        if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hnd))
            throw memory_error("Failed to open process token for current process.");
        
        return temporary_handle(hnd);
    }

    bool check_privilege_enabled(temporary_handle const& tok_handle, LPCSTR privilege) {
        LUID luid;
        if (!LookupPrivilegeValue(nullptr, privilege, &luid)) {
            throw memory_platform_error("Lookup privilege value failed");
        }

        PRIVILEGE_SET ps;
        ps.PrivilegeCount = 1;
        ps.Control = PRIVILEGE_SET_ALL_NECESSARY;
        ps.Privilege[0].Luid = luid;
        ps.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;

        BOOL result = false;
        if (!PrivilegeCheck(tok_handle.peek_handle(), &ps, &result)) {
            return false;
        }

        return result;
    }

    bool enable_privilege(temporary_handle const& tok_handle, LPCSTR privilege) {
        TOKEN_PRIVILEGES Privs = {};
        Privs.PrivilegeCount = 1;
        Privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        if(LookupPrivilegeValue(0, privilege, &Privs.Privileges[0].Luid))
        {
            if (::AdjustTokenPrivileges(tok_handle.peek_handle(), FALSE, &Privs, 0, 0, 0))
                return check_privilege_enabled(tok_handle, privilege);
            else
                throw memory_platform_error("Failed AdjustTokenPrivileges in enable_privileges: " + std::string(privilege));
        }
        else
            throw memory_platform_error("LookupPrivilegeValue failed: " + std::string(privilege));
        
        //return false;
    }

    //returns bool to allow for static assignment in init method
    bool enable_large_pages()
    {   
        temporary_handle tok_handle{get_process_token()};
        std::cout << format("Enable large pages1 {} \n", tok_handle.peek_handle());
        
        if (!enable_privilege(tok_handle, SE_LOCK_MEMORY_NAME))
            throw memory_platform_error("Failed to elevate SE_LOCK_MEMORY_NAME\n");
        if (!enable_privilege(tok_handle, SE_CREATE_GLOBAL_NAME))
            throw memory_platform_error("Failed to elevate SE_CREATE_GLOBAL_NAME\n");

        std::cout << format("Enable large pages: Privileges granted {} \n", tok_handle.peek_handle());

        return true;
    }

    //@todo: Figure out if this needs to be Global
    std::string make_platform_name(std::string const& name) {
        return "Session\\" + name;
    }

    mapping make_mapping(std::string const& name, size_t size, uint32_t access_flag) {
        if ( (size & (get_page_size() - 1)) != 0)
            throw memory_error("Requested mapping that is not a multiple of the minimum large page size");

        LARGE_INTEGER winsize;
        winsize.QuadPart = static_cast<LONGLONG>(size);

        HANDLE handle = CreateFileMappingA(
            INVALID_HANDLE_VALUE,
            NULL,
            access_flag | SEC_LARGE_PAGES | SEC_COMMIT,
            static_cast<DWORD>(winsize.HighPart),
            winsize.LowPart,
            name.data()
        );
        if (handle == NULL)
            throw memory_platform_error("Failed to create file mapping object");

        void* buffer = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS | FILE_MAP_LARGE_PAGES, 0, 0, size);
        if (buffer == nullptr)
            throw memory_platform_error("Failed to map view of file");
        
        return mapping(name, handle, size, buffer);
    }




    template <size_t PageSize>
    bool win32mmapapi<PageSize>::init() {
        static bool large_pages = enable_large_pages();
        return large_pages;
    }
    template bool win32mmapapi<pagesize_2MB>::init();
    template bool win32mmapapi<pagesize_4MB>::init();

    template <size_t PageSize>
    mapping win32mmapapi<PageSize>::create(std::string const& name, size_t size, uint32_t flags) {
        static bool initialized = init();
        return make_mapping(name, size, flags);
    }
    template mapping win32mmapapi<pagesize_2MB>::create(std::string const&, size_t, uint32_t);
    template mapping win32mmapapi<pagesize_4MB>::create(std::string const&, size_t, uint32_t);

    template <size_t PageSize>
    void win32mmapapi<PageSize>::release(mapping const& mapping) {
        if(!UnmapViewOfFile(mapping.data))
            throw memory_error(format("Failed to map view of file {}", mapping.name));
        CloseHandle(mapping.handle);
    }
    template void win32mmapapi<pagesize_2MB>::release(mapping const&);
    template void win32mmapapi<pagesize_4MB>::release(mapping const&);

    template <size_t PageSize>
    void* win32mmapapi<PageSize>::data(mapping const& mapping) noexcept {
        return mapping.data;
    }
    template void* win32mmapapi<pagesize_2MB>::data(mapping const&) noexcept;
    template void* win32mmapapi<pagesize_4MB>::data(mapping const&) noexcept;

}