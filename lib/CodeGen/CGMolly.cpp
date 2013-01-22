#include "CGMolly.h"

#include "llvm/IR/Value.h"
#include "llvm\IR/DerivedTypes.h"
#include "clang\AST\Attr.h"
#include "clang\AST/Decl.h"
#include "clang\AST/DeclCXX.h"
#include "CodeGenModule.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Type.h"

using namespace clang;
using namespace CodeGen;
using namespace llvm;

using llvm::Value;


void CodeGenMolly::annotateFieldType(const clang::RecordDecl *clangType, llvm::StructType *llvmType) {
  assert(clangType);
  assert(llvmType);

  auto &llvmContext = cgm->getLLVMContext();
  auto &clangContext = cgm->getContext();

  const clang::CXXRecordDecl *cxxRecord = dyn_cast_or_null<clang::CXXRecordDecl>(clangType);
  if (!cxxRecord)
    return; // Not a field

  MollyFieldLengthsAttr *lengthsAttr = cxxRecord->getAttr<MollyFieldLengthsAttr>();
  if (!lengthsAttr)
    return; // Not a field

  auto dims = lengthsAttr->lengths_size();

  // Extract the dimensions
  SmallVector<int, 4> lengths;
  SmallVector<llvm::Value*, 4> lengthsAsVals;
  for (auto it = lengthsAttr->lengths_begin(), end = lengthsAttr->lengths_end(); it!=end; ++it) {
    auto lenExpr = *it;
    llvm::APSInt lenAps;
    if (!lenExpr->EvaluateAsInt(lenAps, clangContext)) {
      assert(!"Not evaluateable to int");
    }
    auto len = lenAps.getLimitedValue();
    lengths.push_back(len);
    lengthsAsVals.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmContext), len));
  }

  // Get the metadata node
  llvm::NamedMDNode *fieldsMetadata = cgm->getModule().getOrInsertNamedMetadata("molly.fields");
 
  // Create metadata info
  llvm::MDNode *lengthsNode = llvm::MDNode::get(llvmContext, lengthsAsVals);

  llvm::Value* metadata[] = {
    llvm::MDString::get(llvmContext, "field"), // Just for testing
    llvm::MDString::get(llvmContext, clangType->getNameAsString()), // Clang type name
    llvm::MDString::get(llvmContext, llvmType->getName()), // LLVM type name
    llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmContext), llvmType->getTypeID()), // LLVM unique typeid
    lengthsNode
   };
  llvm::MDNode *fieldNode = llvm::MDNode::get(llvmContext, metadata);

  // Append the the metadata to the module, so this metadata does not get optimized away as unused
  fieldsMetadata->addOperand(fieldNode);
}
