set(LLVM_LINK_COMPONENTS bitreader)

add_clang_library(clangSerialization
  ASTCommon.cpp
  ASTReader.cpp
  ASTReaderDecl.cpp
  ASTReaderStmt.cpp
  ASTWriter.cpp
  ASTWriterDecl.cpp
  ASTWriterStmt.cpp
  GeneratePCH.cpp
  GlobalModuleIndex.cpp
  Module.cpp
  ModuleManager.cpp
  ADDITIONAL_HEADERS
  ASTCommon.h
  ASTReaderInternals.h
LINK_LIBS
  clangSema
  )

add_dependencies(clangSerialization
  ClangAttrClasses
  ClangAttrList
  ClangAttrParsedAttrList
  ClangAttrPCHRead
  ClangAttrPCHWrite
  ClangCommentNodes
  ClangDeclNodes
  ClangDiagnosticCommon
  ClangDiagnosticLex
  ClangDiagnosticSema
  ClangDiagnosticSerialization
  ClangStmtNodes
  )
