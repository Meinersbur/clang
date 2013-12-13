set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_unittest(ASTMatchersTests
  ASTMatchersTest.cpp
LINK_LIBS
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
  )

add_subdirectory(Dynamic)
