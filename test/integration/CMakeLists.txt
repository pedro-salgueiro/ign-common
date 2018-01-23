# This is to make test_config.h visible
include_directories("${CMAKE_BINARY_DIR}")

ign_get_sources(tests)

# FIXME the mesh test does not work
list(REMOVE_ITEM tests mesh.cc)

ign_build_tests(
  TYPE INTEGRATION
  SOURCES ${tests})

if(TARGET INTEGRATION_plugin)

  # We add this dependency to make sure that DummyPlugins gets generated
  # before INTEGRATION_plugin so that its auto-generated header is available.
  # We do not want to link INTEGRATION_plugin to the DummyPlugins library.
  add_dependencies(INTEGRATION_plugin IGNDummyPlugins)

  # This test uses the DL library, so we need to link it.
  target_link_libraries(INTEGRATION_plugin ${DL_TARGET})

  # This executable needs to be able to find the path to the dummy plugin
  # libraries, which can only be determined at generation time for
  # multi-configuration generator types (like Visual Studio).
  target_compile_definitions(INTEGRATION_plugin PRIVATE
    "DETAIL_IGN_DUMMY_PLUGIN_PATH=\"$<TARGET_FILE_DIR:IGNDummyPlugins>\"")

endif()
