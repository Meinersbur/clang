set(LLVM_LINK_COMPONENTS support)

add_clang_library(clangASTMatchers
  ASTMatchFinder.cpp
  ASTMatchersInternal.cpp
  LINK_LIBS clangBasic clangAST
  )

add_dependencies(clangASTMatchers
  ClangAttrClasses
  ClangAttrImpl
  ClangAttrList
  ClangCommentNodes
  ClangDeclNodes
  ClangDiagnosticCommon
  ClangStmtNodes
  )
