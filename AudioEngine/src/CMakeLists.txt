
if (WIN32)
    target_sources(AudioEngine PRIVATE sockapi_windows.cpp)

    target_link_libraries(AudioEngine PRIVATE wsock32 ws2_32)
endif()