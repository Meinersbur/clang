add_clang_executable(c-arcmt-test
  c-arcmt-test.c
  LINK_LIBS libclang
  )

set_target_properties(c-arcmt-test
  PROPERTIES
  LINKER_LANGUAGE CXX)
