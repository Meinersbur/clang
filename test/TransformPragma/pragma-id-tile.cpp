// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -ast-print %s | FileCheck --check-prefix=PRINT --match-full-lines %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -disable-llvm-passes -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -o - %s | FileCheck --check-prefix=TRANS %s

extern "C" void pragma_id_tile(int m, int n, double C[m][n]) {
  #pragma clang loop(i,j) tile
  #pragma clang loop id(i)
  for (int i=0; i<m; i+=1) 
    #pragma clang loop id(j)
    for (int j=0; j<n; j+=1) 
	    C[i][j] = i+j;
}

// FIXME: #pragmas printed in reverse
// PRINT-LABEL: extern "C" void pragma_id_reverse(double *A, int N) {
// PRINT-DAG:   #pragma clang loop(i,j) tile
// PRINT-DAG:   #pragma clang loop id(i)
// PRINT-NEXT:    for (int i = N - 1; i >= 0; i--)
// PRINT-NEXT:  #pragma clang loop id(j)
// PRINT-NEXT:      A[i] = A[i] + 1;
// PRINT-NEXT:  }


// IR-LABEL: define dso_local void @pragma_id_reverse(double* %A, i32 %N) #0 !looptransform !2 {
// IR:         br label %for.cond, !llvm.loop !4
//
// IR: !2 = !{!3}
// IR: !3 = !{!"llvm.loop.reverse", !"myloop"}
// IR: !4 = distinct !{!4, !5}
// IR: !5 = !{!"llvm.loop.id", !"myloop"}


// TRANS: define dso_local void @pragma_id_reverse(double* nocapture %A, i32 %N) local_unnamed_addr #0 !looptransform !2 {
// TRANS: entry:
// TRANS:   %0 = sext i32 %N to i64
// TRANS:   %polly.loop_guard = icmp sgt i32 %N, 0
// TRANS:   br i1 %polly.loop_guard, label %polly.loop_preheader, label %polly.exiting
// 
// TRANS: polly.exiting:                                    ; preds = %polly.loop_header, %entry
// TRANS:   ret void
// 
// TRANS: polly.loop_header:                                ; preds = %polly.loop_header, %polly.loop_preheader
// TRANS:   %polly.indvar = phi i64 [ %1, %polly.loop_preheader ], [ %polly.indvar_next, %polly.loop_header ]
// TRANS:   %scevgep11 = getelementptr double, double* %scevgep, i64 %polly.indvar
// TRANS:   %_p_scalar_ = load double, double* %scevgep11, align 8, !alias.scope !4, !noalias !6
// TRANS:   %p_add = fadd double %_p_scalar_, 1.000000e+00
// TRANS:   store double %p_add, double* %scevgep11, align 8, !alias.scope !4, !noalias !6
// TRANS:   %polly.indvar_next = add nsw i64 %polly.indvar, 1
// TRANS:   %polly.loop_cond = icmp slt i64 %polly.indvar, 0
// TRANS:   br i1 %polly.loop_cond, label %polly.loop_header, label %polly.exiting

// TRANS: polly.loop_preheader:                             ; preds = %entry
// TRANS:   %1 = sub nsw i64 1, %0
// TRANS:   %2 = add nsw i64 %0, -1
// TRANS:   %scevgep = getelementptr double, double* %A, i64 %2
// TRANS:   br label %polly.loop_header
// TRANS: }
