set(LLVM_NO_RTTI 1)

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/../Checkers )

set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_library(clangStaticAnalyzerFrontend
  AnalysisConsumer.cpp
  CheckerRegistration.cpp
  FrontendActions.cpp
  LINK_LIBS clangBasic clangLex clangAST clangFrontend clangRewriteCore clangRewriteFrontend clangStaticAnalyzerCheckers
  )

add_dependencies(clangStaticAnalyzerFrontend
  clangStaticAnalyzerCheckers
  clangStaticAnalyzerCore
  ClangAttrClasses
  ClangAttrList
  ClangCommentNodes
  ClangDeclNodes
  ClangDiagnosticCommon
  ClangDiagnosticFrontend
  ClangStmtNodes
  )

  clangAnalysis
  clangBasic
  clangStaticAnalyzerCore