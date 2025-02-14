# CMakeLists.txt for Towers vs. Invaders

cmake_minimum_required(VERSION 3.11)  # Updated version
project(ramster)

include(FetchContent)

# Using CMake FetchContent to install Box2D
# Ref: https://github.com/tupini07/raylib-cpp-cmake-template/blob/main/CMakeLists.txt
function(add_git_dependency libName gitURL gitTag)
    FetchContent_Declare(${libName}
        GIT_REPOSITORY ${gitURL}
        GIT_TAG        ${gitTag}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(${libName})
    # This assumes that the fetched target is named exactly like libName.
    # Adjust the target name if necessary.
    target_compile_options(${libName} PRIVATE "-w")
endfunction()

# use C++17
set(CMAKE_CXX_STANDARD 17)

# nice hierarchical structure in MSVC
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Add Box2D: Turn off tests for a cleaner build
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
add_git_dependency(
    box2d
    https://github.com/erincatto/box2d.git
    v2.4.1
)

# detect OS
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(IS_OS_MAC 1)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(IS_OS_LINUX 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(IS_OS_WINDOWS 1)
else()
    message(FATAL_ERROR "OS ${CMAKE_SYSTEM_NAME} was not recognized")
endif()

# Create executable target

# Generate the shader folder location to the header
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ext/project_path.hpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/ext/project_path.hpp")

# You can switch to use file GLOB for simplicity but at your own risk
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

# external libraries will be installed into /usr/local/include and /usr/local/lib but that folder is not automatically included on MACs
if (IS_OS_MAC)
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
    # Added for M-series Macs
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC src/)

# Added this so policy CMP0065 doesn't scream
set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS 0)

# External header-only libraries in the ext/
target_include_directories(${PROJECT_NAME} PUBLIC ext/stb_image/)
target_include_directories(${PROJECT_NAME} PUBLIC ext/gl3w)

# Find OpenGL
find_package(OpenGL REQUIRED)

if (OPENGL_FOUND)
   target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
   target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENGL_gl_LIBRARY})
endif()

set(glm_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glm/cmake/glm) # if necessary
find_package(glm REQUIRED)

# Platform-specific configuration
if (IS_OS_LINUX OR IS_OS_MAC)
    # Use pkg-config to find GLFW and SDL on OSX/Linux
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW REQUIRED glfw3)
    pkg_search_module(SDL2 REQUIRED sdl2)
    pkg_search_module(SDL2MIXER REQUIRED SDL2_mixer)

    # Link Frameworks on OSX
    if (IS_OS_MAC)
       find_library(COCOA_LIBRARY Cocoa)
       find_library(CF_LIBRARY CoreFoundation)
       target_link_libraries(${PROJECT_NAME} PUBLIC ${COCOA_LIBRARY} ${CF_LIBRARY})
    endif()

    # Increase warning level
    target_compile_options(${PROJECT_NAME} PUBLIC "-Wall")
elseif (IS_OS_WINDOWS)
    # Precompiled libraries for Windows
    set(GLFW_FOUND TRUE)
    set(SDL2_FOUND TRUE)

    # Include directories
    set(GLFW_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/include")
    set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/include/SDL")

    # Library files
    set(GLFW_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3dll-x64.lib")
    set(SDL2_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x64.lib")
    set(SDL2MIXER_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x64.lib")

    # Matching DLLs
    set(GLFW_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3-x64.dll")
    set(SDL_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x64.dll")
    set(SDLMIXER_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x64.dll")

    # Copy DLLs to build folder
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLFW_DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/glfw3.dll")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL_DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2.dll")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDLMIXER_DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2_mixer.dll")

    # Increase warning level and adjust compile options for Windows
    add_compile_options(/w4)
    add_compile_options(/we4715)
    add_compile_options(/EHsc)
    add_compile_options(/we4239)
endif()

# Check that both GLFW and SDL were found
if (NOT GLFW_FOUND OR NOT SDL2_FOUND)
    if (NOT GLFW_FOUND)
        message(FATAL_ERROR "Can't find GLFW.")
    else()
        message(FATAL_ERROR "Can't find SDL.")
    endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${GLFW_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC ${GLFW_LIBRARIES} ${SDL2_LIBRARIES} ${SDL2MIXER_LIBRARIES} glm::glm)

# Ensure Box2D's include directory is available and link against it
target_include_directories(${PROJECT_NAME} PRIVATE ${box2d_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE box2d)

# Additional linking for Linux
if (IS_OS_LINUX)
    target_link_libraries(${PROJECT_NAME} PUBLIC glfw ${CMAKE_DL_LIBS})
endif()
