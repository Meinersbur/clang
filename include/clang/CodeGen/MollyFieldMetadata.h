#ifndef CLANG_CODEGEN_MOLLYFIELDMETADATA_H
#define CLANG_CODEGEN_MOLLYFIELDMETADATA_H

//#include "llvm/Support/Compiler.h"
//#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ArrayRef.h"
//#include "CGBuilder.h" // typedef CGBuilderTy

namespace llvm {
  class StructType;
  class Function;
  class MDNode;
  class Module;
  class Type;
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
  } // namespace CodeGen
} // namespace clang


namespace clang {
  namespace CodeGen {

    //TODO: Move this class to LLVM to allow other frontends generate metadata for Molly and remove dependency to Clang
    class FieldTypeMetadata {
    public:
      FieldTypeMetadata();
      FieldTypeMetadata(const clang::CXXRecordDecl *clangDecl, llvm::StructType *llvmType, llvm::Type *llvmEltType, llvm::ArrayRef<int> dims);

      const clang::CXXRecordDecl *clangDecl;
      llvm::StructType *llvmType;
      llvm::Type *llvmEltType;
      llvm::SmallVector<int, 4> dimLengths;

      llvm::Function *funcGetLocal;
      llvm::Function *funcSetLocal;
      llvm::Function *funcGetBroadcast;
      llvm::Function *funcSetBroadcast;
      llvm::Function *funcGetMaster;
      llvm::Function *funcSetMaster;
      llvm::Function *funcIslocal;

      llvm::MDNode *buildMetadata();
      void readMetadata(llvm::Module *llvmModule, llvm::MDNode *metadata);
    }; // class FieldTypeMetadata

  } // namespace CodeGen
} // namespace clang
#endif /* CLANG_CODEGEN_MOLLYFIELDMETADATA_H */
