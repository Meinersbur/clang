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
  class Value;
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

    llvm::Value *makeValueFromType(llvm::Type *ty, llvm::Module *module);
    llvm::Type *extractTypeFromValue(llvm::Value *val);

    //TODO: Move this class to LLVM to allow other frontends generate metadata for Molly and remove dependency to Clang
    class FieldTypeMetadata {
    private:
      llvm::MDNode *node;

    public:
      FieldTypeMetadata();
      FieldTypeMetadata(const clang::CXXRecordDecl *clangDecl, llvm::StructType *llvmType, llvm::Type *llvmEltType, uint64_t eltSize, llvm::ArrayRef<int> dims);

      std::string clangTypeName;
      const clang::CXXRecordDecl *clangDecl;
      llvm::StructType *llvmType;

      // Future versions may have different element types depending on coordinate (e.g. at one level there is a struct)
      // In this case, we'd add a isl_set that defines for which elements it is valid
      llvm::Type *llvmEltType;
      // In principle, the element size should be enough, but maybe we can do some more optimizations if we know the exact type. It also saves us from too much casting
      uint64_t eltSize; // In bytes

      llvm::SmallVector<int, 4> dimLengths;

      llvm::Function *funcGetLocal;
      llvm::Function *funcSetLocal;
      llvm::Function *funcGetBroadcast;
      llvm::Function *funcSetBroadcast;
      llvm::Function *funcGetMaster;
      llvm::Function *funcSetMaster;
      llvm::Function *funcIslocal;
      llvm::Function *funcPtrLocal;

      llvm::MDNode *buildMetadata(llvm::Module *llvmModule);
      void readMetadata(llvm::Module *llvmModule, llvm::MDNode *metadata);
    }; // class FieldTypeMetadata


    class FieldVarMetadata {
    private:
      //llvm::MDNode *node;


    public:
       FieldVarMetadata()  {}

       std::string islstr;
       unsigned clusterdims;

       llvm::MDNode *buildMetadata(llvm::Module *llvmModule);
       void readMetadata(llvm::Module *llvmModule, llvm::MDNode *metadata);

    }; // class FieldVarMetadata

  } // namespace CodeGen
} // namespace clang
#endif /* CLANG_CODEGEN_MOLLYFIELDMETADATA_H */
