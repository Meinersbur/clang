#ifndef LLVM_CLANG_AST_STMTMOLLY_H
#define LLVM_CLANG_AST_STMTMOLLY_H

#ifndef MOLLY
#error Must define MOLLY
#endif

#include "clang/AST/Stmt.h"

namespace clang {

  class MollyWhereDirective : public Stmt {
    //ASTContext *Context;
    Stmt *associatedStmt;
    std::string islstr;

  private:
    MollyWhereDirective(ASTContext &C, Stmt *AssociatedStmt, StringRef islstr) : Stmt(Stmt::MollyWhereDirectiveClass), associatedStmt(AssociatedStmt), islstr(islstr) {
    }

  public:
    static MollyWhereDirective *Create(ASTContext &C, Stmt *AssociatedStmt, StringRef islstr) {
      return new (C) MollyWhereDirective(C, AssociatedStmt, islstr);
    }

    const Stmt *getAssociatedStmt() const {
      return associatedStmt;
    }

    Stmt *getAssociatedStmt() {
      return associatedStmt;
    }

    StringRef getIslStr() const {
      return islstr;
    }

#pragma region Required to implement clang::Stmt

    static bool classof(const Stmt *S) {
      return S->getStmtClass() == MollyWhereDirectiveClass;
    }
    static bool classof(const MollyWhereDirective *S) { 
      assert(S->getStmtClass() == MollyWhereDirectiveClass);
      return true;
    }

    child_range children() {
      return child_range(
        reinterpret_cast<Stmt **>(&associatedStmt),
        reinterpret_cast<Stmt **>(&associatedStmt) + 1);
    }

    SourceLocation getLocStart() const { return SourceLocation(); }
    SourceLocation getLocEnd() const { return SourceLocation(); }

#pragma endregion

  }; // class MollyWhereDirective

} // namespace clang
#endif /* LLVM_CLANG_AST_STMTMOLLY_H */
