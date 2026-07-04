cmake_minimum_required(VERSION 3.20)
project(Advanced-Graphics-For-Games)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

# Specify FMOD location via environment variable (not here in env path variable) or CMake cache
set(FMOD_ROOT "$ENV{FMOD_ROOT}")

if(NOT FMOD_ROOT)
    message(FATAL_ERROR "FMOD_ROOT environment variable not set. Set it to your FMOD installation path (e.g., D:/Softwares/FMOD Studio API Windows/api/core)")
endif()

set(FMOD_INCLUDE_DIR "${FMOD_ROOT}/inc")
set(FMOD_LIB_DIR "${FMOD_ROOT}/lib/x64")

if(NOT EXISTS "${FMOD_INCLUDE_DIR}/fmod.hpp")
    message(FATAL_ERROR "FMOD headers not found at ${FMOD_INCLUDE_DIR}. Check your FMOD_ROOT path.")
endif()

# Add FMOD to your project
target_include_directories(Advanced-Graphics-For-Games PRIVATE ${FMOD_INCLUDE_DIR})
target_link_directories(Advanced-Graphics-For-Games PRIVATE ${FMOD_LIB_DIR})
target_link_libraries(Advanced-Graphics-For-Games PRIVATE
    $<$<CONFIG:Debug>:fmodL_vc>
    $<$<CONFIG:Release>:fmod_vc>
)