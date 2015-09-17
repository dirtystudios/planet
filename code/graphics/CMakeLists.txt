FILE(GLOB GRAPHICS_SOURCES src/*.cpp)
FILE(GLOB GRAPHICS_HEADERS src/*.h)

FILE(GLOB GRAPHICS_GL_SOURCES src/GL/*.cpp src/GL/*/*.cpp)
FILE(GLOB GRAPHICS_GL_HEADERS src/GL/*.cpp src/GL/*/*.cpp)

find_package(OpenGL REQUIRED)
message("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(WIN32)
    set(GLEW_ROOT_DIR ${CMAKE_SOURCE_DIR})
    find_package(GLEW REQUIRED)
    add_definitions(-DGLEW_STATIC)
    include_directories(AFTER
       ${GLEW_INCLUDE_DIRS}
    )
endif()

set(GRAPHICS_LIBRARY_DEPS
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
)

add_library(graphics STATIC ${GRAPHICS_SOURCES} ${GRAPHICS_HEADERS} ${GRAPHICS_GL_SOURCES} ${GRAPHICS_GL_HEADERS})
target_link_libraries(graphics ${GRAPHICS_LIBRARY_DEPS})
