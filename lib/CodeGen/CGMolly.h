#ifndef CLANG_CODEGEN_CGMOLLY_H
#define CLANG_CODEGEN_CGMOLLY_H

#include "llvm/Support/Compiler.h"
//#include "CGBuilder.h" // typedef CGBuilderTy

namespace llvm {
  class StructType;
  class Function;
}

namespace clang {
  class RecordDecl;
  class FunctionDecl;
  class CallExpr;
  namespace CodeGen {
    class CodeGenModule;
    class CodeGenFunction;
    class RValue;
  }
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

      static bool EmitMollyBuiltin(clang::CodeGen::RValue &result, clang::CodeGen::CodeGenModule *cgm, clang::CodeGen::CodeGenFunction *cgf, const clang::FunctionDecl *FD, unsigned BuiltinID, const clang::CallExpr *E);
    }; // class CodeGenMolly
  } // namespace CodeGen
} // namespace clang
#endif /* CLANG_CODEGEN_CGMOLLY_H */