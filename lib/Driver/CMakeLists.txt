add_clang_library(clangDriver
  Action.cpp
  Arg.cpp
  ArgList.cpp
  CC1AsOptions.cpp
  Compilation.cpp
  Driver.cpp
  DriverOptions.cpp
  Job.cpp
  Option.cpp
  OptTable.cpp
  Phases.cpp
  Tool.cpp
  ToolChain.cpp
  ToolChains.cpp
  WindowsToolChain.cpp
  Tools.cpp
  Types.cpp
  LINK_LIBS clangBasic
  )

add_dependencies(clangDriver
  ClangAttrList
  ClangCC1AsOptions
  ClangDiagnosticCommon
  ClangDiagnosticDriver
  ClangDriverOptions
  )
