set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_library(clangEdit
  Commit.cpp
  EditedSource.cpp
  RewriteObjCFoundationAPI.cpp
  LINK_LIBS clangBasic clangAST clangLex
  )

add_dependencies(clangEdit
  ClangAttrClasses
  ClangAttrList
  ClangCommentNodes
  ClangDeclNodes
  ClangDiagnosticCommon
  ClangStmtNodes
  )
