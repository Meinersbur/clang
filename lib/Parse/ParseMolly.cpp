#include "clang/Lex/Token.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Sema/Sema.h"
#include "clang/AST/DeclMolly.h"
#include "clang/Parse/Parser.h"
#include <string>
#include "clang/Sema/Scope.h"

using namespace clang;
using namespace std;

#ifndef MOLLY
#error Must define MOLLY
#endif

struct PragmaMollyInfo {
  std::string islstr;
  //int clusterdims;

public:
  //~PragmaMollyInfo() {}
  PragmaMollyInfo(StringRef islstr) : islstr(islstr){}
}; // struct PragmaMollyInfo


struct PragmaMollyTransformInfo {
  std::string islstr; 
  //int clusterdims;

public:
  PragmaMollyTransformInfo(StringRef islstr) : islstr(islstr) { }
}; // struct PragmaMollyTransformInfo


void Parser::HandleMollyTransform() {
  assert(Tok.is(tok::annot_pragma_molly_transform));
  auto transformInfo = static_cast<PragmaMollyTransformInfo*>(Tok.getAnnotationValue()); // FIXME: Who has to free this annotation?
  auto islstr = transformInfo->islstr;
  ConsumeToken();

  Actions.ActOnMollyTransformClause(islstr, 0);
}


// Mostly copied from Parser::ParseOpenMPDeclarativeOrExecutableDirective
StmtResult Parser::ParseMollyAnnotation(bool StandAloneAllowed) {
  assert(Tok.is(tok::annot_pragma_molly) && "Not a Molly directive!");
  auto info = static_cast<PragmaMollyInfo*>(Tok.getAnnotationValue()); assert(info);
  Tok.setAnnotationValue(nullptr);
  ConsumeToken();

  bool CreateDirective;
  StmtResult Directive;
  StmtResult AssociatedStmt;
  {
    //ParseScope MollyDirectiveScope(this, Scope::FnScope | Scope::MollyDirectiveScope | Scope::DeclScope); // Flags taken from #pragma omp parallel

    //Actions.ActOnCapturedRegionStart(SourceLocation(), getCurScope(), CR_Default, 1);
    {
      //Sema::CompoundScopeRAII CompoundScope(Actions);
      AssociatedStmt = ParseStatement();
    }

    assert(AssociatedStmt.isUsable());
    if (!AssociatedStmt.isUsable()) {
      //Actions.ActOnCapturedRegionError();
      CreateDirective = false;
    }
    else {
      //AssociatedStmt = Actions.ActOnCapturedRegionEnd(AssociatedStmt.take());
      CreateDirective = AssociatedStmt.isUsable();
    }

    if (CreateDirective) {
      Directive = Actions.ActOnMollyWhereDirective(AssociatedStmt.get(), info->islstr);
    }
  }

  info->~PragmaMollyInfo();
  PP.getPreprocessorAllocator().Deallocate(info);
  return Directive;
}


void PragmaMollyHandler::HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer, Token &MollyTok) {
  Token OpTok;
  PP.Lex(OpTok);
  IdentifierInfo *OpII = OpTok.getIdentifierInfo();
  if (OpII->isStr("transform")) {
    uint64_t dstlvl = 0;

    Token LParTok;
    PP.Lex(LParTok);
    if (LParTok.isNot(tok::l_paren)) {
      llvm_unreachable("lparen expected");
    }

    string islstr;
    Token tok;
    PP.LexStringLiteral(tok, islstr, nullptr, true);

    // Optional comma-part
    if (tok.is(tok::comma)) {
      Token distlvlTok;
      PP.Lex(distlvlTok);
      if (distlvlTok.isNot(tok::numeric_constant)) {
        llvm_unreachable("int constant expected");
      }
      //TODO: is there a more direct way to just parse an integer literal?
      llvm::APSInt Val;
      auto retval = Actions.ActOnNumericConstant(distlvlTok).get()->isIntegerConstantExpr(Val, Actions.getASTContext());
      assert(retval);
      dstlvl = Val.getZExtValue();

      // Process next token
      PP.Lex(tok);
    }

    if (tok.isNot(tok::r_paren)) {
      llvm_unreachable("rparen expected");
    }

    Token eodTok;
    PP.Lex(eodTok);
    if (eodTok.isNot(tok::eod)) {
      llvm_unreachable("end-of-pragma expected");
    }



    auto Info = (PragmaMollyTransformInfo*)PP.getPreprocessorAllocator().Allocate(sizeof(PragmaMollyInfo), llvm::alignOf<PragmaMollyInfo>());
    new (Info)PragmaMollyTransformInfo(islstr);

    Token *Toks = new Token[1]; //(Token*) PP.getPreprocessorAllocator().Allocate(sizeof(Token) * 1, llvm::alignOf<Token>());
    Token &Annot = Toks[0];
    Annot.startToken();
    Annot.setKind(tok::annot_pragma_molly_transform);
    Annot.setAnnotationValue(const_cast<void*>(static_cast<const void*>(Info)));
    PP.EnterTokenStream(Toks, 1, /*DisableMacroExpansion=*/true, /*OwnsTokens=*/true);

    // PragmaPack creates an annotation token that the the parser will handle. Why? Better distinction between Lexer and Parser?
    //   Possible answer: maybe because Parser can check whether it is placed correctly 
    //   Or maybe to support _Pragma
    // OpenMP invokes the parser to parse the next statement itself.
    // Here, we directly instruct Sema to annotate the next variable.
    // I don't know which approach is better
    //Actions.ActOnMollyTransformClause(islstr, dstlvl);

  } else if (OpII->isStr("where")) {
    Token LParTok;
    PP.Lex(LParTok);
    if (LParTok.isNot(tok::l_paren)) {
      llvm_unreachable("lparen expected");
    }

    string islstr;
    Token RParTok;
    PP.LexStringLiteral(RParTok, islstr, nullptr, true);

    if (RParTok.isNot(tok::r_paren)) {
      llvm_unreachable("rparen expected");
    }

    Token eodTok;
    PP.Lex(eodTok);
    if (eodTok.isNot(tok::eod)) {
      llvm_unreachable("end-of-pragma expected");
    }

    PragmaMollyInfo *Info = (PragmaMollyInfo*)PP.getPreprocessorAllocator().Allocate(sizeof(PragmaMollyInfo), llvm::alignOf<PragmaMollyInfo>());
    new (Info)PragmaMollyInfo(islstr);

    Token *Toks = new Token[1]; //(Token*) PP.getPreprocessorAllocator().Allocate(sizeof(Token) * 1, llvm::alignOf<Token>());
    Token &Annot = Toks[0];
    Annot.startToken();
    Annot.setKind(tok::annot_pragma_molly);
    Annot.setAnnotationValue(const_cast<void*>(static_cast<const void*>(Info)));

    PP.EnterTokenStream(Toks, 1, /*DisableMacroExpansion=*/true, /*OwnsTokens=*/true);
  }
  else {
    llvm_unreachable("operation keyword expected");
  }
}
