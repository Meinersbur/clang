// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -ast-print %s | FileCheck --check-prefix=PRINT --match-full-lines %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -disable-llvm-passes -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -mllvm -polly-use-llvm-names -mllvm -polly-parallel -mllvm -debug-only=polly-ast -o /dev/null %s 2>&1 > /dev/null | FileCheck --check-prefix=AST %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -fno-unroll-loops -mllvm -polly-parallel -mllvm -polly-use-llvm-names -o - %s | FileCheck --check-prefix=TRANS %s
// RUN: %clang -DMAIN -std=c99 -O3 -mllvm -polly -mllvm -polly-process-unprofitable -mllvm -polly-parallel %s -lgomp -o %t_pragma_pack%exeext
// RUN: %t_pragma_pack%exeext | FileCheck --check-prefix=RESULT %s

// REQUIRES: non-ms-sdk

__attribute__((noinline))
void pragma_tile_parallelize_thread(double C[const restrict static 256], double A[const restrict static 256]) {
  #pragma clang loop(outer) parallelize_thread
  #pragma clang loop tile sizes(32) pit_ids(outer)
  for (int i = 0; i < 256; i += 1)
    C[i] = A[i] + i;
}

#ifdef MAIN
#include <stdio.h>
#include <string.h>
int main() {
  double C[256];
  double A[256];
  memset(A, 0, sizeof(A));
  A[1] = 42;
  pragma_tile_parallelize_thread(C,A);
  printf("(%0.0f)\n", C[1]);
  return 0;
}
#endif


// PRINT-LABEL: void pragma_tile_parallelize_thread(double C[const restrict static 256], double A[const restrict static 256]) __attribute__((noinline)) {
// PRINT-NEXT: #pragma clang loop(outer) parallelize_thread
// PRINT-NEXT: #pragma clang loop tile sizes(32) pit_ids(outer)
// PRINT-NEXT:    for (int i = 0; i < 256; i += 1)
// PRINT-NEXT:      C[i] = A[i] + i;
// PRINT-NEXT: }


// IR-LABEL: define dso_local void @pragma_tile_parallelize_thread(double* noalias dereferenceable(2048) %C, double* noalias dereferenceable(2048) %A) #0 !looptransform !2 {
//
// IR:           br label %for.cond, !llvm.loop !5
//
// IR: !2 = !{!3, !9}


// IR: !3 = !{!"llvm.loop.tile", !4, !6, !7, !8}
// IR: !4 = !{!5}
// IR: !6 = !{i64 32}
// IR: !7 = !{!"outer"}
// IR: !8 = !{}
// IR: !9 = !{!"llvm.loop.parallelize_thread", !"outer"}


// AST: if (1)
// AST:   // Loop: threaded
// AST:   #pragma omp parallel for
// AST:   for (int c0 = 0; c0 <= 7; c0 += 1)
// AST:     for (int c1 = 0; c1 <= 31; c1 += 1)
// AST:       Stmt_for_body(32 * c0 + c1);
// AST: else
// AST:   {  /* original code */ }


// TRANS-LABEL: void @pragma_tile_parallelize_thread(
// TRANS: call void @GOMP_parallel_loop_runtime_start
// TRANS: call void @pragma_tile_parallelize_thread_polly_subfn
// TRANS: call void @GOMP_parallel_end

// TRANS-LABEL: define internal void @pragma_tile_parallelize_thread_polly_subfn(
// TRANS: %polly.loop_cond7 = icmp ult i64 %polly.indvar_next6, 32
// TRANS: br i1 %polly.loop_cond7, label %polly.loop_header2, label %polly.loop_exit4, !llvm.loop !14


// RESULT: (43)
