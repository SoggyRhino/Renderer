function(copy_assets)
  cmake_parse_arguments(ARG "" "TARGET" "DIRECTORIES" ${ARGN})

  if(NOT ARG_TARGET)
    message(FATAL_ERROR "copy_assets: TARGET is required")
  endif()

  add_custom_target(copy_assets ALL
          COMMENT "Copying assets to target output directory")

  foreach(dir IN LISTS ARG_DIRECTORIES)
    get_filename_component(dir_name "${dir}" NAME)
    add_custom_command(TARGET copy_assets POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${dir}" "$<TARGET_FILE_DIR:${ARG_TARGET}>/${dir_name}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${dir}" "${CMAKE_BINARY_DIR}/${dir_name}"
    )
  endforeach()

  add_dependencies(${ARG_TARGET} copy_assets)
endfunction()