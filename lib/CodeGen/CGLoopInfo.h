//===---- CGLoopInfo.h - LLVM CodeGen for loop metadata -*- C++ -*---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is the internal state used for llvm translation for loop statement
// metadata.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_CODEGEN_CGLOOPINFO_H
#define LLVM_CLANG_LIB_CODEGEN_CGLOOPINFO_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Compiler.h"
#include "clang/AST/Expr.h"

namespace llvm {
class BasicBlock;
class Instruction;
class MDNode;
} // end namespace llvm

namespace clang {
class Attr;
class ASTContext;
namespace CodeGen {
    class CodeGenFunction;

struct LoopTransformation {
  enum TransformKind { Reversal, Tiling, Interchange ,Pack};
  TransformKind Kind;

  // TODO: If ApplyOn is set, should not appear in the transformation stack
  // TODO: Make a union or class hierachy
  llvm::SmallVector<llvm::StringRef, 4> ApplyOns;
  llvm::SmallVector<int64_t, 4> TileSizes;
   llvm::SmallVector<llvm::StringRef, 4> Permutation;
clang::DeclRefExpr*  Array;

  llvm::StringRef getApplyOn() const {
    assert(ApplyOns.size() <= 1);
    if (ApplyOns.empty())
        return {};
    return ApplyOns[0];
  }

  static LoopTransformation
  createReversal(llvm::StringRef ApplyOn = llvm::StringRef()) {
    LoopTransformation Result;
    Result.Kind = Reversal;
    Result.ApplyOns.push_back(ApplyOn);
    return Result;
  }

  static LoopTransformation
  createTiling(llvm::ArrayRef<llvm::StringRef> ApplyOns,
               llvm::ArrayRef<int64_t> TileSizes) {
    LoopTransformation Result;
    Result.Kind = Tiling;
    // TODO: list-intialize
    for (auto ApplyOn : ApplyOns)
      Result.ApplyOns.push_back(ApplyOn);
    for (auto TileSize : TileSizes)
      Result.TileSizes.push_back(TileSize);
    return Result;
  }

    static LoopTransformation
  createInterchange (llvm::ArrayRef<llvm::StringRef> ApplyOns, llvm::ArrayRef<llvm::StringRef> Permutation) {
    LoopTransformation Result;
    Result.Kind = Interchange;

    for (auto ApplyOn : ApplyOns)
      Result.ApplyOns.push_back(ApplyOn);
    for (auto P : Permutation)
      Result.Permutation.push_back(P);
    return Result;
  }

        static LoopTransformation
  createPack (llvm::StringRef ApplyOn, clang::DeclRefExpr*  Array) {
    LoopTransformation Result;
    Result.Kind = Pack;
      Result.ApplyOns.push_back(ApplyOn);
      Result.Array = Array;
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

  /// Value for llvm.loop.vectorize.width metadata.
  unsigned VectorizeWidth;

  /// Value for llvm.loop.interleave.count metadata.
  unsigned InterleaveCount;

  /// llvm.unroll.
  unsigned UnrollCount;

  /// Value for llvm.loop.distribute.enable metadata.
  LVEnableState DistributeEnable;

  llvm::StringRef LoopId;
  std::vector<LoopTransformation> TransformationStack;
};

/// Information used when generating a structured loop.
class LoopInfo {
public:
  /// Construct a new LoopInfo for the loop with entry Header.
  LoopInfo(llvm::BasicBlock *Header, llvm::Function *F, clang::CodeGen::CodeGenFunction *CGF,
           const LoopAttributes &Attrs, const llvm::DebugLoc &StartLoc,
           const llvm::DebugLoc &EndLoc);

  /// Get the loop id metadata for this loop.
  llvm::MDNode *getLoopID() const { return LoopID; }

  /// Get the header block of this loop.
  llvm::BasicBlock *getHeader() const { return Header; }

  /// Get the set of attributes active for this loop.
  const LoopAttributes &getAttributes() const { return Attrs; }

private:
  /// Loop ID metadata.
  llvm::MDNode *LoopID;
  /// Header block of this loop.
  llvm::BasicBlock *Header;
  /// The attributes for this loop.
  LoopAttributes Attrs;
};

/// A stack of loop information corresponding to loop nesting levels.
/// This stack can be used to prepare attributes which are applied when a loop
/// is emitted.
class LoopInfoStack {
  LoopInfoStack(const LoopInfoStack &) = delete;
  void operator=(const LoopInfoStack &) = delete;

public:
  LoopInfoStack() {}

  /// Begin a new structured loop. The set of staged attributes will be
  /// applied to the loop and then cleared.
  void push(llvm::BasicBlock *Header, llvm::Function *F,clang::CodeGen::CodeGenFunction *CGF,
            const llvm::DebugLoc &StartLoc, const llvm::DebugLoc &EndLoc);

  /// Begin a new structured loop. Stage attributes from the Attrs list.
  /// The staged attributes are applied to the loop and then cleared.
  void push(llvm::BasicBlock *Header, llvm::Function *F,clang::CodeGen::CodeGenFunction *CGF, clang::ASTContext &Ctx,
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

  /// Set the vectorize width for the next loop pushed.
  void setVectorizeWidth(unsigned W) { StagedAttrs.VectorizeWidth = W; }

  /// Set the interleave count for the next loop pushed.
  void setInterleaveCount(unsigned C) { StagedAttrs.InterleaveCount = C; }

  /// Set the unroll count for the next loop pushed.
  void setUnrollCount(unsigned C) { StagedAttrs.UnrollCount = C; }

  void setLoopId(llvm::StringRef Id) { StagedAttrs.LoopId = Id; }

  void addTransformation(LoopTransformation Transform) {
    StagedAttrs.TransformationStack.push_back(Transform);
  }

private:
  /// Returns true if there is LoopInfo on the stack.
  bool hasInfo() const { return !Active.empty(); }
  /// Return the LoopInfo for the current loop. HasInfo should be called
  /// first to ensure LoopInfo is present.
  const LoopInfo &getInfo() const { return Active.back(); }
  /// The set of attributes that will be applied to the next pushed loop.
  LoopAttributes StagedAttrs;
  /// Stack of active loops.
  llvm::SmallVector<LoopInfo, 4> Active;
};

} // end namespace CodeGen
} // end namespace clang

#endif
