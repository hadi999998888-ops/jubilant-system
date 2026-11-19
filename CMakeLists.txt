cmake_minimum_required(VERSION 3.20)
project(FoxBrawlESP CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(FoxBrawlESP WIN32
    src/main.cpp
    src/brawl_esp.cpp
    src/imgui.cpp
    src/imgui_draw.cpp
    src/imgui_tables.cpp
    src/imgui_widgets.cpp
    src/imgui_impl_win32.cpp
    src/imgui_impl_dx11.cpp
)

target_include_directories(FoxBrawlESP PRIVATE src)
target_link_libraries(FoxBrawlESP PRIVATE d3d11 dwmapi psapi)
target_compile_definitions(FoxBrawlESP PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
