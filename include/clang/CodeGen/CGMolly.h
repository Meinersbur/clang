#ifndef CLANG_CODEGEN_CGMOLLY_H
#define CLANG_CODEGEN_CGMOLLY_H

#include "llvm/Support/Compiler.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ArrayRef.h"
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
  }
} // namespace clang


namespace clang {
  namespace CodeGen {
      class FieldTypeMetadata {
    public:
      FieldTypeMetadata() {
        this->clangDecl = NULL;
      }

      FieldTypeMetadata(const clang::CXXRecordDecl *clangDecl, llvm::StructType *llvmType, llvm::ArrayRef<int> dims) {
        assert(clangDecl);
        assert(llvmType);
        this->clangDecl = clangDecl;
        this->llvmType = llvmType;
        dimLengths.append(dims.begin(), dims.end());
      }

      const clang::CXXRecordDecl *clangDecl;
      llvm::StructType *llvmType;
      llvm::SmallVector<int, 4> dimLengths;

      llvm::Function *funcGetBroadcast;
      llvm::Function *funcSetBroadcast;
      llvm::Function *funcGetMaster;
      llvm::Function *funcSetMaster;

      llvm::MDNode *buildMetadata() ;

      void readMetadata(llvm::Module *llvmModule, llvm::MDNode *metadata);
    }; // class FieldTypeMetadata


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
