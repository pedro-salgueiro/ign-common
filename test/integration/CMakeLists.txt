set(TEST_TYPE "INTEGRATION")

if (HAVE_LIBDL)
  set(tests
    plugin.cc
  )
endif()

include_directories(.)

link_directories(${PROJECT_BINARY_DIR}/test)

ign_build_tests(${tests})
