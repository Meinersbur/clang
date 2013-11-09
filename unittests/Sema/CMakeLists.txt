add_clang_unittest(SemaTests
  ExternalSemaSourceTest.cpp
LINK_LIBS
  clangAST clangASTMatchers clangTooling
  )
