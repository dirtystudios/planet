cmake_minimum_required(VERSION 2.8.4)
project(dg)


if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -std=c++11 -g")
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

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")





FILE(GLOB GRAPHICS_BACKEND_SOURCES code/app/src/rendering/backend/*.cpp)
FILE(GLOB GRAPHICS_BACKEND_HEADERS code/app/src/rendering/backend/*.h)
FILE(GLOB GRAPHICS_GL_BACKEND_SOURCES code/app/src/rendering/backend/GL/*.cpp)
FILE(GLOB GRAPHICS_DX11_BACKEND_HEADERS code/app/src/rendering/backend/DX11/*.h)

FILE(GLOB INPUT_SOURCES code/app/src/input/*.cpp)
FILE(GLOB INPUT_HEADERS code/app/src/input/*.h)

FILE(GLOB UTILITIES_SOURCES code/app/src/utilities/*.cpp)
FILE(GLOB UTILITIES_HEADERS code/app/src/utilities/*.h)

FILE(GLOB APP_SOURCES code/app/src/*.cpp)
FILE(GLOB APP_HEADERS code/app/src/*.h)

FILE(GLOB PLATFORM_SOURCES code/app/src/platform/*.cpp)
FILE(GLOB PLATFORM_HEADERS code/app/src/platform/*.h)

FILE(GLOB PLATFORM_SDL_SOURCES code/app/src/platform/SDL/*.cpp)
FILE(GLOB PLATFORM_SDL_HEADERS code/app/src/platform/SDL/*.h)
FILE(GLOB PLATFORM_GLFW3_SOURCES code/app/src/platform/GLFW3/*.cpp)
FILE(GLOB PLATFORM_GLFW3_HEADERS code/app/src/platform/GLFW3/*.h)

find_package(OpenGL REQUIRED)
find_package(LIBNOISE REQUIRED)
find_package(SDL2 REQUIRED)
find_package(FreeType REQUIRED)

include_directories(
    "code/app/src/" 
    "/usr/local/include"
    ${LIBNOISE_INCLUDE_DIRS} 
    ${SDL2_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
)

set(LIBS
    ${OPENGL_LIBRARIES}
    ${LIBNOISE_LIBRARIES}
    ${SDL2_LIBRARY}S}
    ${FREETYPE_LIBRARIES}
)


# create application
add_executable(dg code/app/src/main.cpp 
    ${GRAPHICS_BACKEND_SOURCES} 
    ${GRAPHICS_GL_BACKEND_SOURCES} 
    ${INPUT_SOURCES}
    ${UTILITIES_SOURCES}
    ${APP_SOURCES}
    ${PLATFORM_SOURCES}
    ${PLATFORM_SDL_SOURCES}
)

target_link_libraries(dg ${LIBS})





