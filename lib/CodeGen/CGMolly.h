#ifndef CLANG_CODEGEN_CGMOLLY_H
#define CLANG_CODEGEN_CGMOLLY_H

#include "llvm/Support/Compiler.h"
#include "llvm/ADT/DenseMap.h"
//#include "llvm/ADT/SmallVector.h"
//#include "llvm/ADT/ArrayRef.h"
//#include "CGBuilder.h" // typedef CGBuilderTy

namespace llvm {
  class StructType;
  class Function;
  class MDNode;
  class Module;
} // namespace llvm

namespace clang {
  class Decl;
  class RecordDecl;
  class CXXRecordDecl;
  class FunctionDecl;
  class CallExpr;
  namespace CodeGen {
    class CodeGenModule;
    class CodeGenFunction;
    class RValue;
    class FieldTypeMetadata;
  }
} // namespace clang


namespace clang {
  namespace CodeGen {
    class CodeGenMolly {
    private:
      CodeGenMolly(const CodeGenMolly &) LLVM_DELETED_FUNCTION;
      void operator=(const CodeGenMolly &) LLVM_DELETED_FUNCTION;

      CodeGenModule *cgm;
      //llvm::DenseMap<const clang::RecordDecl*, llvm::MDNode*> fieldDeclToMetadata;
      llvm::DenseMap<const clang::CXXRecordDecl*, FieldTypeMetadata*> fieldsFound;

    public:
      CodeGenMolly(CodeGenModule *cgm) {
        this->cgm = cgm;
      }
      ~CodeGenMolly();

      void annotateFieldType(const clang::CXXRecordDecl *clangType, llvm::StructType *llvmType);
      void annotateFunction(const clang::FunctionDecl *clangFunc, llvm::Function *llvmFunc);

      static bool EmitMollyBuiltin(clang::CodeGen::RValue &result, clang::CodeGen::CodeGenModule *cgm, clang::CodeGen::CodeGenFunction *cgf, const clang::FunctionDecl *FD, unsigned BuiltinID, const clang::CallExpr *E);

      void EmitMetadata();
    }; // class CodeGenMolly
  } // namespace CodeGen
} // namespace clang
#endif /* CLANG_CODEGEN_CGMOLLY_H */
