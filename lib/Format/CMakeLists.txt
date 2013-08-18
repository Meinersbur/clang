set(LLVM_LINK_COMPONENTS support)

set(LLVM_LINK_LIBS
  clangBasic
  clangFrontend
  clangAST
  clangASTMatchers
  clangRewriteCore
  clangRewriteFrontend
  clangTooling
  )

add_clang_library(clangFormat
  BreakableToken.cpp
  ContinuationIndenter.cpp
  Format.cpp
  TokenAnnotator.cpp
  UnwrappedLineParser.cpp
  WhitespaceManager.cpp
  )

add_dependencies(clangFormat
  ClangAttrClasses
  ClangAttrList
  ClangDeclNodes
  ClangDiagnosticCommon
  ClangDiagnosticFrontend
  ClangStmtNodes
  )
