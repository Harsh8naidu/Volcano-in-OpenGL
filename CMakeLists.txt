cmake_minimum_required(VERSION 3.20)
project(Advanced-Graphics-For-Games)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenEXR CONFIG REQUIRED)

# nclgl static library
file(GLOB_RECURSE NCLGL_SOURCES "nclgl/*.cpp")
add_library(nclgl STATIC ${NCLGL_SOURCES})
target_include_directories(nclgl PUBLIC
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/Third Party"
)

# Third Party Libraries
file(GLOB_RECURSE GLAD_SOURCES "Third Party/glad/*.c")
add_library(glad STATIC ${GLAD_SOURCES})
target_include_directories(glad PUBLIC
    "${CMAKE_SOURCE_DIR}/Third Party"
)

file(GLOB_RECURSE SOIL_SOURCES "Third Party/SOIL/Simple OpenGL Image Library/src/src/*.c")
add_library(soil STATIC ${SOIL_SOURCES})
target_include_directories(soil PUBLIC
    "${CMAKE_SOURCE_DIR}/Third Party/SOIL/Simple OpenGL Image Library/src/include"
    "${CMAKE_SOURCE_DIR}/Third Party/SOIL"
)
target_link_libraries(nclgl PRIVATE soil)

# main executable
file(GLOB_RECURSE SOURCES "Blank Project/*.cpp")
add_executable(Advanced-Graphics-For-Games ${SOURCES})

target_include_directories(Advanced-Graphics-For-Games PRIVATE
    "${CMAKE_SOURCE_DIR}/Blank Project"
    "${CMAKE_SOURCE_DIR}/Third Party/glad"
    "${CMAKE_SOURCE_DIR}/Third Party/KHR"
    "${CMAKE_SOURCE_DIR}/Third Party/SOIL"
)

target_link_libraries(Advanced-Graphics-For-Games PRIVATE
    nclgl
    glad
    SOIL
    OpenEXR::OpenEXR
    opengl32
)

target_compile_definitions(Advanced-Graphics-For-Games PRIVATE
    PROJECT_ROOT="${CMAKE_SOURCE_DIR}/"
)