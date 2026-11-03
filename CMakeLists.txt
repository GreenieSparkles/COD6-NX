# cod6 - ReXGlue recompiled project, Nintendo Switch target (stage 5 scaffold).
#
# The PC build (D:\rexglue-projects\cod6) keeps the emulated GPU; this one is the
# cross build to aarch64 + libnx. The generated C++ is the same in both.
#
# Milestone A: no GPU plugin is linked, so the NRO boots through the Horizon
# layer (libnx startup, guest memory, threads, audio, input) with no rendering.
# The graphics work is stage 4 (a native renderer) plus the real NVK driver.
cmake_minimum_required(VERSION 3.25)
project(cod6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDK from the source tree (nfsmw-nx's Switch-capable fork), not find_package.
if(NOT DEFINED REXSDK_DIR)
    set(REXSDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdk" CACHE PATH "ReXGlue SDK source tree")
endif()

include(generated/rexglue.cmake)

set(COD6_SOURCES
    src/main.cpp
    src/switch_stubs.cpp
    src/switch_stubs_pc.cpp  # COD6_PC_PORT
    src/cod6_renderizador.cpp
    src/cod6_nativo.cpp
    src/native/cod6_native_system.cpp
    src/native/cod6_native_renderer.cpp
    src/native/cod6_native_reader.cpp
    src/native/cod6_native_selftest.cpp
    src/native/cod6_native_destinos.cpp
    src/native/nativo/native_renderer.cpp
    src/native/nativo/native_textures.cpp
    src/native/nativo/texture_pool.cpp
    src/native/nativo/native_geometry_shaders.cpp
    src/native/nativo/native_nvk_draw.cpp
    src/native/nativo/cod6_dvar_hook.cpp
    src/native/nativo/cod6_dvars.cpp
    src/native/nativo/cod6_diag_stubs.cpp
    src/native/nativo/cod6_port_stubs.cpp
    src/native/nativo/cod6_cpu_runtime.cpp
    src/native/nativo/cod6_file_writes.cpp
    src/native/nativo/cod6_page_writes.cpp
    src/native/nativo/cod6_native_system_nativo.cpp
    src/native/cod6_native_cp.cpp
)

# El renderizador propio incluye rex/graphics/vulkan/command_processor.h, que
# arrastra el traductor de shaders y con el la cabecera de glslang
# (SPIRV/SpvBuilder.h). Las librerias del SDK reciben ese include por su propio
# CMake; el ejecutable del juego no, y sin esto no compila. Ver COD6_GPU_INCLUDES.
set(COD6_GPU_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/sdk/thirdparty/glslang"
    "${CMAKE_CURRENT_SOURCE_DIR}/sdk/thirdparty/spirv-tools/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/sdk/thirdparty/spirv-headers/include"
)

if(REXGLUE_PLATFORM_SWITCH)
    add_executable(cod6 ${COD6_SOURCES})
    target_include_directories(cod6 PRIVATE ${COD6_GPU_INCLUDES})
    # COD6_PC_PORT: the app's own Vulkan memory allocator header lives here.
    target_include_directories(cod6 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sdk/thirdparty/vulkan-memory-allocator/include")
    # COD6_PC_PORT: the app includes <vulkan/vulkan.h> itself; on Windows the plugin
    # target does not export that path to us.
    target_include_directories(cod6 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sdk/thirdparty/vulkan-headers/include")
    # The real Xenos GPU plugin, on top of the NVK driver built from
    # danfromtico/mesa-switch (libvulkan.a, 88 MB).
    rexglue_setup_target(cod6 GPU_PLUGINS xenos)
    if(WIN32)
        # COD6_PC_PORT: the netlog path uses Winsock directly.
        target_link_libraries(cod6 PRIVATE Ws2_32)
    endif()
    
    target_link_options(cod6 PRIVATE -flto=2 -flto-partition=balanced)
    # The NACP version string is what hbmenu shows, so it has to say which build
    # is on the card. It used to be "0.1.0" for every build, which made the card
    # unidentifiable without pulling the 41 MB NRO back and hashing it (handover
    # chapter 11, rule 1). One source of truth: the COD6_VERSION define.
    set(_cod6_version_file "${CMAKE_CURRENT_SOURCE_DIR}/sdk/src/core/exception_handler_switch.cpp")
    file(STRINGS "${_cod6_version_file}" _cod6_version_line
         REGEX "#define[ \t]+COD6_VERSION[ \t]+\"")
    if(_cod6_version_line MATCHES "COD6_VERSION[ \t]+\"([^\"]+)\"")
      set(COD6_BUILD_LABEL "${CMAKE_MATCH_1}")
    else()
      set(COD6_BUILD_LABEL "0.1.0")
      message(WARNING "COD6_VERSION not found in ${_cod6_version_file}; NACP version stays 0.1.0")
    endif()
    # The field is 16 bytes; keep room for the terminator.
    string(LENGTH "${COD6_BUILD_LABEL}" _cod6_label_len)
    if(_cod6_label_len GREATER 15)
      string(SUBSTRING "${COD6_BUILD_LABEL}" 0 15 COD6_BUILD_LABEL)
    endif()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_cod6_version_file}")
    message(STATUS "COD6 NACP version: ${COD6_BUILD_LABEL}")
    rexglue_switch_add_nro(cod6
        NAME    "cod6"
        AUTHOR  "ReXGlue port"
        VERSION "${COD6_BUILD_LABEL}")
else()
    if(WIN32)
        add_executable(cod6 WIN32 ${COD6_SOURCES})
    else()
        add_executable(cod6 ${COD6_SOURCES})
    endif()
    # COD6_PC_PORT: en PC hace falta lo mismo que en Switch para compilar la app:
    # glslang/SPIRV (el command processor del SDK) e imgui (rex_app.h -> style.h).
    target_include_directories(cod6 PRIVATE ${COD6_GPU_INCLUDES})
    # COD6_PC_PORT: the app's own Vulkan memory allocator header lives here.
    target_include_directories(cod6 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sdk/thirdparty/vulkan-memory-allocator/include")
    # COD6_PC_PORT: the app includes <vulkan/vulkan.h> itself; on Windows the plugin
    # target does not export that path to us.
    target_include_directories(cod6 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sdk/thirdparty/vulkan-headers/include")
    rexglue_setup_target(cod6 GPU_PLUGINS xenos)
    if(WIN32)
        # COD6_PC_PORT: the netlog path uses Winsock directly.
        target_link_libraries(cod6 PRIVATE Ws2_32)
    endif()
    if(TARGET imgui)
        target_include_directories(cod6 PRIVATE
            $<TARGET_PROPERTY:imgui,INTERFACE_INCLUDE_DIRECTORIES>)
    endif()
    # COD6_PC_PORT: mismo esquema que en Switch, el plugin va dentro del ejecutable.
    if(REXGLUE_GPU_PLUGIN_STATIC AND TARGET rexgpu-xenos)
        target_link_libraries(cod6 PRIVATE rexgpu-xenos)
    endif()
    # COD6_TP_INCLUDES: todas las cabeceras de thirdparty que la SDK enlaza en
    # privado (xxhash, fmt, spdlog, toml++, simde, spirv, dxc...), para que la
    # app compile igual que en Switch.
    foreach(_cod6_tp
            imgui renderdoc xxhash xxHash tomlplusplus spirv-tools-headers
            dxc-headers simde disruptorplus tc -ltracy)
        if(TARGET ${_cod6_tp})
            target_include_directories(cod6 PRIVATE
                $<TARGET_PROPERTY:${_cod6_tp},INTERFACE_INCLUDE_DIRECTORIES>)
        endif()
    endforeach()
endif()
