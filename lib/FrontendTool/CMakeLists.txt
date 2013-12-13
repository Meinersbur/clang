set(LLVM_LINK_COMPONENTS
  Option
  Support
  )

add_clang_library(clangFrontendTool
  ExecuteCompilerInvocation.cpp
  )

set(_additional_libs)

if(CLANG_ENABLE_ARCMT)
  list(APPEND _additional_libs 
    clangARCMigrate
    )
endif()

if(CLANG_ENABLE_STATIC_ANALYZER)
  list(APPEND _additional_libs 
    clangStaticAnalyzerFrontend
    clangStaticAnalyzerCheckers
    clangStaticAnalyzerCore
    )
endif()


add_dependencies(clangFrontendTool
  ClangDiagnosticCommon
  ClangDiagnosticFrontend
  ClangDriverOptions
LINK_LIBS
  clangBasic
  clangCodeGen
  clangDriver
  clangFrontend
  clangRewriteCore
  clangRewriteFrontend
  ${_additional_libs}
  )
