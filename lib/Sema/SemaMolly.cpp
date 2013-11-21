#include "clang/Sema/Sema.h"
#include "clang/AST/DeclMolly.h"
#include "clang/AST/StmtMolly.h"

using namespace clang;


void Sema::ActOnMollyTransformClause(llvm::StringRef islaff, int parallelLevel) {
  if (!MollyTransform) {
    MollyTransform = new MollyTransfomClauses();
  }

  MollyTransform->addTransform(islaff, parallelLevel);
}


StmtResult Sema::ActOnMollyWhereDirective(Stmt *AStmt, StringRef islstr) {
  return Owned(MollyWhereDirective::Create(Context, AStmt, islstr));
}
