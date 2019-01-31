// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -ast-print %s | FileCheck --check-prefix=PRINT --match-full-lines %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -disable-llvm-passes -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -mllvm -debug-only=polly-ast -o /dev/null %s 2>&1 > /dev/null | FileCheck --check-prefix=AST %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -o - %s | FileCheck --check-prefix=TRANS %s
// RUN: %clang -DMAIN -std=c99 -O3 -mllvm -polly -mllvm -polly-process-unprofitable %s -o %t_pragma_pack%exeext
// RUN: %t_pragma_pack%exeext | FileCheck --check-prefix=RESULT %s

void pragma_id_tile(int m, int n, double C[m][n]) {
#pragma clang loop(i, j) tile sizes(32, 32)
#pragma clang loop id(i)
  for (int i = 0; i < m; i += 1)
#pragma clang loop id(j)
    for (int j = 0; j < n; j += 1)
      C[i][j] = i + j;
}

#ifdef MAIN
#include <stdio.h>
#include <string.h>
int main() {
  double C[256][128];
  memset(C, 0, sizeof(C));
  C[1][2] = 42;
  pragma_id_tile(256,128,C);
  printf("(%0.0f)\n", C[2][1]);
  return 0;
}
#endif


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
// IR:         br label %for.cond1, !llvm.loop !7
// IR:         br label %for.cond, !llvm.loop !9
//
// IR: !2 = !{!3}
// IR: !3 = !{!"llvm.loop.tile", !4, !5, !6, !6}
// IR: !4 = !{!"i", !"j"}
// IR: !5 = !{i64 32, i64 32}
// IR: !6 = !{}
// IR: !7 = distinct !{!7, !8}
// IR: !8 = !{!"llvm.loop.id", !"j"}
// IR: !9 = distinct !{!9, !10}
// IR: !10 = !{!"llvm.loop.id", !"i"}


// AST: if (1
// AST:     for (int c0 = 0; c0 <= floord(p_0 - 1, 32); c0 += 1)
// AST:       for (int c1 = 0; c1 <= floord(p_1 - 1, 32); c1 += 1)
// AST:         for (int c2 = 0; c2 <= min(31, p_0 - 32 * c0 - 1); c2 += 1)
// AST:           for (int c3 = 0; c3 <= min(31, p_1 - 32 * c1 - 1); c3 += 1)
// AST:             Stmt3(32 * c0 + c2, 32 * c1 + c3);
// AST: else
// AST:     {  /* original code */ }


// TRANS:  %polly.indvar = phi i64 [ 0, %polly.loop_preheader ], [ %polly.indvar_next, %polly.loop_exit37 ]
// TRANS:  %polly.indvar39.us = phi i64 [ %polly.indvar_next40.us, %polly.loop_exit45.us ], [ 0, %polly.loop_header35.preheader ]
// TRANS:  %polly.indvar47.us.us = phi i64 [ %polly.indvar_next48.us.us, %polly.loop_exit53.us.us ], [ 0, %polly.loop_header35.us ]
// TRANS:  %polly.indvar55.us.us = phi i64 [ %polly.indvar_next56.us.us, %polly.loop_header51.us.us ], [ 0, %polly.loop_header43.us.us ]


// RESULT: (3)
