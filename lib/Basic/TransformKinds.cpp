

#include "clang/Basic/TransformKinds.h"
#include "llvm/ADT/StringSwitch.h"


using namespace clang;

TransformDirectiveKind clang::getTransformDirectiveKind(llvm::StringRef Str) {
	return llvm::StringSwitch<TransformDirectiveKind>(Str)
#define TRANSFORM_DIRECTIVE(Name) .Case(#Name, TransformDirectiveKind:: ##Name)
#include "clang/Basic/TransformKinds.def"
		.Default(TransformDirectiveKind::unknown);
}


