add_library(IGNBadPluginAlign         SHARED BadPluginAlign.cc)
add_library(IGNBadPluginAPIVersionNew SHARED BadPluginAPIVersionNew.cc)
add_library(IGNBadPluginAPIVersionOld SHARED BadPluginAPIVersionOld.cc)
add_library(IGNBadPluginSize          SHARED BadPluginSize.cc)
add_library(IGNDummyPlugins SHARED
  DummyPlugins.cc
  DummyPluginsOtherTranslationUnit.cc)

foreach(plugin_target
    IGNBadPluginAlign
    IGNBadPluginAPIVersionNew
    IGNBadPluginAPIVersionOld
    IGNBadPluginSize
    IGNDummyPlugins)

  # We link the plugin libraries to the plugin framework library. If we move
  # the plugin framework out of the main ign-common library, we should link to
  # the new plugin framework library instead of the main ign-common library.
  target_link_libraries(${plugin_target} PRIVATE ${PROJECT_LIBRARY_TARGET_NAME})

endforeach()
