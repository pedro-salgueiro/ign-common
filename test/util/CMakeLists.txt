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
  target_compile_options(${plugin_target} PRIVATE
    -fkeep-inline-functions
    -fno-gnu-unique
    )
endforeach()
