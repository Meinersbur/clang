// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -ast-print %s | FileCheck --check-prefix=PRINT --match-full-lines %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -disable-llvm-passes -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -o - %s | FileCheck --check-prefix=TRANS %s

void pragma_id_tile(int m, int n, double C[m][n]) {
#pragma clang loop(i, j) tile sizes(32, 32)
#pragma clang loop id(i)
  for (int i = 0; i < m; i += 1)
#pragma clang loop id(j)
    for (int j = 0; j < n; j += 1)
      C[i][j] = i + j;
}


// FIXME: #pragmas printed in reverse
// PRINT-LABEL: void pragma_id_tile(int m, int n, double C[m][n]) {
// PRINT-DAG:   #pragma clang loop id(i)
// PRINT-DAG:   #pragma clang loop(i, j) tile sizes(32, 32)
// PRINT-NEXT:    for (int i = 0; i < m; i += 1)
// PRINT-NEXT:  #pragma clang loop id(j)
// PRINT-NEXT:      for (int j = 0; j < n; j += 1)
// PRINT-NEXT:         C[i][j] = i + j;
// PRINT-NEXT:  }


// IR-LABEL: define dso_local void @pragma_id_tile(i32 %m, i32 %n, double* %C) #0 !looptransform !2 {
// IR:         br label %for.cond1, !llvm.loop !6
// IR:         br label %for.cond, !llvm.loop !8
//
// IR: !2 = !{!3}
// IR: !3 = !{!"llvm.loop.tile", !4, !5}
// IR: !4 = !{!"i", !"j"}
// IR: !5 = !{i64 32, i64 32}
// IR: !6 = distinct !{!6, !7}
// IR: !7 = !{!"llvm.loop.id", !"j"}
// IR: !8 = distinct !{!8, !9}
// IR: !9 = !{!"llvm.loop.id", !"i"}


// TRANS: define dso_local void @pragma_id_tile(i32 %m, i32 %n, double* nocapture %C) local_unnamed_addr #0 !looptransform !2 {
// TRANS: entry:
// TRANS:   %0 = sext i32 %n to i64
// TRANS:   %1 = icmp sgt i32 %n, -1
// TRANS:   br i1 %1, label %polly.start, label %for.cond.cleanup
//
// TRANS: for.cond.cleanup:                                 ; preds = %polly.loop_exit37, %entry, %polly.start
// TRANS:   ret void
//
// TRANS: polly.start:                                      ; preds = %entry
// TRANS:   %2 = add nsw i64 %0, -1
// TRANS:   %polly.fdiv_q.shr = ashr i64 %2, 5
// TRANS:   %polly.loop_guard = icmp eq i32 %n, 0
// TRANS:   br i1 %polly.loop_guard, label %for.cond.cleanup, label %polly.loop_preheader
// 
// TRANS: polly.loop_header:                                ; preds = %polly.loop_exit37, %polly.loop_preheader
// TRANS:   %polly.indvar = phi i64 [ 0, %polly.loop_preheader ], [ %polly.indvar_next, %polly.loop_exit37 ]
// TRANS:   br i1 %polly.loop_guard38, label %polly.loop_header35.preheader, label %polly.loop_exit37
//
// TRANS: polly.loop_header35.preheader:                    ; preds = %polly.loop_header
// TRANS:   %3 = shl nsw i64 %polly.indvar, 5
// TRANS:   %4 = sub nsw i64 %0, %3
// TRANS:   %5 = add nsw i64 %4, -1
// TRANS:   %6 = icmp slt i64 %5, 31
// TRANS:   %7 = select i1 %6, i64 %5, i64 31
// TRANS:   %polly.loop_guard46 = icmp sgt i64 %7, -1
// TRANS:   br i1 %polly.loop_guard46, label %polly.loop_header35.us, label %polly.loop_exit37
// 
// TRANS: polly.loop_header35.us:                           ; preds = %polly.loop_header35.preheader, %polly.loop_exit45.us
// TRANS:   %polly.indvar39.us = phi i64 [ %polly.indvar_next40.us, %polly.loop_exit45.us ], [ 0, %polly.loop_header35.preheader ]
// TRANS:   %8 = shl nsw i64 %polly.indvar39.us, 5
// TRANS:   %9 = sub nsw i64 %21, %8
// TRANS:   %10 = add nsw i64 %9, -1
// TRANS:   %11 = icmp slt i64 %10, 31
// TRANS:   %12 = select i1 %11, i64 %10, i64 31
// TRANS:   %polly.loop_guard54.us = icmp sgt i64 %12, -1
// TRANS:   br i1 %polly.loop_guard54.us, label %polly.loop_header43.us.us, label %polly.loop_exit45.us
// 
// TRANS: polly.loop_exit45.us:                             ; preds = %polly.loop_exit53.us.us, %polly.loop_header35.us
// TRANS:   %polly.indvar_next40.us = add nuw nsw i64 %polly.indvar39.us, 1
// TRANS:   %polly.loop_cond41.us = icmp slt i64 %polly.indvar39.us, %polly.fdiv_q.shr33
// TRANS:   br i1 %polly.loop_cond41.us, label %polly.loop_header35.us, label %polly.loop_exit37
// 
// TRANS: polly.loop_header43.us.us:                        ; preds = %polly.loop_header35.us, %polly.loop_exit53.us.us
// TRANS:   %polly.indvar47.us.us = phi i64 [ %polly.indvar_next48.us.us, %polly.loop_exit53.us.us ], [ 0, %polly.loop_header35.us ]
// TRANS:   %13 = add nuw nsw i64 %polly.indvar47.us.us, %3
// TRANS:   %14 = trunc i64 %13 to i32
// TRANS:   br label %polly.loop_header51.us.us
// 
// TRANS: polly.loop_exit53.us.us:                          ; preds = %polly.loop_header51.us.us
// TRANS:   %polly.indvar_next48.us.us = add nuw nsw i64 %polly.indvar47.us.us, 1
// TRANS:   %polly.loop_cond49.us.us = icmp slt i64 %polly.indvar47.us.us, %7
// TRANS:   br i1 %polly.loop_cond49.us.us, label %polly.loop_header43.us.us, label %polly.loop_exit45.us
// 
// TRANS: polly.loop_header51.us.us:                        ; preds = %polly.loop_header51.us.us, %polly.loop_header43.us.us
// TRANS:   %polly.indvar55.us.us = phi i64 [ %polly.indvar_next56.us.us, %polly.loop_header51.us.us ], [ 0, %polly.loop_header43.us.us ]
// TRANS:   %15 = add nuw nsw i64 %polly.indvar55.us.us, %8
// TRANS:   %16 = trunc i64 %15 to i32
// TRANS:   %17 = add i32 %16, %14
// TRANS:   %p_conv.us.us.us = sitofp i32 %17 to double
// TRANS:   %18 = mul i64 %15, %20
// TRANS:   %19 = add nuw i64 %18, %13
// TRANS:   %scevgep.us.us = getelementptr double, double* %C, i64 %19
// TRANS:   store double %p_conv.us.us.us, double* %scevgep.us.us, align 8, !alias.scope !6, !noalias !8
// TRANS:   %polly.indvar_next56.us.us = add nuw nsw i64 %polly.indvar55.us.us, 1
// TRANS:   %polly.loop_cond57.us.us = icmp slt i64 %polly.indvar55.us.us, %12
// TRANS:   br i1 %polly.loop_cond57.us.us, label %polly.loop_header51.us.us, label %polly.loop_exit53.us.us
//
// TRANS: polly.loop_exit37:                                ; preds = %polly.loop_exit45.us, %polly.loop_header35.preheader, %polly.loop_header
// TRANS:   %polly.indvar_next = add nuw nsw i64 %polly.indvar, 1
// TRANS:   %polly.loop_cond = icmp slt i64 %polly.indvar, %polly.fdiv_q.shr
// TRANS:   br i1 %polly.loop_cond, label %polly.loop_header, label %for.cond.cleanup
// 
// TRANS: polly.loop_preheader:                             ; preds = %polly.start
// TRANS:   %20 = zext i32 %n to i64
// TRANS:   %21 = sext i32 %m to i64
// TRANS:   %22 = add nsw i64 %21, -1
// TRANS:   %polly.fdiv_q.shr33 = ashr i64 %22, 5
// TRANS:   %polly.loop_guard38 = icmp sgt i32 %m, 0
// TRANS:   br label %polly.loop_header
// TRANS: }
