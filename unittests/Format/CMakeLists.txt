set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_unittest(FormatTests
  FormatTest.cpp
  FormatTestJS.cpp
  FormatTestProto.cpp
LINK_LIBS
  clangFormat
  clangTooling
  )
