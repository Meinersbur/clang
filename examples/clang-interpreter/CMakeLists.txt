set(LLVM_LINK_COMPONENTS
  Core
  ExecutionEngine
  JIT
  Support
  nativecodegen
  )

add_clang_executable(clang-interpreter
  main.cpp
  LINK_LIBS clangFrontend clangSerialization clangDriver clangCodeGen clangSema clangStaticAnalyzerFrontend clangStaticAnalyzerCheckers clangStaticAnalyzerCore clangAnalysis clangRewriteCore clangRewriteFrontend clangAST clangParse clangLex clangBasic
  )

add_dependencies(clang-interpreter
  clang-headers
  )

  clangCodeGen
  clangDriver
  clangFrontend