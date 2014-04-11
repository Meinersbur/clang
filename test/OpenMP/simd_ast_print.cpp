// RUN: %clang_cc1 -verify -fopenmp=libiomp5 -ast-print %s | FileCheck %s
// RUN: %clang_cc1 -fopenmp=libiomp5 -x c++ -std=c++11 -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp=libiomp5 -std=c++11 -include-pch %t -fsyntax-only -verify %s -ast-print | FileCheck %s
// expected-no-diagnostics

#ifndef HEADER
#define HEADER

void foo() {}
int g_ind = 1;
template<class T, class N> T reduct(T* arr, N num) {
  N i;
  N ind;
  N myind;
  T sum = (T)0;
// CHECK: T sum = (T)0;
#pragma omp simd private(myind, g_ind)
// CHECK-NEXT: #pragma omp simd private(myind,g_ind)
  for (i = 0; i < num; ++i) {
    myind = ind;
    T cur = arr[myind];
    ind += g_ind;
    sum += cur;
  }
}

template<class T> struct S {
  S(const T &a)
    :m_a(a)
  {}
  T result(T *v) const {
    T res;
    T val;
// CHECK: T res;
// CHECK: T val;
    #pragma omp simd private(val)  safelen(7)
// CHECK-NEXT: #pragma omp simd private(val) safelen(7)
    for (T i = 7; i < m_a; ++i) {
      val = v[i-7] + m_a;
      res = val;
    }
    const T clen = 3;
// CHECK: T clen = 3;
    #pragma omp simd safelen(clen-1)
// CHECK-NEXT: #pragma omp simd safelen(clen - 1)
    for(T i = clen+2; i < 20; ++i) {
// CHECK-NEXT: for (T i = clen + 2; i < 20; ++i) {
      v[i] = v[v-clen] + 1;
// CHECK-NEXT: v[i] = v[v - clen] + 1;
    }
// CHECK-NEXT: }
    return res;
  }
  ~S()
  {}
  T m_a;
};

template<int LEN> struct S2 {
  static void func(int n, float *a, float *b, float *c) {
#pragma omp simd safelen(LEN)
    for(int i = 0; i < n; i++) {
      c[i] = a[i] + b[i];
    }
  }
};

// S2<4>::func is called below in main.
// CHECK: template <int LEN = 4> struct S2 {
// CHECK-NEXT: static void func(int n, float *a, float *b, float *c)     {
// CHECK-NEXT: #pragma omp simd safelen(4)
// CHECK-NEXT:   for (int i = 0; i < n; i++) {
// CHECK-NEXT:     c[i] = a[i] + b[i];
// CHECK-NEXT:   }
// CHECK-NEXT: }

int main (int argc, char **argv) {
  int b = argc, c, d, e, f, g;
  int k1=0,k2=0;
  static int *a;
// CHECK: static int *a;
#pragma omp simd
// CHECK-NEXT: #pragma omp simd
  for (int i=0; i < 2; ++i)*a=2;
// CHECK-NEXT: for (int i = 0; i < 2; ++i)
// CHECK-NEXT: *a = 2;
#pragma omp simd private(argc, b)
  for (int i = 0; i < 10; ++i)
  for (int j = 0; j < 10; ++j) {foo(); k1 += 8; k2 += 8;}
// CHECK-NEXT: #pragma omp simd private(argc,b)
// CHECK-NEXT: for (int i = 0; i < 10; ++i)
// CHECK-NEXT: for (int j = 0; j < 10; ++j) {
// CHECK-NEXT: foo();
// CHECK-NEXT: k1 += 8;
// CHECK-NEXT: k2 += 8;
// CHECK-NEXT: }
  for (int i = 0; i < 10; ++i)foo();
// CHECK-NEXT: for (int i = 0; i < 10; ++i)
// CHECK-NEXT: foo();
  const int CLEN = 4;
// CHECK-NEXT: const int CLEN = 4;
  #pragma omp simd safelen(CLEN)
// CHECK-NEXT: #pragma omp simd safelen(CLEN)
  for (int i = 0; i < 10; ++i)foo();
// CHECK-NEXT: for (int i = 0; i < 10; ++i)
// CHECK-NEXT: foo();

  float arr[16];
  S2<4>::func(0,arr,arr,arr);
  return (0);
}

#endif
