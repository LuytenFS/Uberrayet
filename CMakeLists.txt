cmake_minimum_required(VERSION 3.21)
project(Uberrayet LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
include(FetchContent)

# SDL3
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.2.10
    GIT_SHALLOW    TRUE
)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)

# SDL3_image
FetchContent_Declare(
    SDL3_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG        release-3.2.0  # Pin to release tag instead of main
    GIT_SHALLOW    TRUE
)
set(SDLIMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3_image)

# glad
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.6
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  cmake
)
FetchContent_MakeAvailable(glad)

glad_add_library(glad_gl_core_46 REPRODUCIBLE API gl:core=4.6)

# Vulkan SDK
find_package(Vulkan REQUIRED)

# vk-bootstrap
FetchContent_Declare(
    vk-bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
    GIT_TAG        v1.3.302
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(vk-bootstrap)

FetchContent_Declare(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG        v3.1.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(vma)

find_package(OpenGL REQUIRED)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
file(GLOB_RECURSE UBERRAYET_SOURCES 
    CONFIGURE_DEPENDS
    "uberrayet/*.cpp"
    "uberrayet/*.hpp"
    "uberrayet/*.h"
    "code/*.cpp"
    "code/*.hpp"
    "code/*.h"
)

add_executable(Uberrayet
    ${UBERRAYET_SOURCES}
)

target_link_libraries(Uberrayet PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
    glad_gl_core_46
    OpenGL::GL
    Vulkan::Vulkan
    vk-bootstrap::vk-bootstrap
    GPUOpen::VulkanMemoryAllocator
)

target_include_directories(Uberrayet PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/uberrayet
    ${CMAKE_SOURCE_DIR}/code
)

# Set runtime search path for Linux so libSDL3.so and libSDL3_image.so are found next to the executable
if (UNIX AND NOT APPLE)
    set_target_properties(Uberrayet PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# Copy shared libraries post-build
if (WIN32 OR (UNIX AND NOT APPLE))
    add_custom_command(TARGET Uberrayet POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE:SDL3_image::SDL3_image>
            $<TARGET_FILE_DIR:Uberrayet>
    )
endif()

# Sync assets folder
add_custom_command(TARGET Uberrayet POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:Uberrayet>/assets
)