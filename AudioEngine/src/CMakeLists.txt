if (ISWINDOWS)
    target_sources(AudioEngine PRIVATE sockapi_windows.cpp shm_windows.cpp)

    target_link_libraries(AudioEngine PRIVATE ws2_32)
endif()