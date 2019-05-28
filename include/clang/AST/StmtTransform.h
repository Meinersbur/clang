#ifndef LLVM_CLANG_AST_STMTTRANSFROM_H
#define LLVM_CLANG_AST_STMTTRANSFROM_H

#include "clang/AST/Stmt.h"

namespace clang {

	class TransformExecutableDirective : public Stmt {
	};

	class TransformDeclarativeDirective : public Stmt {
	};


}

#endif /* LLVM_CLANG_AST_STMTTRANSFROM_H */
