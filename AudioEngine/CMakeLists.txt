project(AUDIOENGINE)

add_library(AudioEngine STATIC)
set(AudioEngine_DIR "$CMAKE_CURRENT_SOURCE_DIR" PARENT_SCOPE)

set_property(TARGET AudioEngine PROPERTY CXX_STANDARD 20) #C++ 20

target_include_directories(AudioEngine PUBLIC include)

add_subdirectory(src)
