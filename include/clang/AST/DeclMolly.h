#ifndef LLVM_CLANG_AST_DECLMOLLY_H
#define LLVM_CLANG_AST_DECLMOLLY_H

namespace clang {
  class MollyTransfomClauses {

  public:
    class MollyTransformClause {
      std::string islstr;
      unsigned distdims;
    public:
      /* implicit */ MollyTransformClause(llvm::StringRef islstr, unsigned distdims) : islstr(islstr), distdims(distdims) {      }

      StringRef getIslStr() { return islstr; }
      unsigned getClusterDims() { return distdims; } 
    }; // MollyTransformClause

    typedef std::vector<MollyTransformClause> ClausesListTy;

  private:
    ClausesListTy clauses;

  public:
    void addTransform(llvm::StringRef islstr, unsigned distdims) {
      clauses.emplace_back(islstr, distdims);
    }

    ClausesListTy &getClauses() {
      return clauses;
    }

  }; // class MollyTransfomClauses
} // namespace clang

#endif /* LLVM_CLANG_AST_DECLMOLLY_H */
