// RUN: %clang_cc1 -verify -fopenmp -ferror-limit 100 %s

void foo() {
}

bool foobool(int argc) {
  return argc;
}

struct S1; // expected-note {{declared here}}
extern S1 a;
class S2 {
  mutable int a;
public:
  S2():a(0) { }
  S2(S2 &s2):a(s2.a) { }
  static float S2s; // expected-note 2 {{mappable type cannot contain static members}}
  static const float S2sc; // expected-note 2 {{mappable type cannot contain static members}}
};
const float S2::S2sc = 0;
const S2 b;
const S2 ba[5];
class S3 {
  int a;
public:
  S3():a(0) { }
  S3(S3 &s3):a(s3.a) { }
};
const S3 c;
const S3 ca[5];
extern const int f;
class S4 {
  int a;
  S4();
  S4(const S4 &s4);
public:
  S4(int v):a(v) { }
};
class S5 {
  int a;
  S5():a(0) {}
  S5(const S5 &s5):a(s5.a) { }
public:
  S5(int v):a(v) { }
};

S3 h;
#pragma omp threadprivate(h) // expected-error {{threadprivate variables cannot be used in target constructs}}

int main(int argc, char **argv) {
  const int d = 5;
  const int da[5] = { 0 };
  S4 e(4);
  S5 g(5);
  int i;
  int &j = i;
  int *k = &j;
  const int (&l)[5] = da;
  #pragma omp target map // expected-error {{expected '(' after 'map'}} expected-error {{expected expression}}
  #pragma omp target map ( // expected-error {{expected expression}} expected-error {{expected ')'}} expected-note {{to match this '('}}
  #pragma omp target map () // expected-error {{expected expression}}
  #pragma omp target map (alloc) // expected-error {{expected expression}}  expected-error {{expected ':' in 'map' clause}}
  #pragma omp target map (to argc // expected-error {{expected ')'}} expected-note {{to match this '('}} expected-error{{expected ':' in 'map' clause}}
  #pragma omp target map (from: argc, // expected-error {{expected expression}} expected-error {{expected ')'}} expected-note {{to match this '('}}
  #pragma omp target map (tofrom: argc > 0 ? argv[1] : argv[2]) // expected-error {{expected variable name}}
  #pragma omp target map (argc)
  #pragma omp target map (S1) // expected-error {{'S1' does not refer to a value}}
  #pragma omp target map (a, b, c, d, f) // expected-error {{incomplete type 'S1' where a complete type is required}} expected-error 2 {{type 'S2' is not mappable to target}}
  #pragma omp target map (argv[1])
  #pragma omp target map(ba) // expected-error 2 {{type 'S2' is not mappable to target}}
  #pragma omp target map(ca)
  #pragma omp target map(da)
  #pragma omp target map(S2::S2s)
  #pragma omp target map(S2::S2sc)
  #pragma omp target map(e, g)
  #pragma omp target map(h) // expected-note {{used here}}
  #pragma omp target map(k), map(k[:10]) // expected-error {{variable already marked as mapped in current construct}} expected-note {{used here}}
  foo();
  #pragma omp target map(da)
  #pragma omp target map(da[:4])
  foo();
  #pragma omp target map(k, j, l) // expected-note 2 {{used here}}
  #pragma omp target map(k[:4]) // expected-error {{variable already marked as mapped in current construct}}
  #pragma omp target map(j)
  #pragma omp target map(l[:5]) // expected-error {{variable already marked as mapped in current construct}}
  foo();
  #pragma omp target map(k[:4], j, l[:5]) // expected-note 2 {{used here}}
  #pragma omp target map(k) // expected-error {{variable already marked as mapped in current construct}}
  #pragma omp target map(j)
  #pragma omp target map(l) // expected-error {{variable already marked as mapped in current construct}}
  foo();

  return 0;
}
