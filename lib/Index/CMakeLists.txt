set(LLVM_LINK_LIBS
  CommentToXML.cpp
  SimpleFormatContext.h
  clangBasic
  clangAST
  clangFormat
)

add_clang_library(clangIndex
  USRGeneration.cpp
  )
