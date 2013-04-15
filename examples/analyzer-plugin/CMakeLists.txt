set(MODULE TRUE)

set( LLVM_LINK_COMPONENTS support mc)

add_clang_library(SampleAnalyzerPlugin MainCallChecker.cpp
  LINK_LIBS clangStaticAnalyzerCore
  )

add_dependencies(SampleAnalyzerPlugin
  ClangAttrClasses
  ClangAttrList
  ClangCommentNodes
  ClangDeclNodes
  ClangDiagnosticCommon
  ClangStmtNodes
  )

set_target_properties(SampleAnalyzerPlugin
  PROPERTIES
  LINKER_LANGUAGE CXX
  PREFIX "")
