set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Skip FetchContent update checks on reconfigure")
include(FetchContent)

function(_renderer_fetch_dep name repo tag)
  set(_src_dir "${FETCHCONTENT_BASE_DIR}/${name}-src")
  set(_tag_file "${FETCHCONTENT_BASE_DIR}/${name}-subbuild/.fetched_tag")

  set(_saved_disconnected "${FETCHCONTENT_FULLY_DISCONNECTED}")
  if(EXISTS "${_src_dir}" AND EXISTS "${_tag_file}")
    file(READ "${_tag_file}" _cached_tag)
    string(STRIP "${_cached_tag}" _cached_tag)
    if("${_cached_tag}" STREQUAL "${tag}")
      set(FETCHCONTENT_FULLY_DISCONNECTED ON)
    endif()
  endif()

  FetchContent_Declare(
          ${name}
          GIT_REPOSITORY ${repo}
          GIT_TAG        ${tag}
  )
  FetchContent_MakeAvailable(${name})

  set(FETCHCONTENT_FULLY_DISCONNECTED "${_saved_disconnected}")
  if(EXISTS "${_src_dir}")
    file(MAKE_DIRECTORY "${FETCHCONTENT_BASE_DIR}/${name}-subbuild")
    file(WRITE "${_tag_file}" "${tag}\n")
  endif()

  string(TOUPPER "${name}" _name_upper)
  foreach(_var IN ITEMS
          "${name}_SOURCE_DIR" "${name}_BINARY_DIR" "${name}_POPULATED"
          "${_name_upper}_SOURCE_DIR" "${_name_upper}_BINARY_DIR" "${_name_upper}_POPULATED")
    if(DEFINED ${_var})
      set(${_var} "${${_var}}" PARENT_SCOPE)
    endif()
  endforeach()
endfunction()

# Fetch GLFW
_renderer_fetch_dep(
        glfw
        https://github.com/glfw/glfw.git
        92dcf4ce74f2e2554a98fea09be7c705c17daa5a # master
)

# Fetch GLAD
_renderer_fetch_dep(
        glad
        https://github.com/Dav1dde/glad.git
        e86f90457371c6233053bacf0d6f486a51ddcd67 # master
)

# Fetch GLM
_renderer_fetch_dep(
        glm
        https://github.com/g-truc/glm.git
        6f14f4792a0cde5d0cf2c910506724d61cb95834 # master
)

# Fetch stb
_renderer_fetch_dep(
        stb_image
        https://github.com/nothings/stb.git
        9600522de53973336fa033a289858a4354cbb04c # master
)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE
        ${stb_image_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Fetch Dear ImGui
_renderer_fetch_dep(
        imgui
        https://github.com/ocornut/imgui.git
        cf53467ab0db7a783f04b32dd9160e6aa5b720ef # docking
)

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

# Fetch SPlisHSPlasH (library-only to avoid duplicate glfw/imgui targets and Simulator apps)
set(SPH_LIBS_ONLY ON CACHE BOOL "" FORCE)
set(USE_PYTHON_BINDINGS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(_saved_module_path "${CMAKE_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH "${FETCHCONTENT_BASE_DIR}/splishsplash-src/CMake")
get_property(_has_saved_ep_disconnected DIRECTORY PROPERTY EP_UPDATE_DISCONNECTED DEFINED)
if(_has_saved_ep_disconnected)
  get_property(_saved_ep_disconnected DIRECTORY PROPERTY EP_UPDATE_DISCONNECTED)
endif()
set_property(DIRECTORY PROPERTY EP_UPDATE_DISCONNECTED ON)

_renderer_fetch_dep(
        splishsplash
        https://github.com/InteractiveComputerGraphics/SPlisHSPlasH.git
        dba6b26b9387ab6bdd187d6df0a7c9d490d03500 # master
)

if(_has_saved_ep_disconnected)
  set_property(DIRECTORY PROPERTY EP_UPDATE_DISCONNECTED "${_saved_ep_disconnected}")
else()
  set_property(DIRECTORY PROPERTY EP_UPDATE_DISCONNECTED OFF)
endif()
unset(_has_saved_ep_disconnected)
unset(_saved_ep_disconnected)

get_directory_property(_sph_defs DIRECTORY "${splishsplash_SOURCE_DIR}" COMPILE_DEFINITIONS)
if(_sph_defs)
  target_compile_definitions(SPlisHSPlasH PUBLIC ${_sph_defs})
endif()
get_directory_property(_sph_opts DIRECTORY "${splishsplash_SOURCE_DIR}" COMPILE_OPTIONS)
if(_sph_opts)
  target_compile_options(SPlisHSPlasH PUBLIC ${_sph_opts})
endif()

set(CMAKE_MODULE_PATH "${_saved_module_path}")
unset(_saved_module_path)

# Prevent SPlisHSPlasH/CMake/Common.cmake from redirecting Renderer's executable output to ${CMAKE_SOURCE_DIR}/bin
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY CACHE)
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE CACHE)
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG)
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE)
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO)
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL)

# SPlisHSPlasH's ExternalProject_Add calls do not declare BUILD_BYPRODUCTS, which causes
# Ninja builds of `--target Renderer` from a fresh build directory to fail with missing
# `CompactNSearch*.lib` / `Discregrid*.lib`. Register their output rules explicitly.
if(CMAKE_GENERATOR MATCHES "Ninja")
  set(_sph_ext_suffix "")
  if(EXT_CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_sph_ext_suffix "_d")
  endif()
  set(_sph_ns_lib "${CMAKE_BINARY_DIR}/extern/install/NeighborhoodSearch/lib/${CMAKE_STATIC_LIBRARY_PREFIX}CompactNSearch${_sph_ext_suffix}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set(_sph_dg_lib "${CMAKE_BINARY_DIR}/extern/install/Discregrid/lib/${CMAKE_STATIC_LIBRARY_PREFIX}Discregrid${_sph_ext_suffix}${CMAKE_STATIC_LIBRARY_SUFFIX}")

  add_custom_command(
          OUTPUT "${_sph_ns_lib}"
          COMMAND ${CMAKE_COMMAND} -E touch_nocreate "${_sph_ns_lib}"
          DEPENDS "${splishsplash_BINARY_DIR}/CMakeFiles/Ext_NeighborhoodSearch-complete"
          COMMENT "Ensuring CompactNSearch library timestamp"
  )
  add_custom_command(
          OUTPUT "${_sph_dg_lib}"
          COMMAND ${CMAKE_COMMAND} -E touch_nocreate "${_sph_dg_lib}"
          DEPENDS "${splishsplash_BINARY_DIR}/CMakeFiles/Ext_Discregrid-complete"
          COMMENT "Ensuring Discregrid library timestamp"
  )
  add_custom_target(_sph_external_libs DEPENDS "${_sph_ns_lib}" "${_sph_dg_lib}")
  add_dependencies(SPlisHSPlasH _sph_external_libs)
endif()

