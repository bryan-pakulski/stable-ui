cmake_minimum_required(VERSION 3.21)
project(stable-ui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")

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
)
FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    add_subdirectory(
            ${imgui_SOURCE_DIR}
    )
endif()

# Dear ImGui library configuration
add_library(
        imgui
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

# Embed python
find_package (Python REQUIRED
        COMPONENTS Interpreter Development)

add_executable( stable-ui 
                main.cpp 
                src/Rendering/RenderManager.cpp
                src/Rendering/Camera.cpp
                src/Rendering/Canvas.cpp 
                src/Rendering/Helper.cpp 
                src/Rendering/objects/BaseObject.cpp 
                src/Rendering/objects/Selection.cpp
                src/Rendering/objects/image/Image.cpp
                src/Rendering/objects/chunk/Chunk.cpp
                src/SDInterface/SDCommandsInterface.cpp 
                src/SDInterface/py/SnakeHandler.cpp
)

target_include_directories(
        stable-ui PRIVATE
        ${OPENGL_INCLUDE_DIR}
        ${glad_BINARY_DIR}/include
        ${glfw_SOURCE_DIR}/include
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${imgui_SOURCE_DIR}/misc/cpp
        ${glm_SOURCE_DIR}/src
        ${Python_INCLUDE_DIRS}
)

target_link_directories(
        stable-ui PRIVATE
        ${Python_LIBRARY_DIRS}
)

target_link_libraries(
        stable-ui PRIVATE
        imgui
        glad
        glfw
        glm
        ${CMAKE_HOME_DIRECTORY}/lib/libyaml-cpp.a
        ${OPENGL_LIBRARY}
        ${Python_LIBRARIES}
)