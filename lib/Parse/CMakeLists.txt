set(LLVM_LINK_COMPONENTS
  MCParser
  MC
  Support
  )

add_clang_library(clangParse
  ParseAST.cpp
  ParseCXXInlineMethods.cpp
  ParseDecl.cpp
  ParseDeclCXX.cpp
  ParseExpr.cpp
  ParseExprCXX.cpp
  ParseInit.cpp
  ParseObjc.cpp
  ParseOpenMP.cpp
  ParsePragma.cpp
  ParseStmt.cpp
  ParseTemplate.cpp
  ParseTentative.cpp
  Parser.cpp
  ParseMolly.cpp
  LINK_LIBS clangBasic clangAST clangLex clangSema
  )

add_dependencies(clangParse
  ClangAttrClasses
  ClangAttrParserStringSwitches
  ClangAttrList
  ClangAttrParsedAttrList
  ClangCommentNodes
  ClangDeclNodes
  ClangDiagnosticCommon
  ClangDiagnosticParse
  ClangStmtNodes
  )
