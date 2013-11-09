
add_clang_unittest(BasicTests
  CharInfoTest.cpp
  FileManagerTest.cpp
  SourceManagerTest.cpp
LINK_LIBS
  clangBasic
  clangLex
  )
