# GenerateShadersHeader.cmake
#
# generate_shaders_header(
#     [SHADER_DIR <dir>]    # default: ${CMAKE_CURRENT_SOURCE_DIR}/shaders
#     [OUTPUT_DIR <dir>]    # default: ${CMAKE_BINARY_DIR}/generated
# )
#
# Writes <OUTPUT_DIR>/shaders.h with each shader's source embedded directly:
#   triangle.vert -> inline constexpr std::string_view TRIANGLE = R"(...source...)";
# (name = upper(filename.split(".")[0]); on a collision, e.g. triangle.frag,
#  falls back to the full filename: TRIANGLE_FRAG)
#
# Editing a shader re-runs CMake, so the header is regenerated on the next build.

if(NOT COMMAND _assets_write_if_different)
  # Write file only if content differs (avoids needless rebuilds).
  function(_assets_write_if_different path content)
    set(existing "")
    if(EXISTS "${path}")
      file(READ "${path}" existing)
    endif()
    if(NOT "${existing}" STREQUAL "${content}")
      file(WRITE "${path}" "${content}")
    endif()
  endfunction()
endif()

if(NOT COMMAND _assets_identifier)
  # Turn arbitrary text into a valid C identifier.
  function(_assets_identifier out text)
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" id "${text}")
    if(id MATCHES "^[0-9]")
      set(id "_${id}")
    endif()
    set(${out} "${id}" PARENT_SCOPE)
  endfunction()
endif()

function(generate_shaders_header)
  cmake_parse_arguments(ARG "" "SHADER_DIR;OUTPUT_DIR" "" ${ARGN})

  if(NOT ARG_SHADER_DIR)
    set(ARG_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
  endif()
  if(NOT ARG_OUTPUT_DIR)
    set(ARG_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated")
  endif()

  file(MAKE_DIRECTORY "${ARG_OUTPUT_DIR}")
  file(GLOB_RECURSE shader_files CONFIGURE_DEPENDS "${ARG_SHADER_DIR}/*")
  list(SORT shader_files)

  set(delim "__SHADER__")

  set(content "#pragma once\n#include <string_view>\n\n")
  set(seen_names "")
  foreach(file IN LISTS shader_files)
    if(IS_DIRECTORY "${file}")
      continue()
    endif()
    get_filename_component(fname "${file}" NAME)

    string(TOUPPER "${fname}" full)
    _assets_identifier(name "${full}")

    list(FIND seen_names "${name}" found)
    if(NOT found EQUAL -1)
      message(WARNING "Shader name collision for '${fname}', using ${name}")
    endif()
    list(APPEND seen_names "${name}")

    # Re-run CMake when the shader's contents change (not just when files are added/removed).
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${file}")

    file(READ "${file}" source)
    string(REPLACE "\r\n" "\n" source "${source}")

    string(FIND "${source}" ")${delim}\"" bad_pos)
    if(NOT bad_pos EQUAL -1)
      message(FATAL_ERROR "Shader '${fname}' contains the raw-string terminator )${delim}\"")
    endif()

    string(APPEND content
           "inline constexpr std::string_view ${name} = R\"${delim}(${source})${delim}\";\n\n")
  endforeach()

  _assets_write_if_different("${ARG_OUTPUT_DIR}/shaders.h" "${content}")
endfunction()
