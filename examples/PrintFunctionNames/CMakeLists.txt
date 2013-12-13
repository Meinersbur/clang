set(MODULE TRUE)

set( LLVM_LINK_COMPONENTS support mc)

add_clang_library(PrintFunctionNames 
  PrintFunctionNames.cpp
LINK_LIBS
  clangFrontend
  clangAST
  )

add_dependencies(PrintFunctionNames
  ClangAttrClasses
  ClangAttrList
  ClangCommentNodes
  ClangDeclNodes
  ClangDiagnosticCommon
  ClangStmtNodes
  )

set_target_properties(PrintFunctionNames
  PROPERTIES
  LINKER_LANGUAGE CXX
  PREFIX "")
