// RUN: %clang_cc1 %s -ast-print -o - | FileCheck %s

// FIXME: Test fails because attribute order is reversed by ParsedAttributes.
// XFAIL: *

void run1(int *List, int Length) {
  int i = 0;
// CHECK: #pragma loop vectorize(4)
// CHECK-NEXT: #pragma loop interleave(8)
// CHECK-NEXT: #pragma loop vectorize(enable)
// CHECK-NEXT: #pragma loop interleave(enable)
#pragma clang loop vectorize_width(4)
#pragma clang loop interleave_width(8)
#pragma clang loop vectorize(enable)
#pragma clang loop interleave(enable)
// CHECK-NEXT: while (i < Length)
  while (i < Length) {
    List[i] = i;
    i++;
  }
}
