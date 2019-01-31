// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -o /dev/null -mllvm -debug-only=polly-ast %s 2>&1 > /dev/null | FileCheck %s

void pragma_id_reverse(double *A, int N) {
#pragma clang loop(myloop) reverse
#pragma clang loop id(myloop)
  for (int i = 1; i < N; i++)
    A[i] = A[i-1] + 1;
}

// CHECK:      if (p_0 >= 2)
// CHECK-NEXT:        Stmt1();
// CHECK-NEXT:      // Loop_myloop
// CHECK-NEXT:      for (int c0 = 0; c0 < p_0 - 1; c0 += 1)
// CHECK-NEXT:        Stmt2(c0);
// CHECK-NEXT:    }
