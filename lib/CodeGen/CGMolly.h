#ifndef CLANG_CODEGEN_CGMOLLY_H
#define CLANG_CODEGEN_CGMOLLY_H

#include "llvm/Support/Compiler.h"

namespace llvm {
  class StructType;
  class Function;
}

namespace clang {
  class RecordDecl;
  class FunctionDecl;
}


namespace clang {
  namespace CodeGen {
    class CodeGenModule;

    class CodeGenMolly {
    private:
      CodeGenMolly(const CodeGenMolly &) LLVM_DELETED_FUNCTION;
      void operator=(const CodeGenMolly &) LLVM_DELETED_FUNCTION;

      CodeGenModule *cgm;

    public:
      CodeGenMolly(CodeGenModule *cgm) {
        this->cgm = cgm;
      }

      void annotateFieldType(const clang::RecordDecl *clangType, llvm::StructType *llvmType);
      void annotateFunction(const clang::FunctionDecl *clangFunc, llvm::Function *llvmFunc);
    }; // class CodeGenMolly
  } // namespace CodeGen
} // namespace clang
#endif /* CLANG_CODEGEN_CGMOLLY_H */