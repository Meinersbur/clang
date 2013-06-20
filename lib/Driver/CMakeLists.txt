set(LLVM_LINK_LIBS
  clangBasic
  LLVMOption
)

add_clang_library(clangDriver
  Action.cpp
  CC1AsOptions.cpp
  Compilation.cpp
  Driver.cpp
  DriverOptions.cpp
  Job.cpp
  Phases.cpp
  Tool.cpp
  ToolChain.cpp
  ToolChains.cpp
  WindowsToolChain.cpp
  Tools.cpp
  Types.cpp
  )

add_dependencies(clangDriver
  ClangAttrList
  ClangCC1AsOptions
  ClangDiagnosticCommon
  ClangDiagnosticDriver
  ClangDriverOptions
  )
