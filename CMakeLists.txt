cmake_minimum_required(VERSION 3.20)
project(FoxBrawlESP CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(FoxBrawlESP WIN32
    src/main.cpp
    src/brawl_esp.cpp
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_win32.cpp
    third_party/imgui/backends/imgui_impl_dx11.cpp
)

target_include_directories(FoxBrawlESP PRIVATE
    src
    third_party/imgui
    third_party/imgui/backends
)

target_link_libraries(FoxBrawlESP PRIVATE d3d11 dwmapi psapi)

target_compile_definitions(FoxBrawlESP PRIVATE
    UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN
)
