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

message("CMAKE_CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(OpenGL REQUIRED)
find_package(LIBNOISE REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLFW REQUIRED)

#set(SDL2_BUILDING_LIBRARY true)
#add_subdirectory("SDL-Mirror")

# We only want the static library to build
# set_target_properties(SDL2 SDL2main PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)

if (WIN32)
    set(GLEW_ROOT_DIR ${CMAKE_SOURCE_DIR})
    find_package(GLEW REQUIRED)
    add_definitions(-DGLEW_STATIC)
    #i give up
    set(SDL2_INCLUDE_DIR "SDL-Mirror/include")
    set(SDL2_LIBRARIES "../winlibs/SDL2")
    
    set(PLATFORM_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
elseif(APPLE)
    set(PLATFORM_INCLUDE_DIRS "/usr/local/include")     
endif()


include_directories(
    ${PLATFORM_INCLUDE_DIRS}          
    ${LIBNOISE_INCLUDE_DIRS} 
    ${GLEW_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR2}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(LIBS        
    ${OPENGL_LIBRARIES}    
    ${LIBNOISE_LIBRARIES}
    ${GLEW_LIBRARY}
    ${SDL2_LIBRARY}
    ${GLFW_LIBRARIES}
)

set(GFX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/DebugRenderer.cpp
)

set(GFX_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/DebugRenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ChunkedLODTerrainRenderer.h
)

set(GFX_GL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/gfx/gl/GLRenderDevice.cpp
)

set(GFX_GL_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/gfx/gl/GLRenderDevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GLHelpers.h
)

set(UTIL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/File.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.cpp
}

set(UTIL_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/File.h
    ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h
}

set(GAME_SOURCES    
    ${CMAKE_CURRENT_SOURCE_DIR}/App.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SystemSDL2.cpp
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
    ${UTIL_SOURCES}
)

set(ALL_HEADERS   
    ${GAME_HEADERS} 
    ${GFX_HEADERS}
    ${GFX_GL_HEADERS}
    ${UTIL_SOURCES}
)

message("rawr: ${SDL2_LIBRARY}")

# create application
add_executable(dg ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${ALL_SOURCES} ${ALL_HEADERS})
target_link_libraries(dg ${LIBS})

# Source groups for correct organization in Xcode
source_group(src FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${GAME_SOURCES} ${GAME_HEADERS})
source_group(src\\gfx FILES ${GFX_SOURCES} ${GFX_HEADERS})
source_group(src\\gfx\\gl FILES ${GFX_GL_SOURCES} ${GFX_GL_HEADERS})