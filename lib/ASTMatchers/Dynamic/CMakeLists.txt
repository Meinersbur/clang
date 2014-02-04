set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangDynamicASTMatchers
  Diagnostics.cpp
  VariantValue.cpp
  Parser.cpp
  Registry.cpp
LINK_LIBS
add_dependencies(clangDynamicASTMatchers
  clangAST
  clangASTMatchers
  clangBasic
  )
