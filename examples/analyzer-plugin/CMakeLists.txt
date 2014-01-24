set( LLVM_LINK_COMPONENTS support mc)

add_clang_library(SampleAnalyzerPlugin MODULE MainCallChecker.cpp)

add_dependencies(SampleAnalyzerPlugin
  ClangAttrClasses
  ClangAttrList
  ClangCommentNodes
  ClangDeclNodes
  ClangDiagnosticCommon
  ClangStmtNodes
LINK_LIBS
  clangStaticAnalyzerCore
  )

set_target_properties(SampleAnalyzerPlugin
  PROPERTIES
  LINKER_LANGUAGE CXX
  PREFIX "")
