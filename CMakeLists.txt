cmake_minimum_required(VERSION 2.8.4)
project(dg)


if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -std=c++11 -g")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
endif()

message("CMAKE_CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(LIBNOISE REQUIRED)

if(NOT ${GLFW3_FOUND})
    message(FATAL_ERROR "Failure to find GLFW3")
endif()


include_directories(
    /usr/local/include         
    ${GLFW_INCLUDE_DIRS}     
    ${LIBNOISE_INCLUDE_DIRS} 
)

set(LIBS        
    ${GLFW_LIBRARIES}
    ${OPENGL_LIBRARIES}    
    ${LIBNOISE_LIBRARIES}
)

set(GFX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/DebugRenderer.cpp
)

set(GFX_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/DebugRenderer.h
)

set(GFX_GL_SOURCES

)

set(GFX_GL_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/GLHelpers.h
)

set(GAME_SOURCES    
    ${CMAKE_CURRENT_SOURCE_DIR}/App.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SystemGLFW3.cpp        
    ${CMAKE_CURRENT_SOURCE_DIR}/Camera.cpp        
    ${CMAKE_CURRENT_SOURCE_DIR}/Frustum.cpp        
)

set(GAME_HEADERS    
    ${CMAKE_CURRENT_SOURCE_DIR}/Application.h
    ${CMAKE_CURRENT_SOURCE_DIR}/App.h
    ${CMAKE_CURRENT_SOURCE_DIR}/System.h        
    ${CMAKE_CURRENT_SOURCE_DIR}/Log.h    
    ${CMAKE_CURRENT_SOURCE_DIR}/Camera.h    
    ${CMAKE_CURRENT_SOURCE_DIR}/Frustum.h    
)

set(ALL_SOURCES        
    ${GAME_SOURCES}
    ${GFX_SOURCES}
    ${GFX_GL_SOURCES}
)

set(ALL_HEADERS   
    ${GAME_HEADERS} 
    ${GFX_HEADERS}
    ${GFX_GL_HEADERS}
)

# create application
add_executable(dg ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${ALL_SOURCES} ${ALL_HEADERS})
target_link_libraries(dg ${LIBS})

# Source groups for correct organization in Xcode
source_group(src FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${GAME_SOURCES} ${GAME_HEADERS})
source_group(src\\gfx FILES ${GFX_SOURCES} ${GFX_HEADERS})
source_group(src\\gfx\\gl FILES ${GFX_GL_SOURCES} ${GFX_GL_HEADERS})