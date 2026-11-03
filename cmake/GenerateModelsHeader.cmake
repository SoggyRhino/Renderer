if(NOT COMMAND _assets_write_if_different)
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
  function(_assets_identifier out text)
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" id "${text}")
    if(id MATCHES "^[0-9]")
      set(id "_${id}")
    endif()
    set(${out} "${id}" PARENT_SCOPE)
  endfunction()
endif()

function(generate_models_header)
  cmake_parse_arguments(ARG "" "MODEL_DIR;OUTPUT_DIR" "EXTENSIONS" ${ARGN})

  if(NOT ARG_MODEL_DIR)
    set(ARG_MODEL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/models")
  endif()
  if(NOT ARG_OUTPUT_DIR)
    set(ARG_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated")
  endif()
  if(NOT ARG_EXTENSIONS)
    set(ARG_EXTENSIONS gltf)
  endif()

  file(MAKE_DIRECTORY "${ARG_OUTPUT_DIR}")

  set(patterns "")
  foreach(ext IN LISTS ARG_EXTENSIONS)
    list(APPEND patterns "${ARG_MODEL_DIR}/*.${ext}")
  endforeach()
  file(GLOB_RECURSE model_files CONFIGURE_DEPENDS ${patterns})
  list(SORT model_files)

  set(model_decls "")
  set(model_names "")
  foreach(file IN LISTS model_files)
    get_filename_component(stem "${file}" NAME_WE)
    _assets_identifier(name "${stem}")

    # Same stem in different folders: use the relative path instead.
    list(FIND model_names "${name}" found)
    if(NOT found EQUAL -1)
      file(RELATIVE_PATH rel "${ARG_MODEL_DIR}" "${file}")
      string(REGEX REPLACE "\\.[^./]*$" "" rel "${rel}")
      _assets_identifier(name "${rel}")
      message(WARNING "Model name collision for '${file}', using ${name}")
    endif()
    list(APPEND model_names "${name}")

    file(TO_CMAKE_PATH "${file}" file_path)
    string(APPEND model_decls
           "    inline constexpr ModelPath ${name}{\"${file_path}\"};\n")
  endforeach()

  list(LENGTH model_names model_count)
  string(REPLACE ";" ", " model_list "${model_names}")

  set(content
"#pragma once
#include <array>
#include <cstddef>
#include <string_view>

struct ModelPath {
    std::string_view value;
    constexpr explicit ModelPath(std::string_view v) : value(v) {}
    constexpr operator std::string_view() const { return value; }
};

namespace Models {
${model_decls}
    inline constexpr std::size_t count = ${model_count};
    inline constexpr std::array<ModelPath, ${model_count}> all{${model_list}};
}
")

  _assets_write_if_different("${ARG_OUTPUT_DIR}/models.h" "${content}")
endfunction()
