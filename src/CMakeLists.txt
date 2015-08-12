include (${project_cmake_dir}/Utils.cmake)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (HAVE_GTS)
  include_directories(${gts_INCLUDE_DIRS})
  link_directories(${gts_LIBRARY_DIRS})
  add_definitions(${gts_CFLAGS})
endif()


include_directories(${tinyxml_INCLUDE_DIRS})
link_directories(${tinyxml_LIBRARY_DIRS})

set (sources
  Animation.cc
  Assert.cc
  AudioDecoder.cc
  BVHLoader.cc
  ColladaLoader.cc
  ColladaExporter.cc
  Color.cc
  Console.cc
  Exception.cc
  Image.cc
  KeyFrame.cc
  Material.cc
  Mesh.cc
  MeshExporter.cc
  MeshLoader.cc
  MeshManager.cc
  NodeTransform.cc
  Skeleton.cc
  SkeletonAnimation.cc
  SkeletonNode.cc
  STLLoader.cc
  Time.cc
  Timer.cc
  Util.cc
)

if (HAVE_GTS)
set (sources
  ${sources}
  GTSMeshUtils.cc
  MeshCSG.cc
)
endif()


set (gtest_sources
  Animation_TEST.cc
  AudioDecoder_TEST.cc
  Color_TEST.cc
  Console_TEST.cc
  Exception_TEST.cc
  Image_TEST.cc
  Time_TEST.cc
  Util_TEST.cc
)

ign_add_library(ignition-common ${sources})
ign_install_library(ignition-common)

target_link_libraries(ignition-common
  ${IGNITION-MATH_LIBRARIES}
  ${Boost_LIBRARIES}
  ${freeimage_LIBRARIES}
  ${tinyxml_LIBRARIES}
)

if (HAVE_GTS)
  target_link_libraries(ignition-common ${gts_LIBRARIES})
endif()


ign_build_tests(${gtest_sources})
