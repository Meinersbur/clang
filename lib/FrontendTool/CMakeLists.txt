add_clang_library(clangFrontendTool
  ExecuteCompilerInvocation.cpp
  LINK_LIBS clangDriver clangFrontend clangRewriteCore clangRewriteFrontend clangCodeGen clangStaticAnalyzerFrontend clangStaticAnalyzerCheckers clangStaticAnalyzerCore clangARCMigrate
  )

add_dependencies(clangFrontendTool
  ClangDiagnosticCommon
  ClangDiagnosticFrontend
  ClangDriverOptions
  )
