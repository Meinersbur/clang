set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  asmparser
  bitreader
  support
  mc
  )

add_clang_executable(clang-check
  ClangCheck.cpp
  LINK_LIBS clangTooling clangBasic clangRewriteFrontend
  )

install(TARGETS clang-check
  RUNTIME DESTINATION bin)
