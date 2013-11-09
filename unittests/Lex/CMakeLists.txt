add_clang_unittest(LexTests
  LexerTest.cpp
  PPCallbacksTest.cpp
  PPConditionalDirectiveRecordTest.cpp
LINK_LIBS
  clangLex clangParse clangSema
  )
