//===---- CGLoopInfo.cpp - LLVM CodeGen for loop metadata -*- C++ -*-------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CGLoopInfo.h"
#include "CodeGenFunction.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"

using namespace clang::CodeGen;
using namespace llvm;

static MDNode *createMetadata(LLVMContext &Ctx, Function *F,
                              CodeGenFunction *CGF, LoopAttributes &Attrs,
                              const llvm::DebugLoc &StartLoc,
                              const llvm::DebugLoc &EndLoc, MDNode *&AccGroup) {

  if (!Attrs.IsParallel && Attrs.VectorizeWidth == 0 &&
      Attrs.InterleaveCount == 0 && Attrs.UnrollCount == 0 &&
      Attrs.UnrollAndJamCount == 0 && !Attrs.PipelineDisabled &&
      Attrs.PipelineInitiationInterval == 0 &&
      Attrs.VectorizeEnable == LoopAttributes::Unspecified &&
      Attrs.UnrollEnable == LoopAttributes::Unspecified &&
      Attrs.UnrollAndJamEnable == LoopAttributes::Unspecified &&
      Attrs.DistributeEnable == LoopAttributes::Unspecified && !StartLoc &&
      !EndLoc && Attrs.LoopId.empty() && Attrs.TransformationStack.empty())
    return nullptr;

  SmallVector<Metadata *, 4> Args;
  // Reserve operand 0 for loop id self reference.
  auto TempNode = MDNode::getTemporary(Ctx, None);
  Args.push_back(TempNode.get());

  // If we have a valid start debug location for the loop, add it.
  if (StartLoc) {
    Args.push_back(StartLoc.getAsMDNode());

    // If we also have a valid end debug location for the loop, add it.
    if (EndLoc)
      Args.push_back(EndLoc.getAsMDNode());
  }

  // Setting vectorize.width
  if (Attrs.VectorizeWidth > 0) {
    Metadata *Vals[] = {MDString::get(Ctx, "llvm.loop.vectorize.width"),
                        ConstantAsMetadata::get(ConstantInt::get(
                            Type::getInt32Ty(Ctx), Attrs.VectorizeWidth))};
    Args.push_back(MDNode::get(Ctx, Vals));
  }

  // Setting interleave.count
  if (Attrs.InterleaveCount > 0) {
    Metadata *Vals[] = {MDString::get(Ctx, "llvm.loop.interleave.count"),
                        ConstantAsMetadata::get(ConstantInt::get(
                            Type::getInt32Ty(Ctx), Attrs.InterleaveCount))};
    Args.push_back(MDNode::get(Ctx, Vals));
  }

  // Setting unroll.count
  if (Attrs.UnrollCount > 0) {
    Metadata *Vals[] = {MDString::get(Ctx, "llvm.loop.unroll.count"),
                        ConstantAsMetadata::get(ConstantInt::get(
                            Type::getInt32Ty(Ctx), Attrs.UnrollCount))};
    Args.push_back(MDNode::get(Ctx, Vals));
  }

  // Setting unroll_and_jam.count
  if (Attrs.UnrollAndJamCount > 0) {
    Metadata *Vals[] = {MDString::get(Ctx, "llvm.loop.unroll_and_jam.count"),
                        ConstantAsMetadata::get(ConstantInt::get(
                            Type::getInt32Ty(Ctx), Attrs.UnrollAndJamCount))};
    Args.push_back(MDNode::get(Ctx, Vals));
  }

  // Setting vectorize.enable
  if (Attrs.VectorizeEnable != LoopAttributes::Unspecified) {
    Metadata *Vals[] = {MDString::get(Ctx, "llvm.loop.vectorize.enable"),
                        ConstantAsMetadata::get(ConstantInt::get(
                            Type::getInt1Ty(Ctx), (Attrs.VectorizeEnable ==
                                                   LoopAttributes::Enable)))};
    Args.push_back(MDNode::get(Ctx, Vals));
  }

  // Setting unroll.full or unroll.disable
  if (Attrs.UnrollEnable != LoopAttributes::Unspecified) {
    std::string Name;
    if (Attrs.UnrollEnable == LoopAttributes::Enable)
      Name = "llvm.loop.unroll.enable";
    else if (Attrs.UnrollEnable == LoopAttributes::Full)
      Name = "llvm.loop.unroll.full";
    else
      Name = "llvm.loop.unroll.disable";
    Metadata *Vals[] = {MDString::get(Ctx, Name)};
    Args.push_back(MDNode::get(Ctx, Vals));
  }

  // Setting unroll_and_jam.full or unroll_and_jam.disable
  if (Attrs.UnrollAndJamEnable != LoopAttributes::Unspecified) {
    std::string Name;
    if (Attrs.UnrollAndJamEnable == LoopAttributes::Enable)
      Name = "llvm.loop.unroll_and_jam.enable";
    else if (Attrs.UnrollAndJamEnable == LoopAttributes::Full)
      Name = "llvm.loop.unroll_and_jam.full";
    else
      Name = "llvm.loop.unroll_and_jam.disable";
    Metadata *Vals[] = {MDString::get(Ctx, Name)};
    Args.push_back(MDNode::get(Ctx, Vals));
  }

  if (Attrs.DistributeEnable != LoopAttributes::Unspecified) {
    Metadata *Vals[] = {MDString::get(Ctx, "llvm.loop.distribute.enable"),
                        ConstantAsMetadata::get(ConstantInt::get(
                            Type::getInt1Ty(Ctx), (Attrs.DistributeEnable ==
                                                   LoopAttributes::Enable)))};
    Args.push_back(MDNode::get(Ctx, Vals));
  }

  if (Attrs.IsParallel) {
    AccGroup = MDNode::getDistinct(Ctx, {});
    Args.push_back(MDNode::get(
        Ctx, {MDString::get(Ctx, "llvm.loop.parallel_accesses"), AccGroup}));
  }

  if (Attrs.PipelineDisabled) {
    Metadata *Vals[] = {
        MDString::get(Ctx, "llvm.loop.pipeline.disable"),
        ConstantAsMetadata::get(ConstantInt::get(
            Type::getInt1Ty(Ctx), (Attrs.PipelineDisabled == true)))};
    Args.push_back(MDNode::get(Ctx, Vals));
  }

  if (Attrs.PipelineInitiationInterval > 0) {
    Metadata *Vals[] = {
        MDString::get(Ctx, "llvm.loop.pipeline.initiationinterval"),
        ConstantAsMetadata::get(ConstantInt::get(
            Type::getInt32Ty(Ctx), Attrs.PipelineInitiationInterval))};
    Args.push_back(MDNode::get(Ctx, Vals));
  }

  if (!Attrs.LoopId.empty()) {
    Metadata *Vals[] = {MDString::get(Ctx, "llvm.loop.id"),
                        MDString::get(Ctx, Attrs.LoopId)};
    Args.push_back(MDNode::get(Ctx, Vals));
  }

  // Set the first operand to itself.
  MDNode *LoopID = MDNode::get(Ctx, Args);
  LoopID->replaceOperandWith(0, LoopID);

  SmallVector<MDNode *, 4> AdditionalTransforms;
  SmallVector<Metadata *, 8> AllTransforms;
  auto FuncMD = F->getMetadata("looptransform");
  if (FuncMD) {
    for (auto &X : FuncMD->operands()) {
      auto Op = cast<MDNode>(X.get());
      AllTransforms.push_back(Op);
    }
  }

  auto TopLoopId = LoopID;
  for (auto &Transform : reverse(Attrs.TransformationStack)) {
    switch (Transform.Kind) {
    default:
      llvm_unreachable("unexpected transformation");
    case LoopTransformation::Reversal: {
      SmallVector<Metadata *, 4> TransformArgs;
      TransformArgs.push_back(MDString::get(Ctx, "llvm.loop.reverse"));

      auto ApplyOn = Transform.getApplyOn();
      if (ApplyOn.empty()) {
        // Apply on TopLoopId
        assert(TopLoopId);
        TransformArgs.push_back(TopLoopId);
      } else {
        // Apply on Transform.ApplyOn
        // TODO: Search for LoopID instead of using the name?
        TransformArgs.push_back(MDString::get(Ctx, ApplyOn));
      }

      auto MDTransform = MDNode::get(Ctx, TransformArgs);

      // auto Transforms =  MDNode::get(Ctx, MDTransform); // FIXME: Allow
      // multiple transformation
      // F->addMetadata("looptransform", *Transforms);
      AdditionalTransforms.push_back(MDTransform);
      AllTransforms.push_back(MDTransform);

      // TODO: Different scheme for transformations that output more than one
      TopLoopId = MDTransform;
    } break;
    case LoopTransformation::Tiling: {
      SmallVector<Metadata *, 4> TransformArgs;
      TransformArgs.push_back(MDString::get(Ctx, "llvm.loop.tile"));

      SmallVector<Metadata *, 4> ApplyOnArgs;
      if (Transform.ApplyOns.empty()) {
        // Apply on top loop
        assert(TopLoopId);
        ApplyOnArgs.push_back(TopLoopId);
      } else {
        for (auto ApplyOn : Transform.ApplyOns) {
          assert(!ApplyOn.empty() && "Must specify loops to tile");
          ApplyOnArgs.push_back(MDString::get(Ctx, ApplyOn));
        }
      }
      TransformArgs.push_back(MDNode::get(Ctx, ApplyOnArgs));

      SmallVector<Metadata *, 4> TileSizeArgs;
      for (auto TileSize : Transform.TileSizes) {
        assert(TileSize > 0 && "Must specify tile size");
        TileSizeArgs.push_back(ConstantAsMetadata::get(
            ConstantInt::get(Type::getInt64Ty(Ctx), TileSize)));
      }
      TransformArgs.push_back(MDNode::get(Ctx, TileSizeArgs));

      SmallVector<Metadata *, 4> PitIdArgs;
      for (auto PitId : Transform.TilePitIds)
        PitIdArgs.push_back(MDString::get(Ctx, PitId));
      TransformArgs.push_back(MDNode::get(Ctx, PitIdArgs));

      SmallVector<Metadata *, 4> TileIdArgs;
      for (auto TileId : Transform.TileTileIds)
        TileIdArgs.push_back(MDString::get(Ctx, TileId));
      TransformArgs.push_back(MDNode::get(Ctx, TileIdArgs));

      assert(TileSizeArgs.empty() ||
             (TileSizeArgs.size() == ApplyOnArgs.size()));
      auto MDTransform = MDNode::get(Ctx, TransformArgs);

      AdditionalTransforms.push_back(MDTransform);
      AllTransforms.push_back(MDTransform);

      TopLoopId = nullptr; // No unique follow-up node
    } break;
    case LoopTransformation::Interchange: {
      SmallVector<Metadata *, 4> TransformArgs;
      TransformArgs.push_back(MDString::get(Ctx, "llvm.loop.interchange"));

      SmallVector<Metadata *, 4> ApplyOnArgs;
      assert(!Transform.ApplyOns.empty());
      assert(Transform.ApplyOns.size() == Transform.Permutation.size());

      for (auto ApplyOn : Transform.ApplyOns) {
        assert(!ApplyOn.empty() && "Must specify loops to tile");
        ApplyOnArgs.push_back(MDString::get(Ctx, ApplyOn));
      }
      TransformArgs.push_back(MDNode::get(Ctx, ApplyOnArgs));

      SmallVector<Metadata *, 4> PermutationArgs;
      for (auto PermuteItem : Transform.Permutation) {
        assert(!PermuteItem.empty() && "Must specify loop id");
        PermutationArgs.push_back(MDString::get(Ctx, PermuteItem));
      }
      TransformArgs.push_back(MDNode::get(Ctx, PermutationArgs));

      auto MDTransform = MDNode::get(Ctx, TransformArgs);
      AdditionalTransforms.push_back(MDTransform);
      AllTransforms.push_back(MDTransform);

      TopLoopId = nullptr; // No unique follow-up node
    } break;
    case LoopTransformation::Pack: {
      SmallVector<Metadata *, 4> TransformArgs;
      TransformArgs.push_back(MDString::get(Ctx, "llvm.data.pack"));

      auto ApplyOn = Transform.getApplyOn();
      if (ApplyOn.empty()) {
        // Apply on TopLoopId
        assert(TopLoopId);
        TransformArgs.push_back(TopLoopId);
      } else {
        // Apply on Transform.ApplyOn
        // TODO: Search for LoopID instead of using the name?
        TransformArgs.push_back(MDString::get(Ctx, ApplyOn));
      }

     // auto Var = Transform.Array->getDecl();
      auto LVar = CGF->EmitLValue(Transform.Array);
      auto Addr = cast<AllocaInst>(LVar.getPointer());
      assert(!Transform.ArrayBasePtr);
      Transform.ArrayBasePtr = Addr;
      // TransformArgs.push_back(LocalAsMetadata::get(Addr));

      auto Accesses = MDNode::get(Ctx, {});
      TransformArgs.push_back(Accesses);

      TransformArgs.push_back(Transform.OnHeap ? MDString::get(Ctx, "malloc")
                                               : MDString::get(Ctx, "alloca"));

      auto MDTransform = MDNode::get(Ctx, TransformArgs);
      Transform.TransformMD = MDTransform;
      AdditionalTransforms.push_back(MDTransform);
      AllTransforms.push_back(MDTransform);

      // Follow-ups use this one
      TopLoopId = MDTransform;
    } break;

    case LoopTransformation::Unrolling: {
      SmallVector<Metadata *, 4> TransformArgs;
      TransformArgs.push_back(MDString::get(Ctx, "llvm.loop.unroll"));

      auto ApplyOn = Transform.getApplyOn();
      if (ApplyOn.empty()) {
        assert(TopLoopId);
        TransformArgs.push_back(TopLoopId);
      } else {
        TransformArgs.push_back(MDString::get(Ctx, ApplyOn));
      }

      auto UnrollFactor = Transform.Factor;
      auto IsFullUnroll = Transform.Full;
      if (UnrollFactor > 0 && IsFullUnroll) {
        llvm_unreachable("Contradicting state");
      } else if (UnrollFactor > 0) {
        TransformArgs.push_back(ConstantAsMetadata::get(
            ConstantInt::get(Type::getInt64Ty(Ctx), UnrollFactor)));
      } else if (IsFullUnroll) {
        TransformArgs.push_back(MDString::get(Ctx, "full"));
      } else {
        TransformArgs.push_back(
            nullptr); // Determine unroll factor heuristically
      }

      auto MDTransform = MDNode::get(Ctx, TransformArgs);
      Transform.TransformMD = MDTransform;
      AdditionalTransforms.push_back(MDTransform);
      AllTransforms.push_back(MDTransform);

      // Follow-ups use this one
      TopLoopId = MDTransform;

    } break;
	case LoopTransformation::ThreadParallel: {
		SmallVector<Metadata *, 4> TransformArgs;
		TransformArgs.push_back(MDString::get(Ctx, "llvm.loop.parallelize_thread"));

		auto ApplyOn = Transform.getApplyOn();
		if (ApplyOn.empty()) {
			assert(TopLoopId);
			TransformArgs.push_back(TopLoopId);
		} else {
			TransformArgs.push_back(MDString::get(Ctx, ApplyOn));
		}

		auto MDTransform = MDNode::get(Ctx, TransformArgs);
		Transform.TransformMD = MDTransform;
		AdditionalTransforms.push_back(MDTransform);
		AllTransforms.push_back(MDTransform);

		// No further transformations after parallelizing
	} break;
    }
  }

  if (!AdditionalTransforms.empty()) {
    auto AllTransformsMD = MDNode::get(Ctx, AllTransforms);
    F->setMetadata("looptransform", AllTransformsMD);
  }

  return LoopID;
}

LoopAttributes::LoopAttributes(bool IsParallel)
    : IsParallel(IsParallel), VectorizeEnable(LoopAttributes::Unspecified),
      UnrollEnable(LoopAttributes::Unspecified),
      UnrollAndJamEnable(LoopAttributes::Unspecified), VectorizeWidth(0),
      InterleaveCount(0), UnrollCount(0), UnrollAndJamCount(0),
      DistributeEnable(LoopAttributes::Unspecified), PipelineDisabled(false),
      PipelineInitiationInterval(0) {}

void LoopAttributes::clear() {
  IsParallel = false;
  VectorizeWidth = 0;
  InterleaveCount = 0;
  UnrollCount = 0;
  UnrollAndJamCount = 0;
  VectorizeEnable = LoopAttributes::Unspecified;
  UnrollEnable = LoopAttributes::Unspecified;
  UnrollAndJamEnable = LoopAttributes::Unspecified;
  DistributeEnable = LoopAttributes::Unspecified;
  PipelineDisabled = false;
  PipelineInitiationInterval = 0;
  LoopId = StringRef();
  TransformationStack.clear();
}

LoopInfo::LoopInfo(BasicBlock *Header, Function *F,
                   clang::CodeGen::CodeGenFunction *CGF,
                   const LoopAttributes &Attrs, const llvm::DebugLoc &StartLoc,
                   const llvm::DebugLoc &EndLoc)
    : LoopID(nullptr), Header(Header), Attrs(Attrs) {
  LoopID =
      createMetadata(Header->getContext(), F, CGF, this->Attrs, StartLoc, EndLoc, AccGroup);
}

void LoopInfoStack::push(BasicBlock *Header, Function *F,
                         clang::CodeGen::CodeGenFunction *CGF,
                         const llvm::DebugLoc &StartLoc,
                         const llvm::DebugLoc &EndLoc) {
  Active.push_back(LoopInfo(Header, F, CGF, StagedAttrs, StartLoc, EndLoc));
  // Clear the attributes so nested loops do not inherit them.
  StagedAttrs.clear();
}

void LoopInfoStack::push(BasicBlock *Header, Function *F,
                         clang::CodeGen::CodeGenFunction *CGF,
                         clang::ASTContext &Ctx,
                         ArrayRef<const clang::Attr *> Attrs,
                         const llvm::DebugLoc &StartLoc,
                         const llvm::DebugLoc &EndLoc) {

  // Identify loop hint attributes from Attrs.
  for (const auto *Attr : Attrs) {
    if (auto LId = dyn_cast<LoopIdAttr>(Attr)) {
      setLoopId(LId->getLoopName());
      continue;
    }
    if (auto LReversal = dyn_cast<LoopReversalAttr>(Attr)) {
      auto ApplyOn = LReversal->getApplyOn();
      if (ApplyOn.empty()) {
        // Apply to the following loop
      } else {
        // Apply on the loop with that name
      }

      addTransformation(LoopTransformation::createReversal(ApplyOn));
      continue;
    }

    if (auto LTiling = dyn_cast<LoopTilingAttr>(Attr)) {
      SmallVector<int64_t, 4> TileSizes;
      for (auto TileSizeExpr : LTiling->tileSizes()) {
        llvm::APSInt ValueAPS = TileSizeExpr->EvaluateKnownConstInt(Ctx);
        auto ValueInt = ValueAPS.getSExtValue();
        TileSizes.push_back(ValueInt);
      }

      addTransformation(LoopTransformation::createTiling(
          makeArrayRef(LTiling->applyOn_begin(), LTiling->applyOn_size()),
          TileSizes,
          makeArrayRef(LTiling->pitIds_begin(), LTiling->pitIds_size()),
          makeArrayRef(LTiling->tileIds_begin(), LTiling->tileIds_size())));
      continue;
    }

    if (auto LInterchange = dyn_cast<LoopInterchangeAttr>(Attr)) {
      addTransformation(LoopTransformation::createInterchange(
          makeArrayRef(LInterchange->applyOn_begin(),
                       LInterchange->applyOn_size()),
          makeArrayRef(LInterchange->permutation_begin(),
                       LInterchange->permutation_size())));
      continue;
    }

    if (auto Pack = dyn_cast<PackAttr>(Attr)) {
      addTransformation(LoopTransformation::createPack(
          Pack->getApplyOn(), cast<DeclRefExpr>(Pack->getArray()),
          Pack->getOnHeap()));
      continue;
    }

    if (auto Unrolling = dyn_cast<LoopUnrollingAttr>(Attr)) {
      auto Fac = Unrolling->getFactor();
      int64_t FactorInt = -1;
      if (Fac) {
        llvm::APSInt FactorAPS = Fac->EvaluateKnownConstInt(Ctx);
        FactorInt = FactorAPS.getSExtValue();
      }
      addTransformation(LoopTransformation::createUnrolling(
          Unrolling->getApplyOn(), FactorInt, Unrolling->getFull()));
      continue;
    }

	if (auto ThreadParallel = dyn_cast<LoopParallelizeThreadAttr>(Attr)) {
		addTransformation(LoopTransformation::createThreadParallel(			ThreadParallel->getApplyOn()));
		continue;
	}

    const LoopHintAttr *LH = dyn_cast<LoopHintAttr>(Attr);
    const OpenCLUnrollHintAttr *OpenCLHint =
        dyn_cast<OpenCLUnrollHintAttr>(Attr);

    // Skip non loop hint attributes
    if (!LH && !OpenCLHint) {
      continue;
    }

    LoopHintAttr::OptionType Option = LoopHintAttr::Unroll;
    LoopHintAttr::LoopHintState State = LoopHintAttr::Disable;
    unsigned ValueInt = 1;
    // Translate opencl_unroll_hint attribute argument to
    // equivalent LoopHintAttr enums.
    // OpenCL v2.0 s6.11.5:
    // 0 - full unroll (no argument).
    // 1 - disable unroll.
    // other positive integer n - unroll by n.
    if (OpenCLHint) {
      ValueInt = OpenCLHint->getUnrollHint();
      if (ValueInt == 0) {
        State = LoopHintAttr::Full;
      } else if (ValueInt != 1) {
        Option = LoopHintAttr::UnrollCount;
        State = LoopHintAttr::Numeric;
      }
    } else if (LH) {
      auto *ValueExpr = LH->getValue();
      if (ValueExpr) {
        llvm::APSInt ValueAPS = ValueExpr->EvaluateKnownConstInt(Ctx);
        ValueInt = ValueAPS.getSExtValue();
      }

      Option = LH->getOption();
      State = LH->getState();
    }
    switch (State) {
    case LoopHintAttr::Disable:
      switch (Option) {
      case LoopHintAttr::Vectorize:
        // Disable vectorization by specifying a width of 1.
        setVectorizeWidth(1);
        break;
      case LoopHintAttr::Interleave:
        // Disable interleaving by speciyfing a count of 1.
        setInterleaveCount(1);
        break;
      case LoopHintAttr::Unroll:
        setUnrollState(LoopAttributes::Disable);
        break;
      case LoopHintAttr::UnrollAndJam:
        setUnrollAndJamState(LoopAttributes::Disable);
        break;
      case LoopHintAttr::Distribute:
        setDistributeState(false);
        break;
      case LoopHintAttr::PipelineDisabled:
        setPipelineDisabled(true);
        break;
      case LoopHintAttr::UnrollCount:
      case LoopHintAttr::UnrollAndJamCount:
      case LoopHintAttr::VectorizeWidth:
      case LoopHintAttr::InterleaveCount:
      case LoopHintAttr::PipelineInitiationInterval:
        llvm_unreachable("Options cannot be disabled.");
        break;
      }
      break;
    case LoopHintAttr::Enable:
      switch (Option) {
      case LoopHintAttr::Vectorize:
      case LoopHintAttr::Interleave:
        setVectorizeEnable(true);
        break;
      case LoopHintAttr::Unroll:
        setUnrollState(LoopAttributes::Enable);
        break;
      case LoopHintAttr::UnrollAndJam:
        setUnrollAndJamState(LoopAttributes::Enable);
        break;
      case LoopHintAttr::Distribute:
        setDistributeState(true);
        break;
      case LoopHintAttr::UnrollCount:
      case LoopHintAttr::UnrollAndJamCount:
      case LoopHintAttr::VectorizeWidth:
      case LoopHintAttr::InterleaveCount:
      case LoopHintAttr::PipelineDisabled:
      case LoopHintAttr::PipelineInitiationInterval:
        llvm_unreachable("Options cannot enabled.");
        break;
      }
      break;
    case LoopHintAttr::AssumeSafety:
      switch (Option) {
      case LoopHintAttr::Vectorize:
      case LoopHintAttr::Interleave:
        // Apply "llvm.mem.parallel_loop_access" metadata to load/stores.
        setParallel(true);
        setVectorizeEnable(true);
        break;
      case LoopHintAttr::Unroll:
      case LoopHintAttr::UnrollAndJam:
      case LoopHintAttr::UnrollCount:
      case LoopHintAttr::UnrollAndJamCount:
      case LoopHintAttr::VectorizeWidth:
      case LoopHintAttr::InterleaveCount:
      case LoopHintAttr::Distribute:
      case LoopHintAttr::PipelineDisabled:
      case LoopHintAttr::PipelineInitiationInterval:
        llvm_unreachable("Options cannot be used to assume mem safety.");
        break;
      }
      break;
    case LoopHintAttr::Full:
      switch (Option) {
      case LoopHintAttr::Unroll:
        setUnrollState(LoopAttributes::Full);
        break;
      case LoopHintAttr::UnrollAndJam:
        setUnrollAndJamState(LoopAttributes::Full);
        break;
      case LoopHintAttr::Vectorize:
      case LoopHintAttr::Interleave:
      case LoopHintAttr::UnrollCount:
      case LoopHintAttr::UnrollAndJamCount:
      case LoopHintAttr::VectorizeWidth:
      case LoopHintAttr::InterleaveCount:
      case LoopHintAttr::Distribute:
      case LoopHintAttr::PipelineDisabled:
      case LoopHintAttr::PipelineInitiationInterval:
        llvm_unreachable("Options cannot be used with 'full' hint.");
        break;
      }
      break;
    case LoopHintAttr::Numeric:
      switch (Option) {
      case LoopHintAttr::VectorizeWidth:
        setVectorizeWidth(ValueInt);
        break;
      case LoopHintAttr::InterleaveCount:
        setInterleaveCount(ValueInt);
        break;
      case LoopHintAttr::UnrollCount:
        setUnrollCount(ValueInt);
        break;
      case LoopHintAttr::UnrollAndJamCount:
        setUnrollAndJamCount(ValueInt);
        break;
      case LoopHintAttr::PipelineInitiationInterval:
        setPipelineInitiationInterval(ValueInt);
        break;
      case LoopHintAttr::Unroll:
      case LoopHintAttr::UnrollAndJam:
      case LoopHintAttr::Vectorize:
      case LoopHintAttr::Interleave:
      case LoopHintAttr::Distribute:
      case LoopHintAttr::PipelineDisabled:
        llvm_unreachable("Options cannot be assigned a value.");
        break;
      }
      break;
    }
  }

  /// Stage the attributes.
  push(Header, F, CGF, StartLoc, EndLoc);
}

void LoopInfoStack::pop() {
  assert(!Active.empty() && "No active loops to pop");
  Active.pop_back();
}

static bool mayUseArray(AllocaInst *BasePtrAlloca, Value *PtrArg) {
  DenseSet<PHINode *> Closed;
  SmallVector<Value *, 16> Worklist;

  if (auto CurL = dyn_cast<LoadInst>(PtrArg)) {
    Worklist.push_back(CurL->getPointerOperand());
  } else if (auto CurS = dyn_cast<StoreInst>(PtrArg)) {
    Worklist.push_back(CurS->getPointerOperand());
  } else {
    // TODO: Support memcpy, memset, memmove
  }

  while (true) {
    if (Worklist.empty())
      break;
    auto Cur = Worklist.pop_back_val();

    if (auto CurPhi = dyn_cast<PHINode>(Cur)) {
      auto It = Closed.insert(CurPhi);
      if (!It.second)
        continue;
    }

    if (auto CurAlloca = dyn_cast<AllocaInst>(Cur)) {
      continue;
    }

    auto Ty = Cur->getType();
    if (!isa<PointerType>(Ty))
      continue;

#if 0
        if (auto CurGEP = dyn_cast< GetElementPtrInst>(Cur)) {
            // Fallback instead into the llvm::Operator case to also traverse indices?
            Worklist.push_back(CurGEP->getPointerOperand());
            continue;
        }
#endif

    if (auto CurL = dyn_cast<LoadInst>(Cur)) {
      if (CurL->getPointerOperand() == BasePtrAlloca)
        return true;
      continue;
    }

    if (auto CurInst = dyn_cast<Operator>(Cur)) {
      for (auto &Op : CurInst->operands()) {
        Worklist.push_back(Op.get());
      }
      continue;
    }
  }

  return false;
}

static void addArrayTransformUse(const LoopTransformation &Trans,
                                 Instruction *MemAcc) {
  auto AccessMD = MemAcc->getMetadata("llvm.access");
  if (!AccessMD) {
    AccessMD = MDNode::getDistinct(MemAcc->getContext(), {});
    MemAcc->setMetadata("llvm.access", AccessMD);
  }
  assert(AccessMD->isDistinct() && AccessMD->getNumOperands() == 0);

  auto PackMD = Trans.TransformMD;
  auto ListMD = cast<MDNode>(PackMD->getOperand(2).get());

  SmallVector<Metadata *, 8> NewList;
  NewList.reserve(ListMD->getNumOperands() + 1);
  NewList.append(ListMD->op_begin(), ListMD->op_end());
  NewList.push_back(AccessMD);

  auto NewListMD = MDNode::get(MemAcc->getContext(), NewList);
  PackMD->replaceOperandWith(2, NewListMD);
}

void LoopInfoStack::InsertHelper(Instruction *I) const {
  if (I->mayReadOrWriteMemory()) {
    SmallVector<Metadata *, 4> AccessGroups;
    for (const LoopInfo &AL : Active) {
      // Here we assume that every loop that has an access group is parallel.
      if (MDNode *Group = AL.getAccessGroup())
        AccessGroups.push_back(Group);
    }
    MDNode *UnionMD = nullptr;
    if (AccessGroups.size() == 1)
      UnionMD = cast<MDNode>(AccessGroups[0]);
    else if (AccessGroups.size() >= 2)
      UnionMD = MDNode::get(I->getContext(), AccessGroups);
    I->setMetadata("llvm.access.group", UnionMD);
  }

  if (!hasInfo())
    return;

  const LoopInfo &L = getInfo();
  if (!L.getLoopID())
    return;

  if (I->isTerminator()) {
    for (BasicBlock *Succ : successors(I))
      if (Succ == L.getHeader()) {
        I->setMetadata(llvm::LLVMContext::MD_loop, L.getLoopID());
        break;
      }
    return;
  }

  if (I->mayReadOrWriteMemory()) {
    SmallVector<Metadata *, 2> ParallelLoopIDs;
    for (const LoopInfo &AL : Active) {

      for (auto &Trans : AL.getAttributes().TransformationStack) {
        if (Trans.Kind == LoopTransformation::Pack) {
          auto BasePtr = Trans.ArrayBasePtr;
          auto MNode = Trans.TransformMD;
          assert(BasePtr && MNode);

          if (mayUseArray(BasePtr, I))
            addArrayTransformUse(Trans, I);
        }
      }
    }

  }

}
