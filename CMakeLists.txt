cmake_minimum_required(VERSION 3.26)
project(bgfxSdl2ExampleWindowsDx11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("/std:c++17")
add_compile_options("/Zc:__cplusplus")

include_directories(.)
include_directories(engine)

add_definitions(-DBX_CONFIG_DEBUG=1)
add_definitions(-DWIN32)
add_definitions(-D_WINDOWS)
add_definitions(-DNOMINMAX)

set(SRCROOT ${CMAKE_SOURCE_DIR})

include_directories(
        ${SRCROOT}/include
        ${SRCROOT}/include/SDL2
        ${SRCROOT}/include/compat/mscv
)

add_executable(bgfxSdl2ExampleWindowsDx11 main.cpp SDL_uikit_main.c
        main.h)

target_link_options(bgfxSdl2ExampleWindowsDx11 PRIVATE "/NODEFAULTLIB:MSVCRT")
set(bgfxSdl2ExampleWindowsDx11_DLLS
        "${SRCROOT}\\libs\\windows\\debug\\SDL2.dll"
        "${SRCROOT}\\libs\\windows\\debug\\SDL2_image.dll"
)
target_link_libraries(bgfxSdl2ExampleWindowsDx11 PRIVATE
        "${SRCROOT}\\libs\\windows\\debug\\SDL2.lib"
        "${SRCROOT}\\libs\\windows\\debug\\SDL2main.lib"
        "${SRCROOT}\\libs\\windows\\debug\\SDL2_image.lib"
        "${SRCROOT}\\libs\\windows\\debug\\single-threaded\\bgfxDebug.lib"
        "${SRCROOT}\\libs\\windows\\debug\\single-threaded\\bimgDebug.lib"
        "${SRCROOT}\\libs\\windows\\debug\\single-threaded\\bxDebug.lib"
        "vcruntimed.lib"
        "msvcprtd.lib"
        "msvcrtd.lib"
)

foreach (bgfxSdl2ExampleWindowsDx11_DLL_PATH ${bgfxSdl2ExampleWindowsDx11_DLLS})
    add_custom_command(TARGET bgfxSdl2ExampleWindowsDx11 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${bgfxSdl2ExampleWindowsDx11_DLL_PATH}
            $<TARGET_FILE_DIR:bgfxSdl2ExampleWindowsDx11>)
endforeach ()


set(bgfxSdl2ExampleWindowsDx11_ASSETS "${SRCROOT}\\assets")

add_custom_command(TARGET bgfxSdl2ExampleWindowsDx11 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${bgfxSdl2ExampleWindowsDx11_ASSETS}
        $<TARGET_FILE_DIR:bgfxSdl2ExampleWindowsDx11>\\assets)

target_include_directories(bgfxSdl2ExampleWindowsDx11 PUBLIC "${SRCROOT}/include" "${SRCROOT}/include/compat/msvc")