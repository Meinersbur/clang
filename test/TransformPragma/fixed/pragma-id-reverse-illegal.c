// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -ast-print %s | FileCheck --check-prefix=PRINT --match-full-lines %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -disable-llvm-passes -o - %s | FileCheck --check-prefix=IR --match-full-lines %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -O3 -emit-llvm -mllvm -polly -mllvm -polly-process-unprofitable -o /dev/null %s 2>&1 > /dev/null | FileCheck %s --check-prefix=WARN
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -O3 -emit-llvm -mllvm -polly -mllvm -polly-process-unprofitable -o /dev/null -mllvm -debug-only=polly-ast %s 2>&1 > /dev/null | FileCheck %s --check-prefix=AST

void pragma_id_reverse(double *A, int N) {
#pragma clang loop(myloop) reverse
#pragma clang loop id(myloop)
  for (int i = 1; i < N; i++)
    A[i] = A[i-1] + 1;
}


// PRINT-LABEL: void pragma_id_reverse(double *A, int N) {
// PRINT-NEXT:  #pragma clang loop(myloop) reverse
// PRINT-NEXT:  #pragma clang loop id(myloop)
// PRINT-NEXT:    for (int i = 1; i < N; i++)
// PRINT-NEXT:      A[i] = A[i - 1] + 1;
// PRINT-NEXT:  }


// IR-LABEL: define dso_local void @pragma_id_reverse(double* %A, i32 %N) #0 {
// IR:         br label %for.cond, !llvm.loop !2
//
// IR: !2 = distinct !{!2, !3, !4, !5}
// IR: !3 = !{!"llvm.loop.disable_nonforced"}
// IR: !4 = !{!"llvm.loop.id", !"myloop"}
// IR: !5 = !{!"llvm.loop.reverse.enable", i1 true}


// WARN: pragma-id-reverse-illegal.c:6:6: warning: loop not reversed: reversing the loop would violate dependencies


// AST: if (1)
// AST: {
// AST: 	if (p_0 >= 2)
// AST: 		Stmt1();
// AST: 	for (int c0 = 0; c0 < p_0 - 1; c0 += 1)
// AST: 		Stmt2(c0);
// AST: }
// AST: else
// AST: {  /* original code */ }
