set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangTooling
  ArgumentsAdjusters.cpp
  CommonOptionsParser.cpp
  CompilationDatabase.cpp
  FileMatchTrie.cpp
  JSONCompilationDatabase.cpp
  Refactoring.cpp
  RefactoringCallbacks.cpp
  Tooling.cpp
LINK_LIBS
  clangBasic
  clangFrontend
  clangAST
  clangASTMatchers
  clangRewriteCore
  clangRewriteFrontend
  )

add_dependencies(clangTooling
  ClangAttrClasses
  ClangAttrList
  ClangDeclNodes
  ClangDiagnosticCommon
  ClangDiagnosticFrontend
  ClangStmtNodes
  )
