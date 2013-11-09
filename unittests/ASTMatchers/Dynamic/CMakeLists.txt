add_clang_unittest(DynamicASTMatchersTests
  VariantValueTest.cpp
  ParserTest.cpp
  RegistryTest.cpp
LINK_LIBS
  gtest gtest_main clangASTMatchers clangDynamicASTMatchers clangTooling)
