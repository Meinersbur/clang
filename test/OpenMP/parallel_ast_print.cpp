// RUN: %clang_cc1 -verify -fopenmp -ast-print %s | FileCheck %s
// expected-no-diagnostics

void foo() {}

int main (int argc, char **argv) {
  int b = argc, c, d, e, f, g;
  static int a;
// CHECK: static int a;
#pragma omp parallel
// CHECK-NEXT: #pragma omp parallel
  a=2;
// CHECK-NEXT: a = 2;
#pragma omp parallel default(none), private(argc,b) firstprivate(argv) if (argc > 0)
// CHECK-NEXT: #pragma omp parallel default(none) private(argc,b) firstprivate(argv) if(argc > 0)
  foo();
// CHECK-NEXT: foo();
  return tmain(b, &b) + tmain(x, &x);
}
