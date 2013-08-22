set(LLVM_LINK_LIBS
  clangBasic
  clangAST
)

add_clang_library(clangIndex
  USRGeneration.cpp
  )
