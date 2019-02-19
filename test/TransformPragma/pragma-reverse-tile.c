// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -ast-print %s | FileCheck --check-prefix=PRINT --match-full-lines %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -disable-llvm-passes -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -mllvm -polly-use-llvm-names -mllvm -debug-only=polly-ast -o /dev/null %s 2>&1 > /dev/null | FileCheck --check-prefix=AST %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -o - %s | FileCheck --check-prefix=TRANS %s
// RUN: %clang -DMAIN -std=c99 -O3 -mllvm -polly -mllvm -polly-process-unprofitable %s -o %t_pragma_pack%exeext
// RUN: %t_pragma_pack%exeext | FileCheck --check-prefix=RESULT %s

__attribute__((noinline))
void pragma_reverse(int n, double A[n]) {
#pragma clang loop tile sizes(128)
#pragma clang loop reverse
  for (int i = n - 1; i >= 0; i--)
    A[i] = A[i] + 1;
}

#ifdef MAIN
#include <stdio.h>
#include <string.h>
int main() {
  double A[2048];
  memset(A, 0, sizeof(A));
  A[2] = 42;
  pragma_reverse(2048,A);
  printf("(%0.0f)\n", A[2]);
  return 0;
}
#endif


// PRINT-LABEL: void pragma_reverse(int n, double A[n]) __attribute__((noinline)) {
// PRINT-NEXT:   #pragma clang loop tile sizes(128)
// PRINT-NEXT:   #pragma clang loop reverse
// PRINT-NEXT:    for (int i = n - 1; i >= 0; i--)
// PRINT-NEXT:      A[i] = A[i] + 1;
// PRINT-NEXT:  }


// IR-LABEL: define dso_local void @pragma_reverse(i32 %n, double* %A) #0 !looptransform !2 {
// IR:         br label %for.cond, !llvm.loop !4
//
// IR: !2 = !{!3, !5}
// IR: !3 = !{!"llvm.loop.reverse", !4}
// IR: !4 = distinct !{!4}
// IR: !5 = !{!"llvm.loop.tile", !6, !7, !8, !8}
// IR: !6 = !{!3}
// IR: !7 = !{i64 128}
// IR: !8 = !{}


// AST: if (1)
// AST:     for (int c0 = floord(-n + 1, 128); c0 <= 0; c0 += 1)
// AST:       for (int c1 = max(0, -n - 128 * c0 + 1); c1 <= min(127, -128 * c0); c1 += 1)
// AST:         Stmt_for_body(-128 * c0 - c1);
// AST: else
// AST:     {  /* original code */ }


// TRANS: polly.loop_preheader:
// TRANS: add nsw i64 %0, -1


// RESULT: (43)
