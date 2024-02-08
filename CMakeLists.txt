cmake_minimum_required(VERSION 3.20)
project(stable-ui)

set(CMAKE_CXX_STANDARD 20)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (UNIX)
        set(CMAKE_CXX_FLAGS "-Wall -Wextra")
        set(CMAKE_CXX_FLAGS_DEBUG "-g")
        set(CMAKE_CXX_FLAGS_RELEASE "-O3")
else()
        set(CMAKE_CXX_FLAGS "/Wall")
        set(CMAKE_CXX_FLAGS_DEBUG "/Z7")
        set(CMAKE_CXX_FLAGS_RELEASE "/O2")
        include(C:/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()


# for clangd language server
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable caching of fetched content
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

include(FetchContent)

# Download and fetch GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
)

# download and configure the glad project
FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
)
FetchContent_MakeAvailable(glad glm)
FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    FetchContent_Populate(glad)
    set(
            GLAD_PROFILE "core"
            CACHE STRING "OpenGL profile"
    )
    set(
            GLAD_API "gl=4.6"
            CACHE STRING "API type/version pairs, "
            "like \"gl=3.2,gles=\", no version means latest"
    )
    set(
            GLAD_GENERATOR "c"
            CACHE STRING "Language to generate the binding for")
    add_subdirectory(
            ${glad_SOURCE_DIR}
            ${glad_BINARY_DIR}
    )
endif()

# download and configure the GLFW project
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
)
FetchContent_MakeAvailable(glfw)
FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)

    set(GLFW_BUILD_DOCS     off CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    off CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES off CACHE BOOL "" FORCE)

    add_subdirectory(
            ${glfw_SOURCE_DIR}
            ${glfw_BINARY_DIR}
    )
endif()

# download the Dear ImGUI project
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.89.7-docking
)
FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    add_subdirectory(
            ${imgui_SOURCE_DIR}
    )
endif()

# Dear ImGui library configuration, built as static
add_library(
        imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

target_compile_options(
        imgui PRIVATE
        -DIMGUI_IMPL_OPENGL_LOADER_GLAD
)

target_include_directories(
        imgui PRIVATE
        ${imgui_SOURCE_DIR}
        ${glad_BINARY_DIR}/include
        ${glfw_SOURCE_DIR}/include
)

find_package(OpenGL REQUIRED)  # for Dear ImGui
find_package(Threads REQUIRED) # for GLFW
find_package(X11 REQUIRED)     # for GLFW

find_package(ZLIB REQUIRED)

# Download and fetch yaml-cpp
FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG 0.8.0
)

set(YAML_CPP_BUILD_TESTS OFF)
FetchContent_MakeAvailable(yaml-cpp)

# Zero MQ should be install as part of the pipeline or using vcpkg
if (WIN32)
        find_package(ZeroMQ CONFIG REQUIRED)
endif()

add_executable( stable-ui 
                src/main.cpp
                src/StableManager.cpp
                src/Client/Heartbeat.cpp
                src/Client/StableClient.cpp
                src/Helpers/GLHelper.cpp
                src/Indexer/Indexer.cpp
                src/Rendering/OrthographicCamera.cpp
                src/Rendering/Canvas.cpp 
                src/Rendering/RenderManager.cpp
                src/Rendering/objects/BaseObject.cpp 
                src/Rendering/objects/Selection.cpp
                src/Rendering/objects/Layer.cpp
                src/Rendering/objects/Image.cpp
                src/Rendering/objects/GLImage/GLImage.cpp
                src/ThirdParty/sha1/sha1.cpp
                src/ThirdParty/base64/base64.c
        )

# XMP has to be built manually (see src/ThirdParty/build_xmp_libraries.sh)
target_include_directories(
        stable-ui PRIVATE
        ${OPENGL_INCLUDE_DIR}
        ${glad_BINARY_DIR}/include
        ${glfw_SOURCE_DIR}/include
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${imgui_SOURCE_DIR}/misc/cpp
        ${glm_SOURCE_DIR}/src
        ${CMAKE_HOME_DIRECTORY}/src
        ${CMAKE_HOME_DIRECTORY}/src/ThirdParty/XMP/XMP-Toolkit-SDK/public/include
)

target_link_directories(
        stable-ui PRIVATE
)

if(UNIX)
        if(DYNAMIC_LIBS EQUAL "ON")
                target_link_libraries(
                stable-ui PRIVATE
                imgui
                glad
                glfw
                glm
                yaml-cpp
                zmq
                ZLIB::ZLIB
                ${CMAKE_SOURCE_DIR}/lib/libXMPCore.so
                ${CMAKE_SOURCE_DIR}/lib/libXMPFiles.so
                ${OPENGL_LIBRARY}
                )
        else()
                set(ZLIB_USE_STATIC_LIBS "ON")

                target_link_libraries(
                stable-ui PRIVATE
                imgui
                glad
                glfw
                glm
                yaml-cpp
                zmq
                ZLIB::ZLIB
                ${CMAKE_SOURCE_DIR}/lib/staticXMPCore.ar
                ${CMAKE_SOURCE_DIR}/lib/staticXMPFiles.ar
                ${OPENGL_LIBRARY}
                )
        endif()
        
else()
        target_link_libraries(
                stable-ui PRIVATE
                imgui
                glad
                glfw
                glm
                yaml-cpp
                libzmq-static
                ZLIB::ZLIB
                ${CMAKE_SOURCE_DIR}/lib/XMPCore.lib
                ${CMAKE_SOURCE_DIR}/lib/XMPFiles.lib
                ${OPENGL_LIBRARY}
        )
endif()