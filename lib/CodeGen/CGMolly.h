#ifndef CLANG_CODEGEN_CGMOLLY_H
#define CLANG_CODEGEN_CGMOLLY_H

#include <llvm/Support/Compiler.h>
#include <llvm/ADT/DenseMap.h>
#include <clang/AST/Type.h>
#include <vector>

namespace llvm {
  class StructType;
  class Function;
  class MDNode;
  class Module;
  class CallInst;
} // namespace llvm

namespace clang {
  class Decl;
  class VarDecl;
  class RecordDecl;
  class CXXRecordDecl;
  class FunctionDecl;
  class CallExpr;
  class GlobalDecl;
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
      llvm::DenseMap<const clang::CXXRecordDecl*, FieldTypeMetadata*> fieldsFound;
      clang::FunctionDecl *findGlobalFunction(const char *);
      clang::VarDecl *findGlobalVariable(const char *name);
      clang::QualType findMollyType(const char *);

      std::vector<std::pair<llvm::CallInst*, const clang::CXXRecordDecl*>> pendingMetadata;

    public:
      CodeGenMolly(CodeGenModule *cgm)  : cgm(cgm) { }
      ~CodeGenMolly();

      void annotateFieldType(const clang::CXXRecordDecl *clangType, llvm::StructType *llvmType);
      void annotateFunction(const clang::FunctionDecl *clangFunc, llvm::Function *llvmFunc);

      bool EmitMollyBuiltin(clang::CodeGen::RValue &result, clang::CodeGen::CodeGenModule *cgm, clang::CodeGen::CodeGenFunction *cgf, const clang::FunctionDecl *FD, unsigned BuiltinID, const clang::CallExpr *E);

      void EmitMetadata();
    }; // class CodeGenMolly

  } // namespace CodeGen
} // namespace clang
#endif /* CLANG_CODEGEN_CGMOLLY_H */
