# ===========================================================================
# CMakeLists.txt – Desktop (Windows) build for TouchEvents-Modern
#
# Shared C++ source is in Scene/. The same Scene/ code is used by:
#   - This file          → native desktop executable (GLFW + GLEW)
#   - android/.../cpp/   → Android shared library (.so) via NDK
#   - build_web.sh       → WebAssembly/WebGL via Emscripten
#
# Usage:
#   mkdir build && cd build
#   cmake .. -G "Visual Studio 17 2022" -A x64
#   cmake --build . --config Debug
# ===========================================================================
cmake_minimum_required(VERSION 3.15)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
cmake_policy(VERSION 3.15)
project(TouchEvents)
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SCENE_DIR ${CMAKE_CURRENT_LIST_DIR}/Scene)
file(GLOB SCENE_SRC "${SCENE_DIR}/*.cpp")
if(EMSCRIPTEN)
    # --- Web build: use Emscripten's built-in GLFW port, no fetching needed ---
    add_definitions(-DPLATFORM_EMSCRIPTEN -DUSE_GLFW)

    # GLM is header-only and platform-agnostic — still needed here even
    # though we skip GLFW/GLEW fetching for the web build.
    include(FetchContent)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(glm)

    add_executable(TouchEvents ${SCENE_SRC})
    target_include_directories(TouchEvents PRIVATE ${SCENE_DIR})
    target_link_libraries(TouchEvents PRIVATE glm::glm)

    target_compile_options(TouchEvents PRIVATE -sUSE_GLFW=3 -sFULL_ES3=1)
    target_link_options(TouchEvents PRIVATE
        -sUSE_GLFW=3
        -sFULL_ES3=1
        -sWASM=1
        --preload-file "${CMAKE_CURRENT_LIST_DIR}/android/app/src/main/assets@assets"
    )
    set_target_properties(TouchEvents PROPERTIES SUFFIX ".html")
else()
    # --- Desktop (Windows) build: fetch real GLFW/GLEW/GLM ---
    include(ImportDependencies)
    importDependencies()
    add_definitions(-DPLATFORM_WINDOWS -DGLEW_STATIC)
    add_executable(TouchEvents ${SCENE_SRC})
    target_include_directories(TouchEvents PRIVATE ${SCENE_DIR})
    target_link_libraries(TouchEvents
        PRIVATE glfw
        PRIVATE libglew_static glm::glm
    )
    if(MSVC)
        target_compile_options(TouchEvents PRIVATE /W4 /WX- /permissive- /std:c++17)
    endif()
    add_custom_command(TARGET TouchEvents POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_LIST_DIR}/android/app/src/main/assets"
            "$<TARGET_FILE_DIR:TouchEvents>/assets"
        COMMENT "Copying shader assets to build directory"
    )
    set_target_properties(TouchEvents PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:TouchEvents>"
    )
endif()