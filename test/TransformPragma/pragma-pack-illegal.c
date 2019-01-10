// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -ast-print %s | FileCheck --check-prefix=PRINT --match-full-lines %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -disable-llvm-passes -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -fno-unroll-loops -mllvm -polly -mllvm -polly-process-unprofitable -mllvm -polly-allow-nonaffine -mllvm -polly-use-llvm-names -o - %s | FileCheck --check-prefix=TRANS %s
// RUN: %clang -DMAIN -std=c99 -O3 -fno-unroll-loops -mllvm -polly -mllvm -polly-process-unprofitable %s -o %t_pragma_pack%exeext
// RUN: %t_pragma_pack%exeext | FileCheck --check-prefix=RESULT %s

__attribute__((noinline))
void pragma_pack(double C[restrict 16][32], double A[restrict 16*32][16]) {
  for (int i = 0; i < 16; i += 1)
    #pragma clang loop pack array(A)
    for (int j = 0; j < 32; j += 1)
      C[i][j] = A[j*i][i] + i + j;
}

#ifdef MAIN
#include <stdio.h>
#include <string.h>
int main() {
  double C[16][32];
  double A[16*32][16];
  memset(A, 0, sizeof(A));
  A[6][2] = 42;
  pragma_pack(C,A);
  printf("(%0.0f)\n", C[2][3]);
  return 0;
}
#endif


// PRINT-LABEL: void pragma_pack(double C[restrict 16][32], double A[restrict 512][16]) __attribute__((noinline)) {
// PRINT-NEXT:     for (int i = 0; i < 16; i += 1)
// PRINT-NEXT: #pragma clang loop pack array(A)
// PRINT-NEXT:         for (int j = 0; j < 32; j += 1)
// PRINT-NEXT:             C[i][j] = A[j * i][i] + i + j;
// PRINT-NEXT: }


// IR-LABEL: define dso_local void @pragma_pack([32 x double]* noalias %C, [16 x double]* noalias %A) #0 !looptransform !2 {
//
// IR:         %A.addr = alloca [16 x double]*, align 8
// IR:         load double, double* %arrayidx5, align 8, !llvm.access !6
//
// IR:       !2 = !{!3}
// IR:       !3 = !{!"llvm.data.pack", !4, !5, !"alloca"}
// IR:       !4 = distinct !{!4}
// IR:       !5 = !{!6}
// IR:       !6 = distinct !{}


// TRANS-LABEL: @pragma_pack
// TRANS-NOT: Packed_MemRef_

// RESULT: (47)
