// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -ast-print %s | FileCheck --check-prefix=PRINT --match-full-lines %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -disable-llvm-passes -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -o - %s | FileCheck --check-prefix=TRANS %s

void pragma_reverse(int n, double A[n]) {
#pragma clang loop tile sizes(128)
#pragma clang loop reverse
  for (int i = n - 1; i >= 0; i--)
    A[i] = A[i] + 1;
}

// PRINT-LABEL: void pragma_reverse(int n, double A[n]) {
// PRINT-NEXT:   #pragma clang loop reverse
// PRINT-NEXT:   #pragma clang loop tile sizes(128)
// PRINT-NEXT:    for (int i = n - 1; i >= 0; i--)
// PRINT-NEXT:      A[i] = A[i] + 1;
// PRINT-NEXT:  }

// IR-LABEL: define dso_local void @pragma_reverse(i32 %n, double* %A) #0 !looptransform !2 {
// IR:         br label %for.cond, !llvm.loop !4
//
// IR: !2 = !{!3}
// IR: !3 = !{!"llvm.loop.reverse", !4}

// TRANS: define dso_local void @pragma_reverse(i32 %n, double* nocapture %A)
// TRANS: entry:
// TRANS:   %0 = sext i32 %n to i64
// TRANS:   %polly.loop_guard = icmp sgt i32 %n, 0
// TRANS:   br i1 %polly.loop_guard, label %polly.loop_preheader, label %polly.exiting
//
// TRANS: polly.exiting:                                    ; preds = %polly.loop_header, %entry
// TRANS:   ret void
//
// TRANS: polly.loop_header:                                ; preds = %polly.loop_header, %polly.loop_preheader
// TRANS:   %polly.indvar = phi i64 [ %1, %polly.loop_preheader ], [ %polly.indvar_next, %polly.loop_header ]
// TRANS:   %scevgep12 = getelementptr double, double* %scevgep, i64 %polly.indvar
// TRANS:   %_p_scalar_ = load double, double* %scevgep12, align 8
// TRANS:   %p_add = fadd double %_p_scalar_, 1.000000e+00
// TRANS:   store double %p_add, double* %scevgep12, align 8
// TRANS:   %polly.indvar_next = add nsw i64 %polly.indvar, 1
// TRANS:   %polly.loop_cond = icmp slt i64 %polly.indvar, 0
// TRANS:   br i1 %polly.loop_cond, label %polly.loop_header, label %polly.exiting
//
// TRANS: polly.loop_preheader:                             ; preds = %entry
// TRANS:   %1 = sub nsw i64 1, %0
// TRANS:   %2 = add nsw i64 %0, -1
// TRANS:   %scevgep = getelementptr double, double* %A, i64 %2
// TRANS:   br label %polly.loop_header
// TRANS: }
