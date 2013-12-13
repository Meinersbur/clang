set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_unittest(SemaTests
  ExternalSemaSourceTest.cpp
LINK_LIBS
  clangAST
  clangBasic
  clangFrontend
  clangParse
  clangSema
  clangTooling
  )
