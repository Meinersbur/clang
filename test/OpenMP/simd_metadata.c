// RUN: %clang_cc1 -fopenmp -emit-llvm %s -o - | FileCheck %s

void h1(float *c, float *a, float *b, int size)
{
// CHECK-LABEL: define void @h1
  int t = 0;
#pragma omp simd safelen(16) aligned(a, b, c) linear(t)
  for (int i = 0; i < size; ++i) {
    c[i] = a[i] * a[i] + b[i] * b[t];
    ++t;
// do not emit parallel_loop_access metadata due to usage of safelen clause.
// CHECK-NOT: store float {{.+}}, float* {{.+}}, align {{.+}}, !llvm.mem.parallel_loop_access {{![0-9]+}}
  }
}

void h2(float *c, float *a, float *b, int size)
{
// CHECK-LABEL: define void @h2
  int t = 0;
#pragma omp simd aligned(a, b, c) linear(t)
  for (int i = 0; i < size; ++i) {
    c[i] = a[i] * a[i] + b[i] * b[t];
    ++t;
// CHECK: store float {{.+}}, float* {{.+}}, align {{.+}}, !llvm.mem.parallel_loop_access [[LOOP_H2_HEADER:![0-9]+]]
  }
}

void h3(float *c, float *a, float *b, int size)
{
// CHECK-LABEL: define void @h3
#pragma omp simd aligned(a, b, c)
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      c[j*i] = a[i] * b[j];
    }
  }
// do not emit parallel_loop_access for nested loop.
// CHECK-NOT: store float {{.+}}, float* {{.+}}, align {{.+}}, !llvm.mem.parallel_loop_access {{![0-9]+}}
}

void h4(float *c, float *a, float *b, int size)
{
// CHECK-LABEL: define void @h4
#pragma omp simd aligned(a, b, c) collapse(2)
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      c[j*i] = a[i] * b[j];
    }
  }
// emit parallel_loop_access: nested loop is collapsed.
// CHECK: store float {{.+}}, float* {{.+}}, align {{.+}}, !llvm.mem.parallel_loop_access [[LOOP_H4_HEADER:![0-9]+]]
}

// Metadata for h1:
// CHECK: [[LOOP_H1_HEADER:![0-9]+]] = metadata !{metadata [[LOOP_H1_HEADER]], metadata [[LOOP_WIDTH_16:![0-9]+]], metadata [[LOOP_VEC_ENABLE:![0-9]+]]}
// CHECK: [[LOOP_WIDTH_16]] = metadata !{metadata !"llvm.vectorizer.width", i32 16}
// CHECK: [[LOOP_VEC_ENABLE]] = metadata !{metadata !"llvm.vectorizer.enable", i1 true}
//
// Metadata for h2:
// CHECK: [[LOOP_H2_HEADER]] = metadata !{metadata [[LOOP_H2_HEADER]], metadata [[LOOP_VEC_ENABLE]]}
//
// Metadata for h3:
// CHECK: [[LOOP_H3_HEADER:![0-9]+]] = metadata !{metadata [[LOOP_H3_HEADER]], metadata [[LOOP_VEC_ENABLE]]}
//
// Metadata for h4:
// CHECK: [[LOOP_H4_HEADER]] = metadata !{metadata [[LOOP_H4_HEADER]], metadata [[LOOP_VEC_ENABLE]]}
//

