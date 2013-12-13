set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_unittest(FrontendTests
  FrontendActionTest.cpp
LINK_LIBS
  clangAST
  clangFrontend
  )
