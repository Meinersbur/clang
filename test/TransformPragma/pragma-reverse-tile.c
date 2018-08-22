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
// IT: !8 = !{}

// TRANS: define dso_local void @pragma_reverse(i32 %n, double* nocapture %A) local_unnamed_addr #0 !looptransform !2 {
// TRANS: entry:
// TRANS:   %0 = sext i32 %n to i64
// TRANS:   %1 = sub nsw i64 1, %0
// TRANS:   %polly.loop_guard = icmp slt i64 %1, 128
// TRANS:   br i1 %polly.loop_guard, label %polly.loop_preheader, label %polly.exiting
//
// TRANS: polly.exiting:                                    ; preds = %polly.loop_exit15, %entry
// TRANS:   ret void
//
// TRANS: polly.loop_header:                                ; preds = %polly.loop_exit15, %polly.loop_preheader
// TRANS:   %polly.indvar = phi i64 [ %polly.fdiv_q.shr, %polly.loop_preheader ], [ %polly.indvar_next, %polly.loop_exit15 ]
// TRANS:   %2 = shl nsw i64 %polly.indvar, 7
// TRANS:   %3 = sub i64 %1, %2
// TRANS:   %4 = icmp sgt i64 %3, 0
// TRANS:   %5 = select i1 %4, i64 %3, i64 0
// TRANS:   %6 = mul nsw i64 %polly.indvar, -128
// TRANS:   %7 = icmp slt i64 %6, 127
// TRANS:   %8 = select i1 %7, i64 %6, i64 127
// TRANS:   %polly.loop_guard16 = icmp sgt i64 %5, %8
// TRANS:   br i1 %polly.loop_guard16, label %polly.loop_exit15, label %polly.loop_header13
//
// TRANS: polly.loop_exit15:                                ; preds = %polly.loop_header13, %polly.loop_header
// TRANS:   %polly.indvar_next = add nsw i64 %polly.indvar, 1
// TRANS:   %polly.loop_cond = icmp slt i64 %polly.indvar, 0
// TRANS:   br i1 %polly.loop_cond, label %polly.loop_header, label %polly.exiting
//
// TRANS: polly.loop_preheader:                             ; preds = %entry
// TRANS:   %polly.fdiv_q.shr = ashr i64 %1, 7
// TRANS:   %9 = add nsw i64 %0, -1
// TRANS:   %scevgep = getelementptr double, double* %A, i64 %9
// TRANS:   br label %polly.loop_header
//
// TRANS: polly.loop_header13:                              ; preds = %polly.loop_header, %polly.loop_header13
// TRANS:   %polly.indvar17 = phi i64 [ %polly.indvar_next18, %polly.loop_header13 ], [ %5, %polly.loop_header ]
// TRANS:   %10 = sub i64 %polly.indvar17, %6
// TRANS:   %scevgep20 = getelementptr double, double* %scevgep, i64 %10
// TRANS:   %_p_scalar_ = load double, double* %scevgep20, align 8
// TRANS:   %p_add = fadd double %_p_scalar_, 1.000000e+00
// TRANS:   store double %p_add, double* %scevgep20, align 8
// TRANS:   %polly.indvar_next18 = add nuw nsw i64 %polly.indvar17, 1
// TRANS:   %polly.loop_cond19 = icmp slt i64 %polly.indvar17, %8
// TRANS:   br i1 %polly.loop_cond19, label %polly.loop_header13, label %polly.loop_exit15
// TRANS: }
