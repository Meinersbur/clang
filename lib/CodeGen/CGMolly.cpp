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
#include "clang/AST/GlobalDecl.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Instructions.h"

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
  this->funcCreateSendCombuf = cast_or_null<Function>(metadata->getOperand(3));
  this->funcCreateRecvCombuf = cast_or_null<Function>(metadata->getOperand(4));
  this->funcCombufSend = cast_or_null<Function>(metadata->getOperand(5));
  this->funcCombufRecv = cast_or_null<Function>(metadata->getOperand(6));
  //this->varSelfCoords = cast_or_null<GlobalVariable>(metadata->getOperand(7));
  this->funcLocalCoord =  cast_or_null<Function>(metadata->getOperand(7));
  assert( metadata->getNumOperands()==8);
}


llvm::MDNode *MollyRuntimeMetadata::buildMetadata(llvm::Module *llvmModule) {
  auto &llvmContext = llvmModule->getContext();

  llvm::Value* metadata[] = {
    /*[ 0]*/makeValueFromType(tyCombufSend, llvmModule),
    /*[ 1]*/makeValueFromType(tyCombufRecv, llvmModule),
    /*[ 2]*/makeValueFromType(tyRank, llvmModule),
    /*[ 3]*/funcCreateSendCombuf,
    /*[ 4]*/funcCreateRecvCombuf,
    /*[ 5]*/funcCombufSend,
    /*[ 6]*/funcCombufRecv,
    //   /*[ 7]*/varSelfCoords
    /*[ 7]*/funcLocalCoord
  };
  llvm::MDNode *metadataNode = llvm::MDNode::get(llvmContext, metadata);
  //metadataNode->dump();
  return metadataNode;
}


void MollyRuntimeMetadata::addMetadata(llvm::Module *llvmModule){
  auto metadata = llvmModule->getOrInsertNamedMetadata("molly.runtime");
  assert(metadata->getNumOperands()==0);
  metadata->addOperand(buildMetadata(llvmModule));
}


FieldTypeMetadata::FieldTypeMetadata() {
  this->node = nullptr;
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
  this->node = nullptr;
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


llvm::Value *clang::CodeGen::makeValueFromType(llvm::Type *ty, llvm::Module *module) {
  // Types cannot be directly inserted into an MDNode
  // Instead we create a dummy global value with a type that is a pointer to the type to be represented
  if (ty==nullptr)
    return nullptr;

  auto &llvmContext = module->getContext();
  auto pty = llvm::PointerType::getUnqual(ty);
  return Constant::getNullValue(pty);

  auto funcTy = llvm::FunctionType::get(pty, false); 
  auto func = Function::Create(funcTy, GlobalValue::PrivateLinkage, Twine(), module); // "__TypeRepresentative"
  IRBuilder<> builder(BasicBlock::Create(llvmContext, "Entry", func));
  builder.CreateRet(Constant::getNullValue(pty)); // Return something to make it a valif function definition
  return func;
}


llvm::Type *clang::CodeGen::extractTypeFromValue(llvm::Value *val) {
  if (!val)
    return nullptr;
  auto pty = val->getType();
  return cast<llvm::PointerType>(pty)->getElementType();

  auto funcTy = cast<llvm::FunctionType>(val->getType()->getPointerElementType());
  assert(funcTy->getNumParams() == 0);
  auto ptrTy = funcTy->getReturnType();
  assert(ptrTy->isPointerTy());
  return ptrTy->getPointerElementType();
}


llvm::MDNode *FieldTypeMetadata::buildMetadata(llvm::Module *llvmModule) {
  // if (this->node) {
  //  return node; }

  auto &llvmContext = llvmType->getContext();

  // Create metadata info
  llvm::SmallVector<llvm::Value*, 4> lengthsAsValue;
  for (auto it = dimLengths.begin(), end = dimLengths.end(); it!=end; ++it) {
    auto length = *it;
    lengthsAsValue.push_back(ConstantInt::get(llvm::Type::getInt32Ty(llvmContext), length));
  }

  llvm::MDNode *lengthsNode = llvm::MDNode::get(llvmContext, lengthsAsValue);

  assert(funcGetBroadcast);
  assert(funcSetBroadcast);
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
    /*[12]*/makeValueFromType(llvmEltType, llvmModule),
    /*[13]*/llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmContext), eltSize),
    /*[14]*/funcPtrLocal
  };
  llvm::MDNode *fieldNode = llvm::MDNode::get(llvmContext, metadata);
  this->node = fieldNode;
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


llvm::MDNode *FieldVarMetadata::buildMetadata(llvm::Module *llvmModule) {
  auto &llvmContext = llvmModule->getContext();

  llvm::Value* metadata[] = {
    /*[ 0]*/llvm::MDString::get(llvmContext, "fieldvar"), 
    /*[ 1]*/llvm::MDString::get(llvmContext, islstr),
    /*[ 2]*/llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmContext), clusterdims)
  };
  llvm::MDNode *fieldNode = llvm::MDNode::get(llvmContext, metadata);
  return fieldNode;
}


  void FieldVarMetadata::readMetadata(llvm::Module *llvmModule, llvm::MDNode *metadata) {
assert(metadata->getNumOperands() == 3);
      auto magic = dyn_cast<MDString>(metadata->getOperand(0));
  assert(magic->getString() == "fieldvar");

  auto iststrMD = dyn_cast<MDString>(metadata->getOperand(1));
  islstr = iststrMD->getString();

  auto clusterdimsMD = dyn_cast<llvm::ConstantInt>(metadata->getOperand(2));
  clusterdims = clusterdimsMD->getLimitedValue();
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
  if (!clangFunc)
    return nullptr;

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


static llvm::Type *clangType2llvmType(CodeGenModule *cgm, QualType clangTy) { 
  if (clangTy.isNull())
    return nullptr;
  return cgm->getTypes().ConvertType(clangTy);
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

  auto &fieldType = fieldsFound[clangType];
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
  auto llvmEltType = cgm->getTypes().ConvertTypeForMem(clangContext.getTypeDeclType(eltTypedef));
  auto eltSize = cgm->getDataLayout().getTypeAllocSize(llvmEltType);

  fieldType = new FieldTypeMetadata(clangType, llvmType, llvmEltType, eltSize, lengths);
  return;

#if 0
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
#endif
}


void CodeGenMolly::annotateFunction(const clang::FunctionDecl *clangFunc, llvm::Function *llvmFunc) {
  assert(clangFunc);
  assert(llvmFunc);

  auto &llvmContext = cgm->getLLVMContext();
  auto &clangContext = cgm->getContext();

  // TODO: mark special attributes (especially readOnly, mayWriteToMemory, mayThrow)
  llvm::AttrBuilder ab;


  if (llvmFunc->getName() == "_ZN5molly24_array_partial_subscriptIbNS_11_dimlengthsIJLy6EEEES2_EixEy") {
    int a = 0;
  }

  if (clangFunc->hasAttr<MollyInlineAttr>()) {
    ab.addAttribute("molly_inline");
    //DEBUG(dbgs() << "    Inlinable Func:" << clangFunc->getNameAsString() << "\n");
  } else {
    //DEBUG(dbgs() << "NOT Inlinable Func:" << clangFunc->getNameAsString() << "\n");
  }

  if (clangFunc->hasAttr<MollyPureAttr>()) {
    ab.addAttribute("molly_pure");
  }

  auto clangFieldDecl = dyn_cast<CXXRecordDecl>(clangFunc->getParent());
  if (clangFieldDecl) {

    if (fieldsFound.find(clangFieldDecl)==fieldsFound.end()){
      // Not a registered field; because annotateFieldType has been called before any of the classes members are created, annotateFieldType abviously decided that the class is not a field
    } else {
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

      if (clangFunc->hasAttr<MollyFieldRankofAttr>()) {
        ab.addAttribute("molly_field_rankof");
      }

      if (clangFunc->hasAttr<MollyLocalIndexofAttr>()) {
        ab.addAttribute("molly_local_indexof");
      }
    }
  }

  llvmFunc->addAttributes(llvm::AttributeSet::FunctionIndex, llvm::AttributeSet::get(llvmContext, llvm::AttributeSet::FunctionIndex, ab));
}


clang::FunctionDecl *CodeGenMolly::findGlobalFunction(const char *name) {
  auto clangContext = &cgm->getContext();
  auto clangModule = clangContext->getTranslationUnitDecl();

  auto ident = &clangContext->Idents.get(name);
  if (!ident)
    return nullptr;

  auto declName = DeclarationName(ident);
  auto lookup = clangModule->lookup(declName);
  if (lookup.empty())
    return nullptr;

  return cast<FunctionDecl>( *lookup.begin());//FIXME: What if multiple are found?
}


clang::VarDecl *CodeGenMolly::findGlobalVariable(const char *name) {
  auto clangContext = &cgm->getContext();
  auto clangModule = clangContext->getTranslationUnitDecl();

  auto ident = &clangContext->Idents.get(name);
  auto declName = DeclarationName(ident);
  auto lookup = clangModule->lookup(declName);
  assert(!lookup.empty());
  return cast<VarDecl>(*lookup.begin());
}


clang::QualType CodeGenMolly::findMollyType(const char *name) {
  auto module = &cgm->getModule();
  auto clangContext = &cgm->getContext();
  auto clangModule = clangContext->getTranslationUnitDecl();

  auto mollyIdent = &clangContext->Idents.get("molly");
  auto mollyName = DeclarationName(mollyIdent);
  auto mollyLu = clangModule->lookup(mollyName);
  if (mollyLu.empty())
    return QualType();

  auto mollyNamespace = cast<NamespaceDecl>(*mollyLu.begin());

  auto rankIdent = &clangContext->Idents.get(name);
  auto rankName = DeclarationName(rankIdent);
  auto rankLu = mollyNamespace->lookup(rankName);
  assert(!rankLu.empty());
  auto rankClangDecl = *rankLu.begin();
  auto typedefTy = clangContext->getTypedefType(cast<TypedefDecl>(rankClangDecl)) ; //FIXME: Accept other type declarations, not just typedef
  return typedefTy;
}


void CodeGenMolly::EmitMetadata() {
  auto module = &cgm->getModule();
  llvm::NamedMDNode *fieldsMetadata = module->getOrInsertNamedMetadata("molly.fields");

  llvm::DenseMap<const clang::CXXRecordDecl*, llvm::MDNode *> mdNodes;
  for (auto it = fieldsFound.begin(), end = fieldsFound.end(); it!=end; ++it) {
    auto field = it->second;
    if (!field)
      continue;

    auto md = field->buildMetadata(module);
    mdNodes[it->first] = md;
    fieldsMetadata->addOperand(md);
  }

  for (auto &p : pendingMetadata) {
    auto instr = p.first;
    auto structTy = p.second;

    auto md = mdNodes[structTy];
    assert(md);
    instr->setArgOperand(1, md);
  }
  pendingMetadata.clear();

  MollyRuntimeMetadata rtMetadata;
  rtMetadata.tyCombufSend = clangType2llvmType(cgm, findMollyType("combufsend_t"));
  rtMetadata.tyCombufRecv = clangType2llvmType(cgm, findMollyType("combufrecv_t"));
  rtMetadata.tyRank = clangType2llvmType(cgm, findMollyType("rank_t"));
  rtMetadata.funcCreateSendCombuf = clangFunction2llvmFunction(cgm, findGlobalFunction("__molly_sendcombuf_create"));
  rtMetadata.funcCreateRecvCombuf = clangFunction2llvmFunction(cgm, findGlobalFunction("__molly_recvcombuf_create"));
  rtMetadata.funcCombufSend = clangFunction2llvmFunction(cgm, findGlobalFunction("__molly_combuf_send"));
  rtMetadata.funcCombufRecv = clangFunction2llvmFunction(cgm, findGlobalFunction("__molly_combuf_recv"));
  rtMetadata.funcLocalCoord = clangFunction2llvmFunction(cgm, findGlobalFunction("__molly_local_coord"));

  //auto coordsVar = findGlobalVariable("_cart_self_coords");
  //auto llvmCoordsVar = cgm->GetAddrOfGlobalVar(coordsVar);
  //rtMetadata.varSelfCoords = cast<GlobalVariable>(llvmCoordsVar);

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

#if 0
  case Builtin::BI__builtin_molly_localoffset:
    intrincId = Intrinsic::molly_localoffset;
    hasDimArg = true;
    break;
  case Builtin::BI__builtin_molly_locallength:
    intrincId = Intrinsic::molly_locallength;
    hasDimArg = true;
    break;
#endif

  case Builtin::BI__builtin_molly_global_init:
    intrincId = Intrinsic::molly_global_init;
    hasSelfArg = false;
    break;
  case Builtin::BI__builtin_molly_global_free:
    intrincId = Intrinsic::molly_global_free;
    hasSelfArg = false;
    break;
  case Builtin::BI__builtin_molly_field_init:
    intrincId = Intrinsic::molly_field_init;
    break;
  case Builtin::BI__builtin_molly_field_free:
    intrincId = Intrinsic::molly_field_free;
    break;
  case Builtin::BI__builtin_molly_local_indexof:
    intrincId = Intrinsic::molly_local_indexof;
    hasCoordArgs = true;
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


  llvm::Type *llvmPtrToEltType=nullptr;
  const Expr *ptrToSelf=nullptr;
  const CXXRecordDecl *structTy=nullptr;
  if (hasSelfArg) {
    ptrToSelf = E->getArg(curArg);
    curArg += 1;
    ptrToSelf = ptrToSelf->IgnoreParenImpCasts();
    auto ptrToSelfType = ptrToSelf->getType();
    structTy = cast<CXXRecordDecl>(ptrToSelfType->getPointeeType()->getAsCXXRecordDecl());
    auto eltTypedef = cast<TypedefDecl>(findMember(cgm, structTy, "ElementType"));
    auto eltType = clangContext.getTypeDeclType(eltTypedef);
    auto llvmEltType = cgf->ConvertType(eltTypedef);
    llvmPtrToEltType = cgf->ConvertType(clangContext.getPointerType(eltType));
  }

  // Return type
  switch (BuiltinID) {  
  case Builtin::BI__builtin_molly_ptr:
    // Pointer to buffer
    assert(llvmPtrToEltType);
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
    // void or fixed
    //argtypes.push_back(llvm::Type::getVoidTy(llvmContext)); 
    break;
  }

  // Ptr to field is always first argument
  if (hasSelfArg) {
    Value *thisArg = cgf->EmitScalarExpr(ptrToSelf);
    args.push_back(thisArg);
    argtypes.push_back(thisArg->getType()); 
  }

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
      argtypes.push_back(IntegerType::getInt64Ty(llvmContext)); // Type of a coordinate 

      auto coordArgExpr = E->getArg(d);
      curArg += 1;
      Value *coordArg = cgf->EmitScalarExpr(coordArgExpr);
      args.push_back(coordArg);
    }
  }
  assert(curArg == nArgs && "Unexpected number of arguments");

  if (intrincId==Intrinsic::molly_field_init) {
    //auto md = fieldsFound[structTy];
    //args.push_back(md->buildMetadata(&cgm->getModule()));
    args.push_back( llvm::MDNode::get(llvmContext, ArrayRef<llvm::Value*>() )); // Metadata not known yet, must be added later
  }

  Function *func = cgm->getIntrinsic(intrincId, argtypes);
  auto callInstr = builder->CreateCall(func, args, func->getReturnType()->isVoidTy() ? Twine() : "mollycall");

  if (intrincId==Intrinsic::molly_field_init) {
    pendingMetadata.emplace_back(callInstr, structTy);
  }

  result = RValue::get(callInstr);
  return true;
}


 void CodeGenMolly::annotateFieldVar(const clang::VarDecl *clangVar, llvm::GlobalVariable *llvmVar) {
   auto transformAttr = clangVar->getAttr<MollyTransformAttr>();
   if (transformAttr) {
     // #pragma molly transform

     // llvm::GlobalVariable doesn't have attributes (for llvm::CallInst, llvm::InvokeInst and llvm::Function only) nor metadata (for llvm::Instruction and llvm::Module only), so we cannot annotate it with the DataLayout is has
     // Instead, we augment metadata in an artificial init call in the variable initializer
   }
 }


  void CodeGenMolly::annotateFieldVarInit(const clang::VarDecl *clangVar, llvm::GlobalVariable *llvmVar, llvm::Function *llvmInitFunc) {
    auto transformAttr = clangVar->getAttr<MollyTransformAttr>();  // #pragma molly transform
    if (transformAttr) {
      // Append an intrinsic llvm.molly.field.init
      // It will pass the metadata to this field instance

      auto module = llvmVar->getParent();

      auto entry = &llvmInitFunc->getEntryBlock();
      auto term = entry->getTerminator();

      FieldVarMetadata md;
      md.islstr = transformAttr->getIslstr();
      md.clusterdims = transformAttr->getNodedims();
      auto mdnode = md.buildMetadata(module);

      llvm::Value *args[] = { llvmVar, mdnode };
      llvm::Type *tys[] = { args[0]->getType() };
      auto intrInit = Intrinsic::getDeclaration(module, Intrinsic::molly_field_init, tys);

      // Create the init call with the metadata for this var
      auto call = CallInst::Create(intrInit, args, Twine(), term);
    }
  }
