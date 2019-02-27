//===---- CGLoopInfo.h - LLVM CodeGen for loop metadata -*- C++ -*---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This is the internal state used for llvm translation for loop statement
// metadata.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_CODEGEN_CGLOOPINFO_H
#define LLVM_CLANG_LIB_CODEGEN_CGLOOPINFO_H

#include "clang/AST/Expr.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
class BasicBlock;
class Instruction;
class MDNode;
class AllocaInst;
} // end namespace llvm

namespace clang {
class Attr;
class ASTContext;
namespace CodeGen {
class CodeGenFunction;
class LoopInfoStack;
class VirtualLoopInfo;

// FIXME: Don't need this, why not use the attribute instead?
struct LoopTransformation {
  enum TransformKind {
	  Id,
    Reversal,
    Tiling,
    Interchange,
    Pack,
    Unrolling,
    ThreadParallel
  };
  TransformKind Kind;

  // TODO: If ApplyOn is set, should not appear in the transformation stack
  // TODO: Make a union or class hierachy
  llvm::SmallVector<llvm::StringRef, 4> ApplyOns;

  llvm::StringRef Name;
  llvm::StringRef FollowupName;
  llvm::SmallVector<int64_t, 4> TileSizes;
  llvm::SmallVector<llvm::StringRef, 4> TileFloorIds;
  llvm::SmallVector<llvm::StringRef, 4> TileTileIds;

  llvm::SmallVector<llvm::StringRef, 4> Permutation;
  clang::DeclRefExpr *Array;
  bool OnHeap = false;

  int64_t Factor = -1;
  bool Full = false;

  // FIXME: This is set later at CGLoopInfo and forces the emission of this
  // pointer before its first use/even if it is not used. Maybe better hook into
  // CGF->EmitLValue when the array pointer is emited.
  //llvm::AllocaInst *ArrayBasePtr = nullptr;

  llvm::MDNode *TransformMD = nullptr;

  llvm::StringRef getApplyOn() const {
    assert(ApplyOns.size() <= 1);
    if (ApplyOns.empty())
      return {};
    return ApplyOns[0];
  }

  static LoopTransformation createId(llvm::StringRef Name) {
	  LoopTransformation Result;
	  Result.Kind = Id;
	  Result.Name = Name;
	  return Result;
  }

  static LoopTransformation
  createReversal(llvm::StringRef ApplyOn , llvm::StringRef ReversedId ) {
    LoopTransformation Result;
    Result.Kind = Reversal;
	if (!ApplyOn.empty())
		Result.ApplyOns.push_back(ApplyOn);
	Result.FollowupName=ReversedId;
    return Result;
  }

  static LoopTransformation
  createTiling(llvm::ArrayRef<llvm::StringRef> ApplyOns,
               llvm::ArrayRef<int64_t> TileSizes,
               llvm::ArrayRef<StringRef> PitIds,
               llvm::ArrayRef<StringRef> TileIds) {
    LoopTransformation Result;
    Result.Kind = Tiling;
    // TODO: list-intialize
    for (auto ApplyOn : ApplyOns)
      Result.ApplyOns.push_back(ApplyOn);
    for (auto TileSize : TileSizes)
      Result.TileSizes.push_back(TileSize);
    for (auto PitId : PitIds)
      Result.TileFloorIds.push_back(PitId);
    for (auto TileId : TileIds)
      Result.TileTileIds.push_back(TileId);

    return Result;
  }

  static LoopTransformation
  createInterchange(llvm::ArrayRef<llvm::StringRef> ApplyOns,
                    llvm::ArrayRef<llvm::StringRef> Permutation) {
    LoopTransformation Result;
    Result.Kind = Interchange;

    for (auto ApplyOn : ApplyOns)
      Result.ApplyOns.push_back(ApplyOn);
    for (auto P : Permutation)
      Result.Permutation.push_back(P);
    return Result;
  }

  static LoopTransformation createPack(llvm::StringRef ApplyOn,
                                       clang::DeclRefExpr *Array, bool OnHeap) {
    LoopTransformation Result;
    Result.Kind = Pack;
	if (!ApplyOn.empty())
    Result.ApplyOns.push_back(ApplyOn);
    Result.Array = Array;
    Result.OnHeap = OnHeap;
    return Result;
  }

  static LoopTransformation createUnrolling(llvm::StringRef ApplyOn,
                                            int64_t Factor, bool Full) {
    LoopTransformation Result;
    Result.Kind = Unrolling;
	if (!ApplyOn.empty())
    Result.ApplyOns.push_back(ApplyOn);
    Result.Factor = Factor;
    Result.Full = Full;
    return Result;
  }

  static LoopTransformation createThreadParallel(llvm::StringRef ApplyOn) {
    LoopTransformation Result;
    Result.Kind = ThreadParallel;
	if (!ApplyOn.empty())
    Result.ApplyOns.push_back(ApplyOn);
    return Result;
  }
};

/// Attributes that may be specified on loops.
struct LoopAttributes {
  explicit LoopAttributes(bool IsParallel = false);
  void clear();

  /// Generate llvm.loop.parallel metadata for loads and stores.
  bool IsParallel;

  /// State of loop vectorization or unrolling.
  enum LVEnableState { Unspecified, Enable, Disable, Full };

  /// Value for llvm.loop.vectorize.enable metadata.
  LVEnableState VectorizeEnable;

  /// Value for llvm.loop.unroll.* metadata (enable, disable, or full).
  LVEnableState UnrollEnable;

  /// Value for llvm.loop.unroll_and_jam.* metadata (enable, disable, or full).
  LVEnableState UnrollAndJamEnable;

  /// Value for llvm.loop.vectorize.width metadata.
  unsigned VectorizeWidth;

  /// Value for llvm.loop.interleave.count metadata.
  unsigned InterleaveCount;

  /// llvm.unroll.
  unsigned UnrollCount;

  /// llvm.unroll.
  unsigned UnrollAndJamCount;

  /// Value for llvm.loop.distribute.enable metadata.
  LVEnableState DistributeEnable;

  /// Value for llvm.loop.pipeline.disable metadata.
  bool PipelineDisabled;

  /// Value for llvm.loop.pipeline.iicount metadata.
  unsigned PipelineInitiationInterval;

  llvm::StringRef LoopId;
  std::vector<LoopTransformation> TransformationStack;
};

/// Information used when generating a structured loop.
class LoopInfo {
public:
  /// Construct a new LoopInfo for the loop with entry Header.
  LoopInfo(llvm::BasicBlock *Header, llvm::Function *F,
           clang::CodeGen::CodeGenFunction *CGF,  LoopAttributes &Attrs,
           const llvm::DebugLoc &StartLoc, const llvm::DebugLoc &EndLoc,
           LoopInfo *Parent);

  /// Get the loop id metadata for this loop.
  llvm::MDNode *getLoopID() const;

  /// Get the header block of this loop.
  llvm::BasicBlock *getHeader() const { return Header; }

  /// Get the set of attributes active for this loop.
  const LoopAttributes &getAttributes() const { return Attrs; }

  /// Return this loop's access group or nullptr if it does not have one.
  llvm::MDNode *getAccessGroup() const { return AccGroup; }

  void addSubloop(LoopInfo *Sub){Subloops.push_back(Sub); }

  void afterLoop(LoopInfoStack &LIS);

  /// Create the loop's metadata. Must be called after its nested loops have
  /// been processed.
  void finish(LoopInfoStack &LIS);

private:
	VirtualLoopInfo *VInfo=nullptr;

  /// Loop ID metadata.
 // llvm::TempMDTuple TempLoopID;

  /// Header block of this loop.
  llvm::BasicBlock *Header;
  /// The attributes for this loop.
  LoopAttributes Attrs;
  /// The access group for memory accesses parallel to this loop.
  llvm::MDNode *AccGroup = nullptr;
  /// Start location of this loop.
  llvm::DebugLoc StartLoc;
  /// End location of this loop.
  llvm::DebugLoc EndLoc;
  /// The next outer loop, or nullptr if this is the outermost loop.
  LoopInfo *Parent;
  SmallVector<LoopInfo*,4> Subloops;

  clang::CodeGen::CodeGenFunction *CGF;
  bool InTransformation = false;

  llvm::TempMDTuple TempLoopID;
};



class VirtualLoopInfo {
public:
	VirtualLoopInfo();
	VirtualLoopInfo(llvm::StringRef Name);

	void markNondefault() {
		IsDefault = false;
	}
	void markDisableHeuristic() {
		DisableHeuristic = true;
	}

	// llvm::MDNode *getLoopID() {return TempLoopID.get();}

	void addAttribute(llvm::Metadata *Node) { 
		Attributes.push_back(Node);
	}

	void addTransformMD(llvm::Metadata *Node) { 
		Transforms.push_back(Node);
	}

	void addFollowup(const char *FollowupAttributeName, VirtualLoopInfo* Followup){
		Followups.push_back({FollowupAttributeName, Followup});
	}

#if 0
	void addOriginal(VirtualLoopInfo* VInfo) {
		BasedOn.insert(VInfo);
	}

	void trackArray(llvm::AllocaInst *ArrayPtr ) {
		TrackArrays.insert (ArrayPtr);
		for (auto X: BasedOn)
			X->trackArray(ArrayPtr);
	}
#endif

llvm::	MDNode *makeLoopID(llvm::LLVMContext &Ctx);

//private:
llvm::StringRef Name;

bool IsDefault = true;
bool DisableHeuristic = false;

	llvm::SmallVector<llvm::Metadata*,8> Attributes; // inheritable
	llvm::SmallVector<llvm::Metadata*,4> Transforms;

	llvm::SmallVector<std::pair<const char*, VirtualLoopInfo*> ,4> Followups;

#if 0
	llvm::SmallPtrSet<llvm::AllocaInst *, 4>  TrackArrays;
	llvm::SmallPtrSet <VirtualLoopInfo*, 1> BasedOn;
	//llvm::AllocaInst *ArrayBasePtr = nullptr;
#endif
};


/// A stack of loop information corresponding to loop nesting levels.
/// This stack can be used to prepare attributes which are applied when a loop
/// is emitted.
class LoopInfoStack {
  LoopInfoStack(const LoopInfoStack &) = delete;
  void operator=(const LoopInfoStack &) = delete;

public:
  LoopInfoStack(llvm::LLVMContext &Ctx, CodeGenFunction &CGF) : Ctx(Ctx), CGF(CGF) {}

  /// Begin a new structured loop. The set of staged attributes will be
  /// applied to the loop and then cleared.
  LoopInfo* push(llvm::BasicBlock *Header, llvm::Function *F,
            clang::CodeGen::CodeGenFunction *CGF,
            const llvm::DebugLoc &StartLoc, const llvm::DebugLoc &EndLoc);

  /// Begin a new structured loop. Stage attributes from the Attrs list.
  /// The staged attributes are applied to the loop and then cleared.
  void push(llvm::BasicBlock *Header, llvm::Function *F,
            clang::CodeGen::CodeGenFunction *CGF, clang::ASTContext &Ctx,
            llvm::ArrayRef<const Attr *> Attrs, const llvm::DebugLoc &StartLoc,
            const llvm::DebugLoc &EndLoc);

  /// End the current loop.
  void pop();

  /// Return the top loop id metadata.
  llvm::MDNode *getCurLoopID() const { return getInfo().getLoopID(); }

  /// Return true if the top loop is parallel.
  bool getCurLoopParallel() const {
    return hasInfo() ? getInfo().getAttributes().IsParallel : false;
  }

  /// Function called by the CodeGenFunction when an instruction is
  /// created.
  void InsertHelper(llvm::Instruction *I) const;

  /// Set the next pushed loop as parallel.
  void setParallel(bool Enable = true) { StagedAttrs.IsParallel = Enable; }

  /// Set the next pushed loop 'vectorize.enable'
  void setVectorizeEnable(bool Enable = true) {
    StagedAttrs.VectorizeEnable =
        Enable ? LoopAttributes::Enable : LoopAttributes::Disable;
  }

  /// Set the next pushed loop as a distribution candidate.
  void setDistributeState(bool Enable = true) {
    StagedAttrs.DistributeEnable =
        Enable ? LoopAttributes::Enable : LoopAttributes::Disable;
  }

  /// Set the next pushed loop unroll state.
  void setUnrollState(const LoopAttributes::LVEnableState &State) {
    StagedAttrs.UnrollEnable = State;
  }

  /// Set the next pushed loop unroll_and_jam state.
  void setUnrollAndJamState(const LoopAttributes::LVEnableState &State) {
    StagedAttrs.UnrollAndJamEnable = State;
  }

  /// Set the vectorize width for the next loop pushed.
  void setVectorizeWidth(unsigned W) { StagedAttrs.VectorizeWidth = W; }

  /// Set the interleave count for the next loop pushed.
  void setInterleaveCount(unsigned C) { StagedAttrs.InterleaveCount = C; }

  /// Set the unroll count for the next loop pushed.
  void setUnrollCount(unsigned C) { StagedAttrs.UnrollCount = C; }

  /// \brief Set the unroll count for the next loop pushed.
  void setUnrollAndJamCount(unsigned C) { StagedAttrs.UnrollAndJamCount = C; }

  /// Set the pipeline disabled state.
  void setPipelineDisabled(bool S) { StagedAttrs.PipelineDisabled = S; }

  /// Set the pipeline initiation interval.
  void setPipelineInitiationInterval(unsigned C) {
    StagedAttrs.PipelineInitiationInterval = C;
  }

 // void setLoopId(llvm::StringRef Id) { StagedAttrs.LoopId = Id; }

  void addTransformation(const LoopTransformation& Transform) {
	  if (Transform.ApplyOns.empty())
		 StagedAttrs.TransformationStack.push_back(Transform);
	  else {
		  // TODO: Such pragmas are not necessarily in front of a loop, hence should be treated differently:
		  // Collect all such pragmas in the function beforehand (e.g. in Sema) and treat like PendingTransformations.
		  PendingTransformations.push_back(Transform);
	  }
  }


  VirtualLoopInfo *lookupNamedLoop(StringRef LoopName);


  VirtualLoopInfo * applyTransformation(const LoopTransformation &Transform, VirtualLoopInfo *TopLoopId = nullptr) ;
   VirtualLoopInfo* applyReversal(const LoopTransformation &TheTransform, VirtualLoopInfo *On) ;
   VirtualLoopInfo*  applyTiling(const LoopTransformation &TheTransform,llvm:: ArrayRef< VirtualLoopInfo *>On) ;
   VirtualLoopInfo* applyInterchange(const LoopTransformation &Transform,llvm:: ArrayRef<VirtualLoopInfo *>On) ;
   VirtualLoopInfo* applyUnrolling(const LoopTransformation &Transform,llvm:: ArrayRef<VirtualLoopInfo *>On);
   VirtualLoopInfo* applyPack(const LoopTransformation &Transform,llvm:: ArrayRef<VirtualLoopInfo *>On);

  void finish();


  void invalidateVirtualLoop(VirtualLoopInfo *VLI);


private:
	CodeGenFunction &CGF;

  /// Returns true if there is LoopInfo on the stack.
  bool hasInfo() const { return !Active.empty(); }
  /// Return the LoopInfo for the current loop. HasInfo should be called
  /// first to ensure LoopInfo is present.
  const LoopInfo &getInfo() const { return *Active.back(); }
  /// The set of attributes that will be applied to the next pushed loop.
  LoopAttributes StagedAttrs;
  /// Stack of active loops.
  llvm::SmallVector<LoopInfo*, 4> Active;

  llvm::SmallVector<LoopInfo*, 16> OriginalLoops;

  public:
  llvm::DenseMap<llvm::StringRef, VirtualLoopInfo*> NamedLoopMap;
  std::vector<LoopTransformation> PendingTransformations;
  llvm::LLVMContext &Ctx;

  llvm::SmallVector<std::pair<llvm::AllocaInst*,llvm::MDNode *>,4> AccessesToTrack;
};

} // end namespace CodeGen
} // end namespace clang

#endif
