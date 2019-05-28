#ifndef LLVM_CLANG_BASIC_TRANSFORMKINDS_H
#define LLVM_CLANG_BASIC_TRANSFORMKINDS_H

#include "llvm/ADT/StringRef.h"

namespace clang {

	/// OpenMP directives.
	enum class TransformDirectiveKind {
#define TRANSFORM_DIRECTIVE(Name) \
  Name,
#include "clang/Basic/TransformKinds.def"
		unknown
	};


	TransformDirectiveKind getTransformDirectiveKind(llvm::StringRef Str);

}

#endif /* LLVM_CLANG_BASIC_TRANSFORMKINDS_H */
