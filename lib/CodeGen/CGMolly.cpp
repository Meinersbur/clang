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


static Function *findMethod(CodeGenModule *cgm, const clang::CXXRecordDecl *cxxRecord, const char *funcname) {
  auto astContext = &cgm->getContext();
  auto ident = &astContext->Idents.get(funcname);
  auto &declname = astContext->DeclarationNames.getIdentifier(ident);
  auto lures = cxxRecord->lookup(declname); // Does this also lookup in base classes?
  assert(lures.size() == 1);
  auto named = lures[0];
  auto clangreffunc = cast<CXXMethodDecl>(named);

  // This part is actually more complicated to get a function type for methods; for our special case, this should be enough
  auto FInfo = &cgm->getTypes().arrangeCXXMethodDeclaration(clangreffunc);
  auto Ty = cgm->getTypes().GetFunctionType(*FInfo);

  auto refFunc = cgm->GetAddrOfFunction(clangreffunc, Ty);
  return cast<Function>(refFunc);

#if 0
  //  CXXBasePaths paths; paths.setOrigin(cxxRecord);
  bool success = cxxRecord->lookupInBases(&CXXRecordDecl::FindOrdinaryMember, "ref", paths);
  assert(std::count_if(paths.begin(), paths.end(), [](const CXXBasePath &)->bool{return true;}) == 1);
  auto reffuncdecls = paths.front();
  assert(std::count_if(reffuncdecls.Decls.begin(), reffuncdecls.Decls.end(), []( NamedDecl *)->bool{return true;}) == 1);
  auto clangreffunc = cast<FunctionDecl>(reffuncdecls.Decls.front());
  // cgm->GetOrCreateLLVMFunction(cgm->ma
  auto refFunc = cgm->GetAddrOfFunction(clangreffunc);
#endif
}


static void addAddtribute(LLVMContext *llvmContext, Function *func, StringRef attr) {
  llvm::AttrBuilder ab;
  ab.addAttribute(attr);
  func->addAttributes(llvm::AttributeSet::FunctionIndex, llvm::AttributeSet::get(*llvmContext, llvm::AttributeSet::FunctionIndex, ab));
}


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

  //CXXBasePaths paths;
  //paths.setOrigin(cxxRecord);


  auto refFunc = findMethod(cgm, cxxRecord, "ref"); 
  if (refFunc) 
    addAddtribute(&llvmContext, refFunc, "molly_ref");
  assert(refFunc->getAttributes().hasAttribute(AttributeSet::FunctionIndex, "molly_ref"));


  //refFunc->hasExternalLinkage();
  //refFunc->setLinkage(GlobalValue::ExternalLinkage); // Avoid not being emitted
  auto islocalFunc = findMethod(cgm, cxxRecord, "isLocal"); 
  //islocalFunc->setLinkage(GlobalValue::ExternalLinkage);
  if (islocalFunc) 
    addAddtribute(&llvmContext, refFunc, "molly_islocal");

  llvm::Value* metadata[] = {
    llvm::MDString::get(llvmContext, "field"), // Just for testing
    llvm::MDString::get(llvmContext, clangType->getNameAsString()), // Clang type name
    llvm::MDString::get(llvmContext, llvmType->getName()), // LLVM type name
    llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmContext), llvmType->getTypeID()), // LLVM unique typeid
    lengthsNode,
    refFunc,
    islocalFunc
  };
  llvm::MDNode *fieldNode = llvm::MDNode::get(llvmContext, metadata);

  // Append the the metadata to the module, so this metadata does not get optimized away as unused
  fieldsMetadata->addOperand(fieldNode);
}


void CodeGenMolly::annotateFunction(const clang::FunctionDecl *clangFunc, llvm::Function *llvmFunc) {
  assert(clangFunc);
  assert(llvmFunc);

  auto &llvmContext = cgm->getLLVMContext();
  auto &clangContext = cgm->getContext();

  // TODO: mark special attributes (especially readOnly, mayWriteToMemory, mayThrow)
  llvm::AttrBuilder ab;

  if (clangFunc->hasAttr<MollyGetterFuncAttr>()) {
    ab.addAttribute("molly_get");
  }

  if (clangFunc->hasAttr<MollyRefFuncAttr>()) {
    ab.addAttribute("molly_ref");
  }

  if (clangFunc->hasAttr<MollySetterFuncAttr>()) {
    ab.addAttribute("molly_set");
  }

  if (clangFunc->hasAttr<MollyLengthFuncAttr>()) {
    ab.addAttribute("molly_length");
  }

  if (clangFunc->hasAttr<MollyFieldmemberAttr>()) {
    ab.addAttribute("molly_fieldmember");
  }

  llvmFunc->addAttributes(llvm::AttributeSet::FunctionIndex, llvm::AttributeSet::get(llvmContext, llvm::AttributeSet::FunctionIndex, ab));

  if (clangFunc->hasAttr<MollyRefFuncAttr>()) {
    assert(llvmFunc->getAttributes().hasAttribute(AttributeSet::FunctionIndex, "molly_ref"));
  }
}
