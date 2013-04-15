set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangFormat
  TokenAnnotator.cpp
  UnwrappedLineParser.cpp
  Format.cpp
  LINK_LIBS clangBasic clangFrontend clangAST clangASTMatchers clangRewriteCore clangRewriteFrontend clangTooling
  )

add_dependencies(clangFormat
  ClangAttrClasses
  ClangAttrList
  ClangDeclNodes
  ClangDiagnosticCommon
  ClangDiagnosticFrontend
  ClangStmtNodes
  )
