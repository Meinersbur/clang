
set(LLVM_LINK_LIBS
  clangDriver
  clangFrontend
  clangRewriteCore
  clangRewriteFrontend
  clangCodeGen
  clangARCMigrate
)

if(CLANG_ENABLE_STATIC_ANALYZER)
  list(APPEND LLVM_LINK_LIBS
    clangStaticAnalyzerFrontend
    clangStaticAnalyzerCheckers
    clangStaticAnalyzerCore
    )
endif()


add_clang_library(clangFrontendTool
  ExecuteCompilerInvocation.cpp
  )

add_dependencies(clangFrontendTool
  ClangDiagnosticCommon
  ClangDiagnosticFrontend
  ClangDriverOptions
  )
