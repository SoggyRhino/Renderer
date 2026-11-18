function(copy_assets)
  cmake_parse_arguments(ARG "" "TARGET" "DIRECTORIES" ${ARGN})

  if(NOT ARG_TARGET)
    message(FATAL_ERROR "copy_assets: TARGET is required")
  endif()

  get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  get_target_property(_runtime_out ${ARG_TARGET} RUNTIME_OUTPUT_DIRECTORY)

  set(stamp_files "")
  foreach(dir IN LISTS ARG_DIRECTORIES)
    get_filename_component(dir_name "${dir}" NAME)
    file(GLOB_RECURSE asset_files CONFIGURE_DEPENDS "${dir}/*")

    if(_is_multi_config)
      set(stamp_file "${CMAKE_BINARY_DIR}/CMakeFiles/copy_assets_${dir_name}_$<CONFIG>.stamp")
    else()
      set(stamp_file "${CMAKE_BINARY_DIR}/CMakeFiles/copy_assets_${dir_name}.stamp")
    endif()

    set(copy_cmds
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${dir}" "$<TARGET_FILE_DIR:${ARG_TARGET}>/${dir_name}"
    )
    if(_is_multi_config OR _runtime_out)
      list(APPEND copy_cmds
              COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
              "${dir}" "${CMAKE_BINARY_DIR}/${dir_name}"
      )
    endif()

    add_custom_command(
            OUTPUT "${stamp_file}"
            ${copy_cmds}
            COMMAND ${CMAKE_COMMAND} -E touch "${stamp_file}"
            DEPENDS ${asset_files}
            COMMENT "Syncing '${dir_name}' assets to output directory"
            VERBATIM
    )
    list(APPEND stamp_files "${stamp_file}")
  endforeach()

  add_custom_target(copy_assets ALL
          DEPENDS ${stamp_files}
  )

  add_dependencies(${ARG_TARGET} copy_assets)
endfunction()