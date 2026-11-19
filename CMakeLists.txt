cmake_minimum_required(VERSION 3.20)
project(FoxBrawlESP CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Все исходники в корне — без папок
set(SOURCES
    main.cpp
    brawl_esp.cpp
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_impl_win32.cpp
    imgui_impl_dx11.cpp
)

add_executable(FoxBrawlESP WIN32 ${SOURCES})

# Инклюды — тоже всё в корне
target_include_directories(FoxBrawlESP PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(FoxBrawlESP PRIVATE d3d11 dwmapi psapi)

target_compile_definitions(FoxBrawlESP PRIVATE
    UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN
)
