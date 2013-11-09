set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  asmparser
  bitreader
  support
  mc
  )

add_clang_unittest(FormatTests
  FormatTest.cpp
LINK_LIBS
  clangAST
  clangFormat
  clangTooling
  clangRewriteCore
  )
