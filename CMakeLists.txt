cmake_minimum_required(VERSION 3.27)
project(Renderer)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MINGW)
    add_link_options(-static-libstdc++ -static-libgcc)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dependencies.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateShadersHeader.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateModelsHeader.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyAssets.cmake)

set(GENERATED_DIR ${CMAKE_SOURCE_DIR}/src/generated)

generate_shaders_header(
        SHADER_DIR ${CMAKE_SOURCE_DIR}/shaders
        OUTPUT_DIR ${GENERATED_DIR}
)
generate_models_header(
        MODEL_DIR ${CMAKE_SOURCE_DIR}/models
        OUTPUT_DIR ${GENERATED_DIR}
)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/graphics/runtime/Window.cpp
        src/graphics/runtime/Window.h
        src/graphics/runtime/Input.h
        src/graphics/runtime/ImGuiLayer.cpp
        src/graphics/runtime/ImGuiLayer.h
        src/graphics/rendering/wrappers/RenderState.h
        src/graphics/rendering/wrappers/Program.cpp
        src/graphics/rendering/wrappers/Program.h
        src/graphics/rendering/wrappers/Camera.cpp
        src/graphics/rendering/wrappers/Camera.h
        src/scenes/Scene.h
        src/scenes/tests/SquareTestScene.cpp
        src/scenes/tests/SquareTestScene.h
        src/graphics/rendering/wrappers/Buffers.h
        src/scenes/tests/GLTFTestScene.cpp
        src/scenes/tests/GLTFTestScene.h
        src/scenes/tests/AllModelsTestScene.cpp
        src/scenes/tests/AllModelsTestScene.h
        src/graphics/rendering/objects/Mesh.cpp
        src/graphics/rendering/objects/Mesh.h
        src/graphics/rendering/objects/Model.cpp
        src/graphics/rendering/objects/Model.h
        src/graphics/rendering/wrappers/Texture.cpp
        src/graphics/rendering/wrappers/Texture.h

        lib/tiny_gltf_v3.c
        src/scenes/Scene.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${GENERATED_DIR}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE TINYGLTF3_ENABLE_FS)

find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
        glad
        glfw
        glm
        OpenGL::GL
        stb_image
        imgui
)

copy_assets(
        TARGET ${PROJECT_NAME}
        DIRECTORIES
        ${CMAKE_SOURCE_DIR}/models
)

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
