cmake_minimum_required(VERSION 2.8.4)
project(dg)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()


set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -x objective-c++")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
    SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
    SET (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
endif()

message("CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message("CMAKE_CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")

if(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    message(FATAL_ERROR "Using cmake wont work for you")
endif()

set(EXTERNAL_DIR_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(SOURCE_DIR_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(ASSETS_DIR_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(SHADERS_DIR_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders)
set(METAL_SHADERS_DIR_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/metal)

FILE(GLOB GFX_SOURCES ${SOURCE_DIR_ROOT}/render/*.cpp)
FILE(GLOB GFX_HEADERS ${SOURCE_DIR_ROOT}/render/*.h)

FILE(GLOB_RECURSE GFX_SOURCES2 ${SOURCE_DIR_ROOT}/render/backend/interface/*.cpp)
FILE(GLOB_RECURSE GFX_HEADERS2 ${SOURCE_DIR_ROOT}/render/backend/interface/*.h)

FILE(GLOB RENDERERS_SOURCES ${SOURCE_DIR_ROOT}/render/renderers/*.cpp)
FILE(GLOB RENDERERS_HEADERS ${SOURCE_DIR_ROOT}/render/renderers/*.h)
FILE(GLOB RENDERERS_CHUNKED_TERRAIN_SOURCES ${SOURCE_DIR_ROOT}/render/renderers/chunkedterrain/*.cpp)
FILE(GLOB RENDERERS_CHUNKED_TERRAIN_HEADERS ${SOURCE_DIR_ROOT}/render/renderers/chunkedterrain/*.h)
FILE(GLOB RENDERERS_DEBUG_SOURCES ${SOURCE_DIR_ROOT}/render/renderers/debug/*.cpp)
FILE(GLOB RENDERERS_DEBUG_HEADERS ${SOURCE_DIR_ROOT}/render/renderers/debug/*.h)
FILE(GLOB RENDERERS_TERRAIN_SOURCES ${SOURCE_DIR_ROOT}/render/renderers/terrain/*.cpp)
FILE(GLOB RENDERERS_TERRAIN_HEADERS ${SOURCE_DIR_ROOT}/render/renderers/terrain/*.h)
FILE(GLOB RENDERERS_TEXT_SOURCES ${SOURCE_DIR_ROOT}/render/renderers/text/*.cpp)
FILE(GLOB RENDERERS_TEXT_HEADERS ${SOURCE_DIR_ROOT}/render/renderers/text/*.h)
FILE(GLOB RENDERERS_SKYBOX_SOURCES ${SOURCE_DIR_ROOT}/render/renderers/skybox/*.cpp)
FILE(GLOB RENDERERS_SKYBOX_HEADERS ${SOURCE_DIR_ROOT}/render/renderers/skybox/*.h)
FILE(GLOB RENDERERS_UI_SOURCES ${SOURCE_DIR_ROOT}/render/renderers/ui/*.cpp)
FILE(GLOB RENDERERS_UI_HEADERS ${SOURCE_DIR_ROOT}/render/renderers/ui/*.h)

FILE(GLOB GFX_BACKEND_SOURCES ${SOURCE_DIR_ROOT}/render/backend/*.cpp)
FILE(GLOB GFX_BACKEND_HEADERS ${SOURCE_DIR_ROOT}/render/backend/*.h)

FILE(GLOB GFX_BACKEND_GL_SOURCES ${SOURCE_DIR_ROOT}/render/backend/impl/gl/*.cpp)    
FILE(GLOB GFX_BACKEND_GL_HEADERS ${SOURCE_DIR_ROOT}/render/backend/impl/gl/*.h)    
FILE(GLOB GFX_BACKEND_METAL_SOURCES ${SOURCE_DIR_ROOT}/render/backend/impl/metal/*.cpp ${SOURCE_DIR_ROOT}/render/backend/impl/metal/*.mm)    
FILE(GLOB GFX_BACKEND_METAL_HEADERS ${SOURCE_DIR_ROOT}/render/backend/impl/metal/*.h)    


FILE(GLOB GFX_BACKEND_COMMON_SOURCES ${SOURCE_DIR_ROOT}/render/backend/impl/common/*.cpp)
FILE(GLOB GFX_BACKEND_COMMON_HEADERS ${SOURCE_DIR_ROOT}/render/backend/impl/common/*.h)

FILE(GLOB INPUT_SOURCES ${SOURCE_DIR_ROOT}/input/*.cpp)
FILE(GLOB INPUT_HEADERS ${SOURCE_DIR_ROOT}/input/*.h)

FILE(GLOB UTILITIES_SOURCES ${SOURCE_DIR_ROOT}/utilities/*.cpp)
FILE(GLOB UTILITIES_HEADERS ${SOURCE_DIR_ROOT}/utilities/*.h)
FILE(GLOB UTILITIES_WIN32_SOURCES ${SOURCE_DIR_ROOT}/utilities/win32/*.cpp)
FILE(GLOB UTILITIES_WIN32_HEADERS ${SOURCE_DIR_ROOT}/utilities/win32/*.h)
FILE(GLOB UTILITIES_OSX_SOURCES ${SOURCE_DIR_ROOT}/utilities/osx/*.cpp)
FILE(GLOB UTILITIES_OSX_HEADERS ${SOURCE_DIR_ROOT}/utilities/osx/*.h)

FILE(GLOB APP_SOURCES ${SOURCE_DIR_ROOT}/*.cpp)
FILE(GLOB APP_HEADERS ${SOURCE_DIR_ROOT}/*.h)

FILE(GLOB COMPONENT_HEADERS ${SOURCE_DIR_ROOT}/components/*.h)
FILE(GLOB COMPONENT_SOURCES ${SOURCE_DIR_ROOT}/components/*.cpp)

FILE(GLOB EXT_HEADERS ${SOURCE_DIR_ROOT}/ext/*.h)

FILE(GLOB UI_SOURCES ${SOURCE_DIR_ROOT}/ui/*.cpp)
FILE(GLOB UI_HEADERS ${SOURCE_DIR_ROOT}/ui/*.h)

FILE(GLOB PLATFORM_SOURCES ${SOURCE_DIR_ROOT}/platform/*.cpp)
FILE(GLOB PLATFORM_HEADERS ${SOURCE_DIR_ROOT}/platform/*.h)

FILE(GLOB PLATFORM_SDL_SOURCES ${SOURCE_DIR_ROOT}/platform/sdl/*.cpp)
FILE(GLOB PLATFORM_SDL_HEADERS ${SOURCE_DIR_ROOT}/platform/sdl/*.h)
# FILE(GLOB PLATFORM_GLFW3_SOURCES ${SOURCE_DIR_ROOT}/platform/GLFW3/*.cpp)
# FILE(GLOB PLATFORM_GLFW3_HEADERS ${SOURCE_DIR_ROOT}/platform/GLFW3/*.h)

set(CMAKE_EXE_LINKER_FLAGS
    "-framework QuartzCore -framework Metal"
)

message("${QUARTZCORE_LIBRARY} ${METAL_LIBRARY}")

find_package(OpenGL REQUIRED)
find_package(LIBNOISE REQUIRED)
find_package(SDL2 REQUIRED)
find_package(FreeType REQUIRED)
find_package(Assimp REQUIRED)

include_directories(
    ${SOURCE_DIR_ROOT}/ 
    ${SOURCE_DIR_ROOT}/render/
    ${SOURCE_DIR_ROOT}/render/renderers/
    ${SOURCE_DIR_ROOT}/render/renderers/chunkedterrain/
    ${SOURCE_DIR_ROOT}/render/renderers/terrain/
    ${SOURCE_DIR_ROOT}/render/renderers/ui/
    ${SOURCE_DIR_ROOT}/render/renderers/skybox/
    ${SOURCE_DIR_ROOT}/render/renderers/debug/
    ${SOURCE_DIR_ROOT}/render/renderers/text/
    ${SOURCE_DIR_ROOT}/render/backend/
    ${SOURCE_DIR_ROOT}/render/backend/interface
    ${SOURCE_DIR_ROOT}/render/backend/interface/tasks
    ${SOURCE_DIR_ROOT}/render/backend/interface/enums    
    ${SOURCE_DIR_ROOT}/render/backend/impl/metal
    ${SOURCE_DIR_ROOT}/render/backend/impl/gl
    ${SOURCE_DIR_ROOT}/render/backend/impl/common
    ${SOURCE_DIR_ROOT}/render/
    ${SOURCE_DIR_ROOT}/utilities/
    ${SOURCE_DIR_ROOT}/utilities/win32
    ${SOURCE_DIR_ROOT}/utilities/osx
    ${SOURCE_DIR_ROOT}/platform/
    ${SOURCE_DIR_ROOT}/platform/SDL/
    ${SOURCE_DIR_ROOT}/components
    ${SOURCE_DIR_ROOT}/input/
    ${SOURCE_DIR_ROOT}/ui/
    ${SOURCE_DIR_ROOT}/ext/
    /usr/local/include
    ${LIBNOISE_INCLUDE_DIRS} 
    ${SDL2_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${EXTERNAL_DIR_ROOT}/include/enum-flags/include
)

set(LIBS  
    ${OPENGL_LIBRARIES}  
    ${LIBNOISE_LIBRARIES}
    ${SDL2_LIBRARY}
    ${FREETYPE_LIBRARIES}
    ${ASSIMP_LIBRARIES}
)

link_directories(/usr/local/lib)

# create application
FILE(GLOB_RECURSE ALL_H RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCE_DIR_ROOT}/*.h)
FILE(GLOB_RECURSE ALL_METAL RELATIVE ${SHADERS_DIR_ROOT} ${METAL_SHADERS_DIR_ROOT}/*.metal)

add_executable(dg 
    ${SOURCE_DIR_ROOT}/main.cpp
    ${GFX_SOURCES} 
    ${GFX_SOURCES2}
    ${GFX_BACKEND_SOURCES} 
    ${GFX_BACKEND_METAL_SOURCES} 
    ${GFX_BACKEND_GL_SOURCES} 
    ${GFX_BACKEND_COMMON_SOURCES}
    ${RENDERERS_SOURCES}
    ${COMPONENT_SOURCES}
    ${RENDERERS_CHUNKED_TERRAIN_SOURCES}
    ${RENDERERS_TERRAIN_SOURCES}
    ${RENDERERS_DEBUG_SOURCES}
    ${RENDERERS_SKYBOX_SOURCES}
    ${RENDERERS_UI_SOURCES}
    ${RENDERERS_TEXT_SOURCES}
    ${INPUT_SOURCES}
    ${UTILITIES_SOURCES}
    ${UTILITIES_OSX_SOURCES}
    ${APP_SOURCES}
    ${PLATFORM_SOURCES}
    ${PLATFORM_SDL_SOURCES}
    ${UI_SOURCES}
    ${ALL_H} #need to have ALL_H files in this otherwise they wont be added to sourcegroups
     ${METAL_SHADERS_DIR_ROOT}/blinn.metal)

set_source_files_properties( ${METAL_SHADERS_DIR_ROOT}/blinn.metal PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(dg ${LIBS})

# create source groups
FILE(GLOB_RECURSE ALL_CPP RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${SOURCE_DIR_ROOT}/*.cpp ${SOURCE_DIR_ROOT}/*.mm)
set(ALL_FILES ${ALL_H} ${ALL_CPP})

foreach(FILE ${ALL_FILES}) 
  get_filename_component(PARENT_DIR "${FILE}" PATH)

  # skip src or include and changes /'s to \\'s
  string(REGEX REPLACE "(\\./)?(src|include)/?" "" GROUP "${PARENT_DIR}")
  string(REPLACE "/" "\\" GROUP "${GROUP}")

  set(GROUP "src\\${GROUP}")  
  source_group("${GROUP}" FILES "${FILE}")
endforeach()
