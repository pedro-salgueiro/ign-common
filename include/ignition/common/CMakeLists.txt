include (${project_cmake_dir}/Utils.cmake)

set (headers
  Base64.hh
  Battery.hh
  Console.hh
  EnumIface.hh
  Event.hh
  HeightmapData.hh
  KeyEvent.hh
  MovingWindowFilter.hh
  PluginInfo.hh
  PluginMacros.hh
  SemanticVersion.hh
  SingletonT.hh
  StringUtils.hh
  SubMesh.hh
  System.hh
  SystemPaths.hh
  Time.hh
  Timer.hh
  Types.hh
  Util.hh
  URI.hh
  WorkerPool.hh
)

if (HAVE_IGN_MATH)
  set (headers ${headers}
    Animation.hh
    BVHLoader.hh
    ColladaExporter.hh
    ColladaLoader.hh
    Dem.hh
    KeyFrame.hh
    Material.hh
    MaterialDensity.hh
    Mesh.hh
    MeshExporter.hh
    MeshLoader.hh
    MeshManager.hh
    MouseEvent.hh
    NodeAnimation.hh
    NodeTransform.hh
    OBJLoader.hh
    Skeleton.hh
    SkeletonAnimation.hh
    SkeletonNode.hh
    STLLoader.hh
    SVGLoader.hh
  )
endif (HAVE_IGN_MATH)

if (HAVE_UUID)
  set (headers ${headers}
    Uuid.hh
  )
endif (HAVE_UUID)

if (HAVE_LIBDL)
  set (headers ${headers}
    PluginLoader.hh
  )
endif (HAVE_LIBDL)

# FIXME: Some classes fail to build on Windows
if(NOT WIN32)
  set (headers ${headers}
    Video.hh
    VideoEncoder.hh
  )

  if (HAVE_IGN_MATH)
    set (headers ${headers}
      GTSMeshUtils.hh
    )
  endif()

  if (HAVE_AVCODEC)
    set (headers ${headers}
      AudioDecoder.hh
      ffmpeg_inc.hh
    )
  endif()

  if (HAVE_FREEIMAGE)
    set (headers ${headers}
      Image.hh
      ImageHeightmap.hh
    )
  endif (HAVE_FREEIMAGE)

  if (HAVE_GTS)
    set (headers ${headers}
      MeshCSG.hh
    )
  endif (HAVE_GTS)

endif()

set (ign_headers "" CACHE INTERNAL "Ignition headers" FORCE)
foreach (hdr ${headers})
  APPEND_TO_CACHED_STRING(ign_headers
    "Ignition headers" "#include <ignition/${IGN_PROJECT_NAME}/${hdr}>\n")
endforeach()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/ign_auto_headers.hh.in
  ${CMAKE_CURRENT_BINARY_DIR}/${IGN_PROJECT_NAME}.hh)

ign_install_includes("${INCLUDE_INSTALL_DIR_POSTFIX}/ignition" ${CMAKE_CURRENT_BINARY_DIR}/${IGN_PROJECT_NAME}.hh)
ign_install_includes("${INCLUDE_INSTALL_DIR_POSTFIX}/ignition/${IGN_PROJECT_NAME}" ${headers})
