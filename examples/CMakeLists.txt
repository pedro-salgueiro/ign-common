cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)

# Find the ignition-common library
find_package(ignition-common0 REQUIRED)

add_subdirectory(plugins)

file(GLOB example_files "*.cc")
list(SORT example_files)

foreach(example_file ${example_files})

  get_filename_component(example ${example_file} NAME_WE)

  add_executable(${example} ${example_file})

  target_link_libraries(${example} PRIVATE ignition-common0::ignition-common0)

  # This is only needed for the examples that use plugins, but it doesn't hurt
  # to set it for all the examples.
  target_compile_definitions(${example}
    PRIVATE
      "IGN_COMMON_EXAMPLE_PLUGIN_LIBDIR=\"${PLUGIN_LIBDIR}\"")

endforeach()
