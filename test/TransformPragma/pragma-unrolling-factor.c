// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -ast-print %s | FileCheck --check-prefix=PRINT --match-full-lines %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -disable-llvm-passes -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -mllvm -polly-use-llvm-names -mllvm -debug-only=polly-ast -o /dev/null %s 2>&1 > /dev/null | FileCheck --check-prefix=AST %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -mllvm -polly-use-llvm-names -o - %s | FileCheck --check-prefix=TRANS %s
// RUN: %clang -DMAIN -std=c99 -O3 -mllvm -polly -mllvm -polly-process-unprofitable %s -o %t_pragma_pack%exeext
// RUN: %t_pragma_pack%exeext | FileCheck --check-prefix=RESULT %s

__attribute__((noinline))
void pragma_unrolling(double C[const restrict static 256], double A[const restrict static 256]) {
  #pragma clang loop unrolling factor(4)
  for (int i = 0; i < 256; i += 1)
    C[i] = A[i] + i;
}

#ifdef MAIN
#include <stdio.h>
#include <string.h>
int main() {
  double C[256];
  double A[256];
  memset(A, 0, sizeof(A));
  A[1] = 42;
  pragma_unrolling(C,A);
  printf("(%0.0f)\n", C[1]);
  return 0;
}
#endif


// PRINT-LABEL: void pragma_unrolling(double C[const restrict static 256], double A[const restrict static 256]) __attribute__((noinline)) {
// PRINT-NEXT: #pragma clang loop unrolling factor(4)
// PRINT-NEXT:    for (int i = 0; i < 256; i += 1)
// PRINT-NEXT:      C[i] = A[i] + i;
// PRINT-NEXT: }


// IR-LABEL: define dso_local void @pragma_unrolling(double* noalias dereferenceable(2048) %C, double* noalias dereferenceable(2048) %A) #0 !looptransform !2 {
//
// IR:           br label %for.cond, !llvm.loop !4
//
// IR: !2 = !{!3}
// IR: !3 = !{!"llvm.loop.unroll", !4, i64 4}
// IR: !4 = distinct !{!4}


// AST: if (1)
// AST:     for (int c0 = 0; c0 <= 255; c0 += 1) {
// AST:       for (int c3 = 0; c3 <= 127; c3 += 1)
// AST:         CopyStmt_0(c0, 0, c3);
// AST:       for (int c1 = 0; c1 <= 127; c1 += 1)
// AST:         Stmt_for_body4(c0, c1);
// AST:     }
// AST: else
// AST:     {  /* original code */ }


// TRANS-LABEL: @pragma_pack
// TRANS: Packed_MemRef_


// RESULT: (45)
