// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -ast-print %s | FileCheck --check-prefix=PRINT --match-full-lines %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -disable-llvm-passes -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -mllvm -polly-use-llvm-names -mllvm -debug-only=polly-ast -o /dev/null %s 2>&1 > /dev/null | FileCheck --check-prefix=AST %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -fno-unroll-loops -mllvm -polly -mllvm -polly-process-unprofitable -mllvm -polly-allow-nonaffine -mllvm -polly-use-llvm-names -o - %s | FileCheck --check-prefix=TRANS %s
// RUN: %clang -DMAIN -std=c99 -O3 -fno-unroll-loops -mllvm -polly -mllvm -polly-process-unprofitable %s -o %t_pragma_pack%exeext
// RUN: %t_pragma_pack%exeext | FileCheck --check-prefix=RESULT %s

__attribute__((noinline))
void matmul(int M, int N, int K, double C[restrict M][N], double A[restrict M][K], double B[restrict K][N]) {
  // #pragma clang loop(i2,i2) vectorize 
  #pragma clang loop(j2/*,k2*/) pack array(B)
  #pragma clang loop(i2/*,k2*/) pack array(A)
  #pragma clang loop(i1,j1,k1,i2,j2) interchange permutation(j1,k1,i1,j2,i2)
  #pragma clang loop(i,j,k) tile sizes(96,2048,256) pit_ids(i1,j1,k1) tile_ids(i2,j2,k2)
  #pragma clang loop id(i)
  for (int i = 0; i < M; i += 1)
    #pragma clang loop id(j)
    for (int j = 0; j < N; j += 1)
      #pragma clang loop id(k)
      for (int k = 0; k < K; k += 1)
        C[i][j] += A[i][k] * B[k][j];
}


#ifdef MAIN
#include <stdio.h>
#include <string.h>
int main() {
  double C[16][32];
  double A[16][64];
  double B[64][32];
  memset(C, 0, sizeof(C));
  memset(A, 0, sizeof(A));
  memset(B, 0, sizeof(B));
  A[1][2] = 2;
  A[1][3] = 3;
  B[2][4] = 5;
  B[3][4] = 7;
  matmul(16,32,64,C,A,B);
  printf("(%0.0f)\n", C[1][4]);
  return 0;
}
#endif


// PRINT-LABEL: void matmul(int M, int N, int K, double C[restrict M][N], double A[restrict M][K], double B[restrict K][N]) __attribute__((noinline)) {
// PRINT-NEXT: #pragma clang loop(j2) pack array(B)
// PRINT-NEXT: #pragma clang loop(i2) pack array(A)
// PRINT-NEXT: #pragma clang loop(i1, j1, k1, i2, j2) interchange permutation(j1, k1, i1, j2, i2)
// PRINT-NEXT: #pragma clang loop(i, j, k) tile sizes(96, 2048, 256) pit_ids(i1, j1, k1) tile_ids(i2, j2, k2)
// PRINT-NEXT: #pragma clang loop id(i)
// PRINT-NEXT:     for (int i = 0; i < M; i += 1)
// PRINT-NEXT: #pragma clang loop id(j)
// PRINT-NEXT:         for (int j = 0; j < N; j += 1)
// PRINT-NEXT: #pragma clang loop id(k)
// PRINT-NEXT:             for (int k = 0; k < K; k += 1)
// PRINT-NEXT:                 C[i][j] += A[i][k] * B[k][j];
// PRINT-NEXT: }


// IR-LABEL: define dso_local void @matmul(i32 %M, i32 %N, i32 %K, double* noalias %C, double* noalias %A, double* noalias %B) #0 !looptransform !2 {
// IR: !"llvm.loop.tile"
// IR: !"llvm.loop.interchange"
// IR: !"llvm.data.pack"
// IR: !"llvm.data.pack"

// RESULT: (41)
