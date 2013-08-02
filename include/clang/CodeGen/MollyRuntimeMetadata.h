#ifndef CLANG_CODEGEN_MOLLYRUNTIMEMETADATA_H
#define CLANG_CODEGEN_MOLLYRUNTIMEMETADATA_H

namespace llvm {
class Module;
class MDNode;
class Function;
class Type;
class Value;
} // namespace llvm


namespace clang {
  namespace CodeGen {

        llvm::Value *makeValueFromType(llvm::Type *ty);
    llvm::Type *extractTypeFromValue(llvm::Value *val);

    /// Captures entry points to Molly's runtime library
    /// TODO: It would be nicer if could be outsourced into a separately compiled library, so other front-ends can use Molly, i.e. The optimizer just expects some symbol names to exist
    /// This is more difficult to do for functions in MollyFieldMetadata that have variable number of indices
    class MollyRuntimeMetadata {
    public:
      llvm::Type *tyCombuf;
      llvm::Type *tyRank;
      llvm::Function *funcCreateSendCombuf;
      llvm::Function *funcCreateRecvCombuf;
      llvm::Function *funcCombufSend;
      llvm::Function *funcCombufRecv;

    public:
      MollyRuntimeMetadata() : tyCombuf(nullptr), tyRank(nullptr), funcCreateSendCombuf(nullptr), funcCreateRecvCombuf(nullptr), funcCombufSend(nullptr), funcCombufRecv(nullptr) {}

      void readMetadata(llvm::Module *llvmModule);
      void readMetadata(llvm::Module *llvmModule, llvm::MDNode *metadata);

      llvm::MDNode *buildMetadata(llvm::Module *llvmModule);
      void addMetadata(llvm::Module *llvmModule);

    }; // class MollyRuntimeMetadata

  } // namespace CodeGen
} // namespace clang
#endif /* CLANG_CODEGEN_MOLLYRUNTIMEMETADATA_H */
