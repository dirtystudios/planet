cmake_minimum_required(VERSION 4.3)

option(BUILD_ASAN "Build with address sanitizer" OFF)

message("CMake Version: ${CMAKE_VERSION}")

set(PL_DIR_ROOT     ${CMAKE_CURRENT_LIST_DIR})
set(PL_DIR_SOURCES  "${PL_DIR_ROOT}/src")
set(PL_DIR_ASSETS   "${PL_DIR_ROOT}/assets")
set(PL_DIR_EXTERNAL "${PL_DIR_ROOT}/external")

# CMake vcpkg toolchain
if(NOT DEFINED ENV{VCPKG_ROOT})
    set(ENV{VCPKG_ROOT} ${PL_DIR_EXTERNAL}/vcpkg)
endif()
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

project(planet C CXX)

if (MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.15")
        add_compile_options("$<$<CONFIG:Debug>:/JMC>")
    endif()
    if (BUILD_ASAN)
        add_compile_options(/fsanitize=address)
    endif()

    add_compile_options(
        "$<$<COMPILE_LANGUAGE:CXX,C>:/bigobj>"
        # Error on deprecated calls, object redefinition
        "$<$<COMPILE_LANGUAGE:CXX,C>:/we4996;/we4099>"
        "$<$<COMPILE_LANGUAGE:CXX,C>:/utf-8>"
        # Use new 'conforming' windows preprocessor
        "$<$<COMPILE_LANGUAGE:CXX,C>:/Zc:preprocessor>"
        # Enable standard C++ stack unwinding and assume extern "C" funcs don't throw
        "$<$<COMPILE_LANGUAGE:CXX,C>:/EHsc>"
        "$<$<COMPILE_LANGUAGE:CXX,C>:/Zc:__cplusplus>"
    )

    string(REPLACE "/Ob1" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    string(REPLACE "/Ob2" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    add_compile_options("$<$<NOT:$<CONFIG:Debug>>:/Ob3;/Zc:inline>")

    add_link_options("$<$<CONFIG:Release>:/DEBUG;/OPT:REF;/OPT:ICF>")
    add_compile_options("/arch:AVX2")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR 
    CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (BUILD_ASAN)
        add_compile_options(-fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

# Platform
if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -DUNICODE -D_UNICODE -D _CRT_SECURE_NO_WARNINGS)
elseif (APPLE)
    add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

# Deps
find_package(SDL3 REQUIRED)
find_package(xxHash CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(unofficial-enet CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(unofficial-noise CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(simdutf CONFIG REQUIRED)
find_package(slang CONFIG REQUIRED)
find_package(Stb REQUIRED)

# Sources
macro(PL_FILES_APPEND)
    file(GLOB FILES_APPEND CONFIGURE_DEPENDS ${ARGV1})
    list(APPEND ${ARGV0} ${FILES_APPEND})
endmacro()
macro(PL_FOLDER_APPEND)
    PL_FILES_APPEND(PL_HEADER_DIRS ${ARGV0})
    PL_FILES_APPEND(PL_HEADERS "${ARGV0}/*.h")
    PL_FILES_APPEND(PL_HEADERS "${ARGV0}/*.inc")
    PL_FILES_APPEND(PL_SOURCES "${ARGV0}/*.cpp")
    if (APPLE)
        PL_FILES_APPEND(PL_SOURCES "${ARGV0}/*.mm")
    endif()
endmacro()

set(PL_HEADERS)
set(PL_HEADER_DIRS)
set(PL_SOURCES)
PL_FOLDER_APPEND(${PL_DIR_SOURCES})
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/animation)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/components)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/events)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/ext)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/input)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/math)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/platform)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/platform/sdl)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/animation)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/backend)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/backend/interface)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/backend/interface/enums)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/backend/impl)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/backend/impl/common)
if (WIN32)
    PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/backend/impl/dx11)
    PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/backend/impl/gl)
elseif (APPLE)
    PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/backend/impl/gl)
    PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/backend/impl/metal)
endif()
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/material)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/mesh)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/renderers)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/renderers/debug)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/renderers/mesh)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/renderers/skybox)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/renderers/terrain)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/renderers/terrain/elevation)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/renderers/terrain/normals)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/renderers/text)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/render/renderers/ui)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/scheduler)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/simulation)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/ui)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/ui/frametypes)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/ui/frametypes/debug)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/utilities)
PL_FOLDER_APPEND(${PL_DIR_SOURCES}/utilities/common)
if (WIN32)
    PL_FOLDER_APPEND(${PL_DIR_SOURCES}/utilities/win32)
elseif (APPLE)
    PL_FOLDER_APPEND(${PL_DIR_SOURCES}/utilities/osx)
endif()

# Compile Options
add_executable(planet ${PL_SOURCES})
target_include_directories(planet 
                            PRIVATE 
                                ${PL_DIR_SOURCES}
                                ${PL_HEADER_DIRS}
                                "${PL_DIR_EXTERNAL}/include/enum-flags/include")
set_target_properties(planet 
                        PROPERTIES 
                            CXX_STANDARD 20
                            CXX_EXTENSIONS OFF
                            CXX_STANDARD_REQUIRED ON
                            POSITION_INDEPENDENT_CODE 1
                            CXX_VISIBILITY_PRESET hidden)

if(NOT DEFINED CMAKE_MSVC_DEBUG_INFORMATION_FORMAT)
    set_target_properties(${target_name} PROPERTIES MSVC_DEBUG_INFORMATION_FORMAT ProgramDatabase)
endif()
if(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    set_target_properties(${target_name} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

target_link_libraries(planet PRIVATE
    $<IF:$<TARGET_EXISTS:unofficial::noise::noise>,unofficial::noise::noise,unofficial::noise::noise-static>
)
target_link_libraries(planet 
                        PRIVATE 
                            xxHash::xxhash 
                            assimp::assimp
                            unofficial::enet::enet
                            fmt::fmt
                            Freetype::Freetype
                            GLEW::GLEW
                            glm::glm
                            assimp::assimp
                            slang::slang
                            simdutf::simdutf
                            SDL3::SDL3)
target_include_directories(planet PRIVATE ${Stb_INCLUDE_DIR})

if (WIN32)
    target_link_libraries(planet PRIVATE d3d11.lib dxgi.lib dxguid.lib d3dcompiler.lib)
endif()

target_compile_definitions(planet PRIVATE GLM_ENABLE_EXPERIMENTAL)
