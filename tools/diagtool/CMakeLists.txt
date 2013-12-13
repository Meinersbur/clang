set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_executable(diagtool
  diagtool_main.cpp
  DiagTool.cpp
  DiagnosticNames.cpp
  ListWarnings.cpp
  ShowEnabledWarnings.cpp
  TreeView.cpp
  LINK_LIBS clangBasic clangLex clangSema clangFrontend
)

add_dependencies(diagtool
  ClangDiagnosticIndexName
  )

if(UNIX)
  set(CLANGXX_LINK_OR_COPY create_symlink)
else()
  set(CLANGXX_LINK_OR_COPY copy)
endif()
