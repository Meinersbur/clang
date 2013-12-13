set(LLVM_LINK_COMPONENTS
  Option
  Support
  )

add_clang_executable(clang-check
  ClangCheck.cpp
  LINK_LIBS
  clangAST
  clangBasic
  clangDriver
  clangFrontend
  clangRewriteFrontend
  clangStaticAnalyzerFrontend
  clangTooling
  )

install(TARGETS clang-check
  RUNTIME DESTINATION bin)
