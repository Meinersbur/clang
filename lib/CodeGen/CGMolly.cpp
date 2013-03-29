#define DEBUG_TYPE "cgmolly"
#include "CGMolly.h"
#include "clang/CodeGen/MollyFieldMetadata.h"

#include "llvm/IR/Value.h"
#include "llvm\IR/DerivedTypes.h"
#include "clang\AST\Attr.h"
#include "clang\AST/Decl.h"
#include "clang\AST/DeclCXX.h"
#include "CodeGenModule.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Type.h"
#include "CGValue.h"
#include "clang/Basic/Builtins.h"
#include "llvm/IR/Intrinsics.h"
#include "CodeGenModule.h"
#include "llvm/ADT/SmallVector.h"
#include "CodeGenFunction.h"
#include "clang/AST/Decl.h"

using namespace clang;
using namespace clang::CodeGen;
using namespace llvm;


llvm::MDNode *FieldTypeMetadata::buildMetadata() {
  auto &llvmContext = llvmType->getContext();

  // Create metadata info
  llvm::SmallVector<llvm::Value*, 4> lengthsAsValue;
  for (auto it = dimLengths.begin(), end = dimLengths.end(); it!=end; ++it) {
    auto length = *it;
    lengthsAsValue.push_back(ConstantInt::get(llvm::Type::getInt32Ty(llvmContext), length));
  }

  llvm::MDNode *lengthsNode = llvm::MDNode::get(llvmContext, lengthsAsValue);

  llvm::Value* metadata[] = {
    /*[ 0]*/llvm::MDString::get(llvmContext, "field"), // Just for testing
    /*[ 1]*/llvm::MDString::get(llvmContext, clangDecl->getNameAsString()), // Clang type name
    /*[ 2]*/llvm::MDString::get(llvmContext, llvmType->getName()), // LLVM type name
    /*[ 3]*/llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmContext), llvmType->getTypeID()), // LLVM unique typeid
    /*[ 4]*/lengthsNode, // Size of each dimension
    /*[ 5]*/funcGetLocal,
    /*[ 6]*/funcSetLocal,
    /*[ 7]*/funcGetBroadcast,
    /*[ 8]*/funcSetBroadcast,
    /*[ 9]*/funcGetMaster,
    /*[10]*/funcSetMaster
  };
  llvm::MDNode *fieldNode = llvm::MDNode::get(llvmContext, metadata);
  return fieldNode;
}


void FieldTypeMetadata::readMetadata(llvm::Module *llvmModule, llvm::MDNode *metadata) {
  auto magic = dyn_cast<MDString>(metadata->getOperand(0));
  assert(magic->getString() == "field");

  auto clangNameMD = dyn_cast<MDString>(metadata->getOperand(1));
  auto clangName = clangNameMD->getString();

  auto llvmNameMD = dyn_cast<MDString>(metadata->getOperand(2));
  auto llvmName = llvmNameMD->getString();
  llvmType = llvmModule->getTypeByName(llvmName);

  auto typeidMD = dyn_cast<llvm::ConstantInt>(metadata->getOperand(3));
  auto tid = typeidMD->getLimitedValue();

  auto lengthsMD = dyn_cast<llvm::MDNode>(metadata->getOperand(4));
  auto dims = lengthsMD->getNumOperands();
  dimLengths.clear();
  for (unsigned i = 0; i < dims; i+=1) {
    auto lenMD = dyn_cast<llvm::ConstantInt>(lengthsMD->getOperand(i));
    auto len = lenMD->getLimitedValue();
    dimLengths.push_back(len);
  }
  funcGetLocal = NULL; funcSetLocal = NULL;
  funcGetBroadcast = cast<llvm::Function>(metadata->getOperand(7));
  funcSetBroadcast = cast<llvm::Function>(metadata->getOperand(8));
  funcGetMaster = cast<llvm::Function>(metadata->getOperand(9));
  funcSetMaster = cast<llvm::Function>(metadata->getOperand(10));
}



CodeGenMolly::~CodeGenMolly() {
  for (auto it = fieldsFound.begin(), end = fieldsFound.end(); it!=end; ++it) {
    delete it->second;
  }
}


static NamedDecl *findMember(CodeGenModule *cgm, const clang::CXXRecordDecl *cxxRecord, const char *funcname) {
  auto astContext = &cgm->getContext();
  auto ident = &astContext->Idents.get(funcname);
  auto declname = astContext->DeclarationNames.getIdentifier(ident);
  auto lures = cxxRecord->lookup(declname); // Does this also lookup in base classes?
  if (lures.empty())
    return NULL;
  assert(lures.size() == 1);
  auto named = lures[0];
  return named;
}


static Function *findTemplateMethod(CodeGenModule *cgm, const clang::CXXRecordDecl *cxxRecord, const char *funcname) {
#if 0
  auto astContext = &cgm->getContext();
  auto ident = &astContext->Idents.get(funcname);
  auto declname = astContext->DeclarationNames.getIdentifier(ident);
  auto lures = cxxRecord->lookup(declname); // Does this also lookup in base classes?
  if (lures.empty())
    return NULL;
  assert(lures.size() == 1);
  auto named = lures[0];


  if (auto tmpl = dyn_cast<FunctionTemplateDecl>(named)) {
    auto tmplfunc = cast<CXXMethodDecl>(tmpl->getTemplatedDecl());

    tmpl->findSpecialization( 

      //cgm->Sema->DeduceTemplateArguments
      // Sema->MarkFunctionReferenced
      //PendingLocalImplicitInstantiations
      //PendingInstantiations
      //InstantiateFunctionDefinition
      // PerformPendingInstantiations
  } 




  // This part is actually more complicated to get a function type for methods; for our special case, this should be enough
  auto FInfo = &cgm->getTypes().arrangeCXXMethodDeclaration(clangreffunc);
  auto Ty = cgm->getTypes().GetFunctionType(*FInfo);

  auto refFunc = cgm->GetAddrOfFunction(clangreffunc, Ty);
  return cast<Function>(refFunc);
#endif
}


static Function *clangFunction2llvmFunction(CodeGenModule *cgm, FunctionDecl *clangFunc) {
  auto method = cast<CXXMethodDecl>(clangFunc);
  // This part is actually more complicated to get a function type for methods; for our special case, this should be enough
  auto FInfo = &cgm->getTypes().arrangeCXXMethodDeclaration(method);
  auto Ty = cgm->getTypes().GetFunctionType(*FInfo);

  auto llvmFunc = cgm->GetAddrOfFunction(clangFunc, Ty);
  return cast<Function>(llvmFunc);
}


static Function *findMethod(CodeGenModule *cgm, const clang::CXXRecordDecl *cxxRecord, const char *funcname) {
  auto astContext = &cgm->getContext();
  auto ident = &astContext->Idents.get(funcname);
  auto declname = astContext->DeclarationNames.getIdentifier(ident);
  auto lures = cxxRecord->lookup(declname); // Does this also lookup in base classes?
  if (lures.empty())
    return NULL;
  assert(lures.size() == 1);
  auto named = lures[0];
  auto clangreffunc = cast<CXXMethodDecl>(named);

  // This part is actually more complicated to get a function type for methods; for our special case, this should be enough
  auto FInfo = &cgm->getTypes().arrangeCXXMethodDeclaration(clangreffunc);
  auto Ty = cgm->getTypes().GetFunctionType(*FInfo);

  auto refFunc = cgm->GetAddrOfFunction(clangreffunc, Ty);
  return cast<Function>(refFunc);
}


static void addAddtribute(LLVMContext *llvmContext, Function *func, StringRef attr) {
  llvm::AttrBuilder ab;
  ab.addAttribute(attr);
  func->addAttributes(llvm::AttributeSet::FunctionIndex, llvm::AttributeSet::get(*llvmContext, llvm::AttributeSet::FunctionIndex, ab));
}


void CodeGenMolly::annotateFieldType(const clang::CXXRecordDecl *clangType, llvm::StructType *llvmType) {
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

  auto &fieldType  = fieldsFound[clangType];
  if (fieldType)
    return;

  // Extract the dimensions
  SmallVector<int, 4> lengths;
  //SmallVector<llvm::Value*, 4> lengthsAsVals;
  for (auto it = lengthsAttr->lengths_begin(), end = lengthsAttr->lengths_end(); it!=end; ++it) {
    auto lenExpr = *it;
    llvm::APSInt lenAps;
    if (auto parmpack = dyn_cast<SubstNonTypeTemplateParmPackExpr>(lenExpr)) {
      auto argpack = parmpack->getArgumentPack();
      for (auto it = argpack.pack_begin(), end = argpack.pack_end(); it!=end; ++it) {
        auto packelt = &*it;
        auto val = packelt->getAsIntegral();
        auto len = val.getLimitedValue();
        lengths.push_back(len);
        //lengthsAsVals.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmContext), len));
      }
    } else if (lenExpr->EvaluateAsInt(lenAps, clangContext)) {
      auto len = lenAps.getLimitedValue();
      lengths.push_back(len);
      //lengthsAsVals.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmContext), len));
    } else
      assert(!"Not evaluateable to int");
  }
  auto dims = lengths.size();

  fieldType = new FieldTypeMetadata(clangType, llvmType, lengths);
  return;


  // Get the metadata node
  llvm::NamedMDNode *fieldsMetadata = cgm->getModule().getOrInsertNamedMetadata("molly.fields");

  // Create metadata info
  //llvm::MDNode *lengthsNode = llvm::MDNode::get(llvmContext, lengthsAsVals);

  //CXXBasePaths paths;
  //paths.setOrigin(cxxRecord);

#if 0
  auto ptrMember = findMember(cgm, cxxRecord, "ptr");
  auto ptrTempl = cast<FunctionTemplateDecl>(ptrMember);
  void *insertpos;
  SmallVector<TemplateArgument, 4>  ptrParamList;
  for (auto i=dims-dims;i<dims;i+=1) {
    TemplateArgument intparam(clangContext.IntTy);
    ptrParamList.push_back(intparam);
  }
  TemplateArgument ptrArgs(ptrParamList.data(), ptrParamList.size());
  auto ptrSpez = ptrTempl->findSpecialization(&ptrArgs, lengths.size(),insertpos);
  assert(ptrSpez && "Member not generated");
  auto ptrFunc = clangFunction2llvmFunction(cgm, ptrSpez);
  if (ptrFunc) {
    addAddtribute(&llvmContext, ptrFunc, "molly_ptr");
  }
#endif
#if 0
  auto refFunc = findMethod(cgm, cxxRecord, "ref"); 
  if (refFunc) { 
    addAddtribute(&llvmContext, refFunc, "molly_ref");
    assert(refFunc->getAttributes().hasAttribute(AttributeSet::FunctionIndex, "molly_ref"));
  }

  auto ptrFunc = findMethod(cgm, cxxRecord, "ptr"); 
  if (ptrFunc) { 
    addAddtribute(&llvmContext, ptrFunc, "molly_ptr");
  }


  //refFunc->hasExternalLinkage();
  //refFunc->setLinkage(GlobalValue::ExternalLinkage); // Avoid not being emitted
  auto islocalFunc = findMethod(cgm, cxxRecord, "isLocal"); 
  //islocalFunc->setLinkage(GlobalValue::ExternalLinkage);
  if (islocalFunc) 
    addAddtribute(&llvmContext, islocalFunc, "molly_islocal");

  auto getrankofFunc = findMethod(cgm, cxxRecord, "getRankOf"); 
  if (getrankofFunc) 
    addAddtribute(&llvmContext, getrankofFunc, "molly_getrankof");
#endif

  llvm::Value* metadata[] = {
    llvm::MDString::get(llvmContext, "field"), // Just for testing
    llvm::MDString::get(llvmContext, clangType->getNameAsString()), // Clang type name
    llvm::MDString::get(llvmContext, llvmType->getName()), // LLVM type name
    llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmContext), llvmType->getTypeID()), // LLVM unique typeid
    NULL,
    NULL,
    NULL
  };
  llvm::MDNode *fieldNode = llvm::MDNode::get(llvmContext, metadata);
  //assert(fieldDeclToMetadata.find(clangType) == fieldDeclToMetadata.end());
  //fieldDeclToMetadata[clangType] = fieldNode;

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

  if (clangFunc->hasAttr<MollyInlineAttr>()) {
    ab.addAttribute("molly_inline");
    DEBUG(dbgs() << "    Inlinable Func:" << clangFunc->getNameAsString() << "\n");
  } else {
    DEBUG(dbgs() << "NOT Inlinable Func:" << clangFunc->getNameAsString() << "\n");
  }

  auto clangFieldDecl = dyn_cast<CXXRecordDecl>(clangFunc->getParent());
  if (clangFieldDecl) {

    if (fieldsFound.find(clangFieldDecl)==fieldsFound.end())
      return; // Not a registered field; because annotateFieldType has been called before any of the classes members are created, annotateFieldType abviously decided that the class is not a field
    auto field = fieldsFound[clangFieldDecl];

    if (clangFunc->hasAttr<MollyGetterFuncAttr>()) {
      ab.addAttribute("molly_get");
    }

    if (clangFunc->hasAttr<MollyRefFuncAttr>()) {
      ab.addAttribute("molly_ptr");
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

    if (clangFunc->hasAttr<MollyGetRankOfFuncAttr>()) {
      ab.addAttribute("molly_getrankof");
    }

    if (clangFunc->hasAttr<MollyGetBroadcastAttr>()) {
      ab.addAttribute("molly_get_broadcast");
      assert(!field->funcGetBroadcast && "Just one function implementation for Molly specials"); 
      field->funcGetBroadcast = llvmFunc; //TODO: Check function signature
    }

    if (clangFunc->hasAttr<MollySetBroadcastAttr>()) {
      ab.addAttribute("molly_set_broadcast");
      assert(!field->funcSetBroadcast && "Just one function implementation for Molly specials"); 
      field->funcSetBroadcast = llvmFunc; //TODO: Check function signature
    }

    if (clangFunc->hasAttr<MollyGetMasterAttr>()) {
      ab.addAttribute("molly_get_master");
      assert(!field->funcGetMaster && "Just one function implementation for Molly specials"); 
      field->funcGetMaster = llvmFunc; //TODO: Check function signature
    }

    if (clangFunc->hasAttr<MollySetMasterAttr>()) {
      ab.addAttribute("molly_set_master");
      assert(!field->funcSetMaster && "Just one function implementation for Molly specials"); 
      field->funcSetMaster = llvmFunc; //TODO: Check function signature
    }
  }

  llvmFunc->addAttributes(llvm::AttributeSet::FunctionIndex, llvm::AttributeSet::get(llvmContext, llvm::AttributeSet::FunctionIndex, ab));
}


void CodeGenMolly::EmitMetadata() {
  llvm::NamedMDNode *fieldsMetadata = cgm->getModule().getOrInsertNamedMetadata("molly.fields");

  for (auto it = fieldsFound.begin(), end = fieldsFound.end(); it!=end; ++it) {
    auto field = it->second;
    if (!field)
      continue;

    fieldsMetadata->addOperand(field->buildMetadata());
  }
}



bool CodeGenMolly::EmitMollyBuiltin(clang::CodeGen::RValue &result, clang::CodeGen::CodeGenModule *cgm, clang::CodeGen::CodeGenFunction *cgf, const clang::FunctionDecl *FD, unsigned BuiltinID, const clang::CallExpr *E) {
  Intrinsic::ID intrincId;
  switch (BuiltinID) {
  case Builtin::BI__builtin_molly_ptr:
    intrincId = Intrinsic::molly_ptr;
    break;
  case Builtin::BI__builtin_molly_get:
    intrincId = Intrinsic::molly_get;
    break;
  case Builtin::BI__builtin_molly_set:
    intrincId = Intrinsic::molly_set;
    break;
  case Builtin::BI__builtin_molly_islocal:
    intrincId = Intrinsic::molly_islocal;
    break;
  case Builtin::BI__builtin_molly_rankof:
    intrincId = Intrinsic::molly_rankof;
    break;
  default:
    return false;
  }

  auto &llvmContext = cgm->getLLVMContext();
  auto &clangContext = cgm->getContext();

  auto nArgs = E->getNumArgs();
  auto builder = &cgf->Builder;
  auto nDims = (nArgs - 1);

  // Query element type
  auto ptrToSelf = E->getArg(0);
  auto ptrToSelfType = ptrToSelf->getType();
  auto structTy = cast<CXXRecordDecl>(ptrToSelfType->getPointeeType()->getAsCXXRecordDecl());
  auto eltTypedef = cast<TypedefDecl>(findMember(cgm, structTy, "ElementType"));
  auto eltType = clangContext.getTypeDeclType(eltTypedef);
  auto llvmEltType = cgf->ConvertType(eltTypedef);
  auto llvmPtrToEltType = cgf->ConvertType(clangContext.getPointerType(eltType));

  Value *thisArg = cgf->EmitScalarExpr(ptrToSelf);
  SmallVector<llvm::Type*, 6> argtypes;
  SmallVector<Value*, 4> args;
  args.push_back(thisArg);
  switch (BuiltinID) {
  case Builtin::BI__builtin_molly_ptr:
    argtypes.push_back(llvmPtrToEltType);
    argtypes.push_back(thisArg->getType()); 
    //args.push_back(thisArg);
    break;
  case Builtin::BI__builtin_molly_get:
    argtypes.push_back(llvmEltType);
    argtypes.push_back(thisArg->getType()); 
    //args.push_back(thisArg);
    break;
  case Builtin::BI__builtin_molly_set:  {
    // Return type
    argtypes.push_back(llvm::Type::getVoidTy(llvmContext)); 

    // Type of "this"
    argtypes.push_back(thisArg->getType()); 
    //args.push_back(thisArg);

    // Value argument
    nDims = (nArgs - 2);
    auto valArg = cgf->EmitScalarExpr(E->getArg(nArgs-1));
    argtypes.push_back(llvmEltType); 
    args.push_back(valArg);
                                        } break;
  case Builtin::BI__builtin_molly_islocal:
    argtypes.push_back(llvm::Type::getInt1Ty(llvmContext));
    argtypes.push_back(thisArg->getType()); 
    //args.push_back(thisArg);
    break;
  case Builtin::BI__builtin_molly_rankof:
    argtypes.push_back(llvm::Type::getInt32Ty(llvmContext));
    argtypes.push_back(thisArg->getType()); 
    //args.push_back(thisArg);
    break;
  default:
    return false;
  }

  //argtypes.push_back(llvm::Type::getVoidTy(llvmContext)); // Return type
  //llvm::Type *&rtnTy = argtypes[0];
  //argtypes.push_back(thisArg->getType()); // type of "this"

  // Coordinates
  for (auto d = nArgs-nDims;d<nArgs;d+=1) {
    argtypes.push_back(IntegerType::getInt32Ty(llvmContext)); // Coordinates

    Value *dimCoordArg = cgf->EmitScalarExpr(E->getArg(d));
    args.push_back(dimCoordArg);
  }

  Function *func = cgm->getIntrinsic(intrincId, argtypes);

  auto callInstr = builder->CreateCall(func, args, "mollycall");
  result = RValue::get(callInstr);
  return true;
}


