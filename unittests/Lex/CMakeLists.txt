set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_unittest(LexTests
  LexerTest.cpp
  PPCallbacksTest.cpp
  PPConditionalDirectiveRecordTest.cpp
LINK_LIBS
  clangAST
  clangBasic
  clangLex
  clangParse
  clangSema
  )
