#include "ParsePragma.h"

#include "clang/Lex/Token.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Sema/Sema.h"
#include <string>

using namespace clang;
using namespace std;


void PragmaMollyHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &MollyTok) {

  Token OpTok;
  PP.Lex(OpTok);
  IdentifierInfo *OpII = OpTok.getIdentifierInfo();
  if (OpII->isStr("transform")) {

    Token LParTok;
    PP.Lex(LParTok);
    if (LParTok.isNot(tok::l_paren)) {
      llvm_unreachable("lparen expected");
    }

    Token islstrTok;
    string islstr;
    PP.LexStringLiteral(islstrTok, islstr, nullptr, true);

    Token commaTok;
    PP.Lex(commaTok);
    if (LParTok.isNot(tok::comma)) {
      llvm_unreachable("comma expected");
    }

    Token distlvlTok;
    PP.Lex(distlvlTok);
    if (distlvlTok.isNot(tok::numeric_constant)) {
      llvm_unreachable("int constant expected");
    }
    //TODO: is there a more direct way to just parse an integer literal?
    llvm::APSInt Val;
    auto retval = Actions.ActOnNumericConstant(islstrTok).get()->isIntegerConstantExpr(Val, Actions.getASTContext());
    assert(retval);
    auto dstlvl = Val.getZExtValue();

    Token rparenTok;
    PP.Lex(rparenTok);
    if (LParTok.isNot(tok::r_paren)) {
      llvm_unreachable("rparen expected");
    }

    //Actions.ActOn

  } else {
    llvm_unreachable("operation keyword expected");
  }

  Token eodTok;
  PP.Lex(eodTok);

  if (eodTok.isNot(tok::eod)) {
    PP.Diag(eodTok.getLocation(), diag::err_pragma_detect_mismatch_malformed);
    return;
  }
}
