cmake_minimum_required(VERSION 3.27)
project(Renderer)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MINGW)
    add_link_options(-static-libstdc++ -static-libgcc)
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)

include(FetchContent)

# Fetch GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(glfw)

# Fetch GLAD
FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(glad)

# Fetch GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(glm)

# Fetch stb
FetchContent_Declare(
        stb_image
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(stb_image)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE
        ${stb_image_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Fetch Dear ImGui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        docking # or a release tag like v1.91.5
)
FetchContent_MakeAvailable(imgui)

# Define ImGui static library along with GLFW and OpenGL3 backends
add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)

# Link GLFW and GLAD so the ImGui backends can access glfw3.h and OpenGL headers
target_link_libraries(imgui PUBLIC
        glfw
        glad
)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/utils/Window.cpp
        src/utils/Window.h
        src/utils/Program.cpp
        src/utils/Program.h
        src/scenes/Scene.cpp
        src/scenes/Scene.h
        src/scenes/SquareTestScene.cpp
        src/scenes/SquareTestScene.h
        src/Paths.h
        src/utils/Buffers.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
        glad
        glfw
        glm
        OpenGL::GL
        stb_image
        imgui
)

set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/shaders)

add_custom_target(copy_shaders ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADERS_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADERS_DIR} ${CMAKE_BINARY_DIR}/shaders
        COMMENT "Copying shaders to target output directory"
)
add_dependencies(${PROJECT_NAME} copy_shaders)

if(MINGW)
    get_filename_component(MINGW_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    if(EXISTS "${MINGW_BIN_DIR}/libwinpthread-1.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>/libwinpthread-1.dll
                COMMENT "Copying MinGW runtime DLLs to output directory"
        )
    endif()
endif()
