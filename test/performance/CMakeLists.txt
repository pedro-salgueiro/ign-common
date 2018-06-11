ign_get_sources(tests)

# plugin_specialization test causes lcov to hang
# see ign-cmake issue 25
if("${CMAKE_BUILD_TYPE_UPPERCASE}" STREQUAL "COVERAGE")
  list(REMOVE_ITEM tests
    plugin_specialization.cc)
endif()

ign_build_tests(
  TYPE PERFORMANCE
  SOURCES ${tests})

if(TARGET PERFORMANCE_plugin_specialization)

  # We add this dependency to make sure that DummyPlugins gets generated
  # before INTEGRATION_plugin so that its auto-generated header is available.
  # We do not want to link INTEGRATION_plugin to the DummyPlugins library.
  add_dependencies(INTEGRATION_plugin IGNDummyPlugins)

  # This executable needs to be able to find the path to the dummy plugin
  # libraries, which can only be determined at generation time for
  # multi-configuration generator types (like Visual Studio).
  target_compile_definitions(PERFORMANCE_plugin_specialization PRIVATE
    "DETAIL_IGN_DUMMY_PLUGIN_PATH=\"$<TARGET_FILE_DIR:IGNDummyPlugins>\"")

endif()
