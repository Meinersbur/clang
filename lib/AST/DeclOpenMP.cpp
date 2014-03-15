//===--- DeclOpenMP.cpp - Declaration OpenMP AST Node Implementation ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// \brief This file implements OMPThreadPrivateDecl, OMPDeclareReduction
/// classes.
///
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclOpenMP.h"
#include "clang/AST/Expr.h"

using namespace clang;

//===----------------------------------------------------------------------===//
// OMPThreadPrivateDecl Implementation.
//===----------------------------------------------------------------------===//

void OMPThreadPrivateDecl::anchor() { }

OMPThreadPrivateDecl *OMPThreadPrivateDecl::Create(ASTContext &C,
                                                   DeclContext *DC,
                                                   SourceLocation L,
                                                   ArrayRef<Expr *> VL) {
  OMPThreadPrivateDecl *D = new (C, DC, VL.size() * sizeof(Expr *))
      OMPThreadPrivateDecl(OMPThreadPrivate, DC, L);
  D->NumVars = VL.size();
  D->setVars(VL);
  return D;
}

OMPThreadPrivateDecl *OMPThreadPrivateDecl::CreateDeserialized(ASTContext &C,
                                                               unsigned ID,
                                                               unsigned N) {
  OMPThreadPrivateDecl *D = new (C, ID, N * sizeof(Expr *))
      OMPThreadPrivateDecl(OMPThreadPrivate, 0, SourceLocation());
  D->NumVars = N;
  return D;
}

void OMPThreadPrivateDecl::setVars(ArrayRef<Expr *> VL) {
  assert(VL.size() == NumVars &&
         "Number of variables is not the same as the preallocated buffer");
  Expr **Vars = reinterpret_cast<Expr **>(this + 1);
  std::copy(VL.begin(), VL.end(), Vars);
}

//===----------------------------------------------------------------------===//
// OMPDeclareReductionDecl Implementation.
//===----------------------------------------------------------------------===//

void OMPDeclareReductionDecl::anchor() { }

unsigned OMPDeclareReductionDecl::getFirstElementOffset() {
  unsigned Size = sizeof(OMPDeclareReductionDecl);
  // Realign
  Size =
    llvm::RoundUpToAlignment(Size,
                             llvm::alignOf<OMPDeclareReductionDecl::ReductionData>());
  return Size;
}

OMPDeclareReductionDecl *
OMPDeclareReductionDecl::Create(ASTContext &C, DeclContext *DC,
                                SourceLocation L,
                                DeclarationName Name,
                                unsigned N) {
  unsigned Size = getFirstElementOffset() +
                  N * sizeof(OMPDeclareReductionDecl::ReductionData);
  OMPDeclareReductionDecl *D =
    new (C, DC, Size) OMPDeclareReductionDecl(OMPDeclareReduction,
                                              DC, L, Name);
  D->NumTypes = N;
  return D;
}

OMPDeclareReductionDecl *OMPDeclareReductionDecl::CreateDeserialized(ASTContext &C,
                                                                     unsigned ID,
                                                                     unsigned N) {
  unsigned Size = getFirstElementOffset() +
                  N * sizeof(OMPDeclareReductionDecl::ReductionData);

  OMPDeclareReductionDecl *D =
    new (C, ID, Size) OMPDeclareReductionDecl(OMPDeclareReduction,
                                              0, SourceLocation(),
                                              DeclarationName());
  D->NumTypes = N;
  return D;
}

void OMPDeclareReductionDecl::setData(
                           ArrayRef<OMPDeclareReductionDecl::ReductionData> RD) {
  assert(RD.size() == NumTypes &&
         "Number of inits is not the same as the preallocated buffer");
  unsigned Size = getFirstElementOffset();
  OMPDeclareReductionDecl::ReductionData *Data =
    reinterpret_cast<OMPDeclareReductionDecl::ReductionData *>(
                                               reinterpret_cast<char *>(this) + Size);
  for (unsigned i = 0; i < NumTypes; ++i)
    Data[i] = RD[i];
}

