// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -o /dev/null -mllvm -debug-only=polly-ast %s 2>&1 > /dev/null | FileCheck %s

void pragma_id_tile(int m, int n, double C[m][n]) {
#pragma clang loop(i, j) tile sizes(32, 32)
#pragma clang loop id(i)
  for (int i = 0; i < m; i += 1)
#pragma clang loop id(j)
    for (int j = 0; j < n; j += 1)
      C[i][j] = C[j*2][i];
}


// CHECK: if (
// CHECK:     // Loop_i
// CHECK:     for (int c0 = 0; c0 < p_0; c0 += 1) {
// CHECK:       // Loop_j
// CHECK:       for (int c1 = 0; c1 < p_1; c1 += 1)
// CHECK:         Stmt3(c0, c1);
// CHECK:     }
// CHECK: else
// CHECK:     {  /* original code */ }
