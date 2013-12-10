set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangFormat
  BreakableToken.cpp
  ContinuationIndenter.cpp
  Format.cpp
  FormatToken.cpp
  TokenAnnotator.cpp
  UnwrappedLineParser.cpp
  WhitespaceManager.cpp
LINK_LIBS
  clangBasic
  clangFrontend
  clangAST
  clangASTMatchers
  clangRewriteCore
  clangRewriteFrontend
  clangTooling
  )

add_dependencies(clangFormat
  ClangAttrClasses
  ClangAttrList
  ClangDeclNodes
  ClangDiagnosticCommon
  ClangDiagnosticFrontend
  ClangStmtNodes
  )
