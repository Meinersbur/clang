set(LLVM_LINK_COMPONENTS
  support
  )

add_clang_executable(arcmt-test
  arcmt-test.cpp
LINK_LIBS 
  clangARCMigrate
  clangBasic
  clangFrontend
  clangLex
  )
