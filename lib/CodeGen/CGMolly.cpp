#define DEBUG_TYPE "cgmolly"
#include "CGMolly.h"
#include "clang/CodeGen/MollyFieldMetadata.h"

#include "llvm/IR/Value.h"
#include "llvm/IR/DerivedTypes.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
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
#include "clang/CodeGen/MollyRuntimeMetadata.h"

using namespace clang;
using namespace clang::CodeGen;
using namespace llvm;


void MollyRuntimeMetadata::readMetadata(llvm::Module *llvmModule) {
  auto metadata = llvmModule->getNamedMetadata("molly.runtime");
  assert(metadata->getNumOperands()==1);
  readMetadata(llvmModule, metadata->getOperand(0));
}


void MollyRuntimeMetadata::readMetadata(llvm::Module *llvmModule, llvm::MDNode *metadata) {
  this->tyCombufSend = extractTypeFromValue(metadata->getOperand(0));
  this->tyCombufRecv = extractTypeFromValue(metadata->getOperand(1));
  this->tyRank = extractTypeFromValue(metadata->getOperand(2));
  this->funcCreateSendCombuf = cast<Function>(metadata->getOperand(3));
  this->funcCreateRecvCombuf = cast<Function>(metadata->getOperand(4));
  this->funcCombufSend = cast<Function>(metadata->getOperand(5));
  this->funcCombufRecv = cast<Function>(metadata->getOperand(6));
  assert( metadata->getNumOperands()==7);
}


llvm::MDNode *MollyRuntimeMetadata::buildMetadata(llvm::Module *llvmModule) {
  auto &llvmContext = llvmModule->getContext();

  llvm::Value* metadata[] = {
    /*[ 0]*/makeValueFromType(tyCombufSend),
    /*[ 1]*/makeValueFromType(tyCombufRecv),
    /*[ 2]*/makeValueFromType(tyRank),
    /*[ 3]*/funcCreateSendCombuf,
    /*[ 4]*/funcCreateRecvCombuf,
    /*[ 5]*/funcCombufSend,
    /*[ 6]*/funcCombufRecv
  };
  llvm::MDNode *metadataNode = llvm::MDNode::get(llvmContext, metadata);
  return metadataNode;
}


void MollyRuntimeMetadata::addMetadata(llvm::Module *llvmModule){
  auto metadata = llvmModule-> getOrInsertNamedMetadata("molly.runtime");
  assert(metadata->getNumOperands()==0);
  metadata->addOperand(buildMetadata(llvmModule));
}


FieldTypeMetadata::FieldTypeMetadata() {
  this->clangDecl = NULL;
  this->llvmType = NULL;
  this->llvmEltType = NULL;
  this->eltSize = -1;

  this->funcGetLocal = NULL;
  this->funcSetLocal = NULL;
  this->funcGetBroadcast = NULL;
  this->funcSetBroadcast = NULL;
  this->funcGetMaster = NULL;
  this->funcSetMaster = NULL;
  this->funcIslocal = NULL;
  this->funcPtrLocal = nullptr;
}


FieldTypeMetadata::FieldTypeMetadata(const clang::CXXRecordDecl *clangDecl, llvm::StructType *llvmType, llvm::Type *llvmEltType, uint64_t eltSize, llvm::ArrayRef<int> dims) {
  assert(clangDecl);
  assert(llvmType);
  this->clangDecl = clangDecl;
  this->llvmType = llvmType;
  this->llvmEltType = llvmEltType;
  this->eltSize = eltSize;
  dimLengths.append(dims.begin(), dims.end());

  this->funcGetLocal = NULL;
  this->funcSetLocal = NULL;
  this->funcGetBroadcast = NULL;
  this->funcSetBroadcast = NULL;
  this->funcGetMaster = NULL;
  this->funcSetMaster = NULL;
  this->funcIslocal = NULL;
  this->funcPtrLocal = nullptr;
}


llvm::Value *clang::CodeGen::makeValueFromType(llvm::Type *ty) {
  // Types cannot be directly inserted into an MDNode
  // Instead we create a dummy function with a return type that is a pointer to the type to be represented

  //auto &llvmContext = ty->getContext();
  auto funcTy = llvm::FunctionType::get(llvm::PointerType::getUnqual(ty) ,false); 
  auto func = Function::Create(funcTy, GlobalValue::PrivateLinkage, "TypeRepresentative");
  return func;
}


llvm::Type *clang::CodeGen::extractTypeFromValue(llvm::Value *val) {
  auto funcTy = cast<llvm::FunctionType>(val->getType()->getPointerElementType());
  assert(funcTy->getNumParams() == 0);
  auto ptrTy = funcTy->getReturnType();
  assert(ptrTy->isPointerTy());
  return ptrTy->getPointerElementType();
}


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
    /*[10]*/funcSetMaster,
    /*[11]*/funcIslocal,
    /*[12]*/makeValueFromType(llvmEltType),
    /*[13]*/llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmContext), eltSize),
    /*[14]*/funcPtrLocal
  };
  llvm::MDNode *fieldNode = llvm::MDNode::get(llvmContext, metadata);
  return fieldNode;
}


void FieldTypeMetadata::readMetadata(llvm::Module *llvmModule, llvm::MDNode *metadata) {
  auto magic = dyn_cast<MDString>(metadata->getOperand(0));
  assert(magic->getString() == "field");

  auto clangNameMD = dyn_cast<MDString>(metadata->getOperand(1));
  clangTypeName = clangNameMD->getString();

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
  funcGetLocal = cast_or_null<llvm::Function>(metadata->getOperand(5)); 
  funcSetLocal = cast_or_null<llvm::Function>(metadata->getOperand(6));
  funcGetBroadcast = cast<llvm::Function>(metadata->getOperand(7));
  funcSetBroadcast = cast<llvm::Function>(metadata->getOperand(8));
  funcGetMaster = cast<llvm::Function>(metadata->getOperand(9));
  funcSetMaster = cast<llvm::Function>(metadata->getOperand(10));
  funcIslocal = cast<llvm::Function>(metadata->getOperand(11));

  this->llvmEltType = extractTypeFromValue(metadata->getOperand(12));
  auto eltSizeMD = dyn_cast<llvm::ConstantInt>(metadata->getOperand(13));
  this->eltSize = eltSizeMD->getLimitedValue();

  funcPtrLocal = cast<llvm::Function>(metadata->getOperand(14));
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


static Function *clangFunction2llvmFunction(CodeGenModule *cgm, FunctionDecl *clangFunc) {
  if (isa<CXXMethodDecl>(clangFunc)) {
    auto method = cast<CXXMethodDecl>(clangFunc);
    // This part is actually more complicated to get a function type for methods; for our special case, this should be enough
    auto FInfo = &cgm->getTypes().arrangeCXXMethodDeclaration(method);
    auto ty = cgm->getTypes().GetFunctionType(*FInfo);
    auto llvmFunc = cgm->GetAddrOfFunction(clangFunc, ty);
    return cast<Function>(llvmFunc);
  }

  auto llvmFunc = cgm->GetAddrOfFunction(clangFunc);
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
  assert(cgm->getTypes().ConvertType(clangContext.getTypeDeclType(clangType)) == llvmType);

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

  // Determine the element type
  auto eltTypedef = cast<TypedefDecl>(findMember(cgm, clangType, "ElementType"));
  auto eltType = clangContext.getTypeDeclType(eltTypedef);
  auto llvmEltType = cgm->getTypes().ConvertType(clangContext.getTypeDeclType(eltTypedef));
  auto eltSize = cgm->getDataLayout().getTypeAllocSize(llvmEltType);

  fieldType = new FieldTypeMetadata(clangType, llvmType, llvmEltType, eltSize, lengths);
  return;


  // Get the metadata node
  llvm::NamedMDNode *fieldsMetadata = cgm->getModule().getOrInsertNamedMetadata("molly.fields");

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
    //DEBUG(dbgs() << "    Inlinable Func:" << clangFunc->getNameAsString() << "\n");
  } else {
    //DEBUG(dbgs() << "NOT Inlinable Func:" << clangFunc->getNameAsString() << "\n");
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

    if (clangFunc->hasAttr<MollyIsLocalFuncAttr>()) {
      ab.addAttribute("molly_islocalfunc");
      assert(!field->funcIslocal && "Just one function implementation for Molly specials"); 
      field->funcIslocal = llvmFunc; //TODO: Check function signature
    }

    if (clangFunc->hasAttr<MollyPtrLocalAttr>()) {
      ab.addAttribute("molly_ptrlocalfunc");
      assert(!field->funcPtrLocal && "Just one function implementation for Molly specials"); 
      field->funcPtrLocal = llvmFunc;
    }
  }

  llvmFunc->addAttributes(llvm::AttributeSet::FunctionIndex, llvm::AttributeSet::get(llvmContext, llvm::AttributeSet::FunctionIndex, ab));
}


clang::FunctionDecl *CodeGenMolly::findGlobalFunction(const char *name) {
  auto clangContext = &cgm->getContext();
  auto clangModule = clangContext->getTranslationUnitDecl();

  auto ident = &clangContext->Idents.get(name);
  auto declName = DeclarationName(ident);
  auto lookup = clangModule->lookup(declName);
  assert(!lookup.empty());
  return cast<FunctionDecl>( *lookup.begin());//FIXME: What if multiple are found?
}


clang::QualType CodeGenMolly::findMollyType(const char *name) {
  auto module = &cgm->getModule();
  auto clangContext = &cgm->getContext();
  auto clangModule = clangContext->getTranslationUnitDecl();

  auto mollyIdent = &clangContext->Idents.get("molly");
  auto mollyName = DeclarationName(mollyIdent);
  auto mollyLu = clangModule->lookup(mollyName);
  assert(!mollyLu.empty());
  auto mollyNamespace = cast<NamespaceDecl>(*mollyLu.begin());

  auto rankIdent = &clangContext->Idents.get(name);
  auto rankName = DeclarationName(rankIdent);
  auto rankLu = mollyNamespace->lookup(rankName);
  assert(!rankLu.empty());
  auto rankClangDecl = *rankLu.begin();
  auto typedefTy = clangContext->getTypedefType(cast<TypedefDecl>( rankClangDecl)) ; //FIXME: Accept other type declarations, not just typedef
  return typedefTy;
}


void CodeGenMolly::EmitMetadata() {
  llvm::NamedMDNode *fieldsMetadata = cgm->getModule().getOrInsertNamedMetadata("molly.fields");

  for (auto it = fieldsFound.begin(), end = fieldsFound.end(); it!=end; ++it) {
    auto field = it->second;
    if (!field)
      continue;

    fieldsMetadata->addOperand(field->buildMetadata());
  }

#if 0
  auto module = &cgm->getModule();
  auto clangContext = &cgm->getContext();
  auto clangModule = clangContext->getTranslationUnitDecl();
  //auto declCtx = clangModule->getDeclContext();

  auto x= &clangContext->Idents.get("__molly_combuf_send");
  auto dn= DeclarationName(x);
  auto lu = clangModule->lookup(dn);
  assert(!lu.empty());
  auto clangCombufSend = cast<FunctionDecl>( *lu.begin());
  auto llvmCombufSend = clangFunction2llvmFunction(cgm, clangCombufSend);

  auto mollyIdent = &clangContext->Idents.get("molly");
  auto mollyName = DeclarationName(mollyIdent);
  auto mollyLu = clangModule->lookup(mollyName);
  assert(!mollyLu.empty());
  auto mollyNamespace = cast<NamespaceDecl>(*mollyLu.begin());

  auto rankIdent = &clangContext->Idents.get("rank_t");
  auto rankName = DeclarationName(rankIdent);
  auto rankLu = mollyNamespace->lookup(rankName);
  assert(!rankLu.empty());
  auto rankClangDecl = *rankLu.begin();
  auto rankTy = cgm->getTypes().ConvertType(clangContext->getTypedefType(cast<TypedefDecl>( rankClangDecl))  ); //FIXME: Accept other type declarations, not just typedef
#endif

  MollyRuntimeMetadata rtMetadata;
  rtMetadata.tyCombufSend =  cgm->getTypes().ConvertType(findMollyType("combufsend_t"));
  rtMetadata.tyCombufRecv =  cgm->getTypes().ConvertType(findMollyType("combufrecv_t"));
  rtMetadata.tyRank = cgm->getTypes().ConvertType(findMollyType("rank_t"));
  rtMetadata.funcCreateSendCombuf = clangFunction2llvmFunction(cgm, findGlobalFunction("__molly_sendcombuf_create"));
  rtMetadata.funcCreateRecvCombuf = clangFunction2llvmFunction(cgm, findGlobalFunction("__molly_recvcombuf_create"));
  rtMetadata.funcCombufSend = clangFunction2llvmFunction(cgm, findGlobalFunction("__molly_combuf_send"));
  rtMetadata.funcCombufRecv = clangFunction2llvmFunction(cgm, findGlobalFunction("__molly_combuf_recv"));
  rtMetadata.addMetadata(&cgm->getModule());
}


bool CodeGenMolly::EmitMollyBuiltin(clang::CodeGen::RValue &result, clang::CodeGen::CodeGenModule *cgm, clang::CodeGen::CodeGenFunction *cgf, const clang::FunctionDecl *FD, unsigned BuiltinID, const clang::CallExpr *E) {
  Intrinsic::ID intrincId;
  bool hasSelfArg = true;
  bool hasDimArg = false;
  bool hasValArg = false;
  bool hasCoordArgs = false;
  switch (BuiltinID) {
  case Builtin::BI__builtin_molly_ptr:
    intrincId = Intrinsic::molly_ptr;
    hasCoordArgs = true;
    break;
  case Builtin::BI__builtin_molly_get:
    intrincId = Intrinsic::molly_get;
    hasValArg = true;
    hasCoordArgs = true;
    break;
  case Builtin::BI__builtin_molly_set:
    intrincId = Intrinsic::molly_set;
    hasValArg = true;
    hasCoordArgs = true;
    break;
  case Builtin::BI__builtin_molly_islocal:
    intrincId = Intrinsic::molly_islocal;
    hasCoordArgs = true;
    break;
  case Builtin::BI__builtin_molly_rankof:
    intrincId = Intrinsic::molly_rankof;
    hasCoordArgs = true;
    break;
  case Builtin::BI__builtin_molly_localoffset:
    intrincId = Intrinsic::molly_localoffset;
    hasDimArg = true;
    break;
  case Builtin::BI__builtin_molly_locallength:
    intrincId = Intrinsic::molly_locallength;
    hasDimArg = true;
    break;
  default:
    return false; // Not a Molly intrinsic
  }

  auto &llvmContext = cgm->getLLVMContext();
  auto &clangContext = cgm->getContext();
  auto builder = &cgf->Builder;
  auto nArgs = E->getNumArgs();
  //auto nDims = (nArgs - 1);

  int curArg = 0;
  SmallVector<llvm::Type*, 6> argtypes;
  SmallVector<Value*, 4> args;

  // Query element type 
  // Pointer to field is always first argument
  assert(hasSelfArg);
  auto ptrToSelf = E->getArg(curArg);
  curArg += 1;
  ptrToSelf = ptrToSelf->IgnoreParenImpCasts();
  auto ptrToSelfType = ptrToSelf->getType();
  auto structTy = cast<CXXRecordDecl>(ptrToSelfType->getPointeeType()->getAsCXXRecordDecl());
  auto eltTypedef = cast<TypedefDecl>(findMember(cgm, structTy, "ElementType"));
  auto eltType = clangContext.getTypeDeclType(eltTypedef);
  auto llvmEltType = cgf->ConvertType(eltTypedef);
  auto llvmPtrToEltType = cgf->ConvertType(clangContext.getPointerType(eltType));

  // Return type
  switch (BuiltinID) {  
  case Builtin::BI__builtin_molly_ptr:
    // Pointer to buffer
    argtypes.push_back(llvmPtrToEltType);
    break;
  case Builtin::BI__builtin_molly_islocal:
    // bool
    //argtypes.push_back(llvm::Type::getInt1Ty(llvmContext)); //FIXME: This is fixed, isn't it?
    break;
  case Builtin::BI__builtin_molly_rankof:
    // int
    //argtypes.push_back(llvm::Type::getInt32Ty(llvmContext));  //FIXME: This is fixed, isn't it?
    break;
  default:
    // No return or fixed
    //argtypes.push_back(llvm::Type::getVoidTy(llvmContext)); 
    break;
  }

  // Ptr to field is always first argument
  Value *thisArg = cgf->EmitScalarExpr(ptrToSelf);
  args.push_back(thisArg);
  argtypes.push_back(thisArg->getType()); 

  // Dimension arg
  if (hasDimArg) {
    auto argDimExpr = E->getArg(curArg); 
    curArg+=1;
    auto argDim = cgf->EmitScalarExpr(argDimExpr); 
    args.push_back(argDim);
  }

  // val arg
  if (hasValArg) {
    auto argValExpr = E->getArg(curArg); 
    curArg+=1;
    auto argVar = cgf->EmitScalarExpr(argValExpr); // Pointer to buffer the value is read/written to
    args.push_back(argVar);
  }

  // Coordinate arguments, must be last
  if (hasCoordArgs) {
    auto nDims = nArgs - curArg; //TODO: Check if matches field type
    for (auto d = nArgs-nDims;d<nArgs;d+=1) {
      argtypes.push_back(IntegerType::getInt32Ty(llvmContext)); // Type of a coordinate 

      auto coordArgExpr = E->getArg(d);
      curArg += 1;
      Value *coordArg = cgf->EmitScalarExpr(coordArgExpr);
      args.push_back(coordArg);
    }
  }
  assert(curArg == nArgs && "Unexpected number of arguments");

  Function *func = cgm->getIntrinsic(intrincId, argtypes);
  auto callInstr = builder->CreateCall(func, args, "mollycall");
  result = RValue::get(callInstr);
  return true;
}
