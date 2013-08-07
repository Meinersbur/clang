#ifndef CLANG_CODEGEN_CGMOLLY_H
#define CLANG_CODEGEN_CGMOLLY_H

#include <llvm/Support/Compiler.h>
#include <llvm/ADT/DenseMap.h>
#include <clang/AST/Type.h>

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
      llvm::DenseMap<const clang::CXXRecordDecl*, FieldTypeMetadata*> fieldsFound;
      clang::FunctionDecl *findGlobalFunction(const char *);
      clang::QualType findMollyType(const char *);

    public:
      CodeGenMolly(CodeGenModule *cgm)  : cgm(cgm) { }
      ~CodeGenMolly();

      void annotateFieldType(const clang::CXXRecordDecl *clangType, llvm::StructType *llvmType);
      void annotateFunction(const clang::FunctionDecl *clangFunc, llvm::Function *llvmFunc);

      static bool EmitMollyBuiltin(clang::CodeGen::RValue &result, clang::CodeGen::CodeGenModule *cgm, clang::CodeGen::CodeGenFunction *cgf, const clang::FunctionDecl *FD, unsigned BuiltinID, const clang::CallExpr *E);

      void EmitMetadata();
    }; // class CodeGenMolly

  } // namespace CodeGen
} // namespace clang
#endif /* CLANG_CODEGEN_CGMOLLY_H */
