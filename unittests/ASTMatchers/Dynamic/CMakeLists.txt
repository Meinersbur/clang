set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_unittest(DynamicASTMatchersTests
  VariantValueTest.cpp
  ParserTest.cpp
  RegistryTest.cpp
LINK_LIBS
  clangAST
  clangASTMatchers
  clangDynamicASTMatchers
  clangFrontend
  clangTooling
  )
