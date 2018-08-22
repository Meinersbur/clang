// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -ast-print %s | FileCheck --check-prefix=PRINT --match-full-lines %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -disable-llvm-passes -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -mllvm -polly-use-llvm-names -mllvm -debug-only=polly-ast -o /dev/null %s 2>&1 > /dev/null | FileCheck --check-prefix=AST %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -mllvm -polly-use-llvm-names -o - %s | FileCheck --check-prefix=TRANS %s
// RUN: %clang -DMAIN -std=c99 -O3 -mllvm -polly -mllvm -polly-process-unprofitable %s -o %t_pragma_pack%exeext
// RUN: %t_pragma_pack%exeext | FileCheck --check-prefix=RESULT %s

__attribute__((noinline))
void pragma_pack(double C[256][128], double A[128][256]) {
  for (int i = 0; i < 256; i += 1)
    #pragma clang loop pack array(A)
    for (int j = 0; j < 128; j += 1)
      C[i][j] = A[j][i] + i + j;
}

#ifdef MAIN
#include <stdio.h>
#include <string.h>
int main() {
  double C[256][128];
  double A[128][256];
  memset(A, 0, sizeof(A));
  A[1][2] = 42;
  pragma_pack(C,A);
  printf("(%0.0f)\n", C[2][1]);
  return 0;
}
#endif


// PRINT-LABEL: void pragma_pack(double C[256][128], double A[128][256]) __attribute__((noinline)) {
// PRINT-NEXT:     for (int i = 0; i < 256; i += 1)
// PRINT-NEXT: #pragma clang loop pack array(A)
// PRINT-NEXT:         for (int j = 0; j < 128; j += 1)
// PRINT-NEXT:             C[i][j] = A[j][i] + i + j;
// PRINT-NEXT: }


// IR-LABEL: define dso_local void @pragma_pack([128 x double]* %C, [256 x double]* %A) #0 !looptransform !2 {
//
// IR:         %A.addr = alloca [256 x double]*, align 8
// IR:         load double, double* %arrayidx5, align 8, !llvm.access !6
//
// IR:       !2 = !{!3}
// IR:       !3 = !{!"llvm.data.pack", !4, !5}
// IR:       !4 = distinct !{!4}
// IR:       !5 = !{!6}
// IR:       !6 = distinct !{}


// AST:     for (int c0 = 0; c0 <= 255; c0 += 1) {
// AST:      {
// AST-NEXT:        for (int c2 = 0; c2 <= 127; c2 += 1)
// AST-NEXT:           CopyStmt_0(c0, c2, 0);
// AST-NEXT:         for (int c1 = 0; c1 <= 127; c1 += 1)
// AST-NEXT:           Stmt_for_body4(c0, c1);
// AST-NEXT:         for (int c2 = 0; c2 <= 127; c2 += 1)
// AST-NEXT:           CopyStmt_1(c0, c2, 0);
// AST-NEXT:       }
// AST-NEXT:     }


// TRANS-LABEL: @pragma_pack
// TRANS: Packed_MemRef_


// RESULT: (45)
