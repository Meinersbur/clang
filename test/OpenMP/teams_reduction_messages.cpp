// RUN: %clang_cc1 -triple x86_64-apple-macos10.7.0 -verify -fopenmp -ferror-limit 100 %s

void foo() {
}

bool foobool(int argc) {
  return argc;
}

struct S1; // expected-note {{declared here}} expected-note 2 {{forward declaration of 'S1'}}
extern S1 a;
class S2 {
  mutable int a;
  S2 &operator +=(const S2 &arg) {return (*this);} // expected-note {{implicitly declared private here}}
public:
  S2():a(0) { }
  S2(S2 &s2):a(s2.a) { }
  static float S2s; // expected-note {{predetermined as shared}}
  static const float S2sc;
};
const float S2::S2sc = 0; // expected-note {{'S2sc' defined here}}
S2 b; // expected-note {{'b' defined here}}
const S2 ba[5]; // expected-note {{'ba' defined here}}
class S3 {
  int a;
public:
  S3():a(0) { }
  S3(const S3 &s3):a(s3.a) { }
  S3 operator +=(const S3 &arg1) {return arg1;}
};
int operator +=(const S3 &arg1, const S3 &arg2) {return 5;} // expected-note {{candidate function not viable: no known conversion from 'class S6' to 'const S3' for 1st argument}}
S3 c; // expected-note {{'c' defined here}}
const S3 ca[5]; // expected-note {{'ca' defined here}}
extern const int f; // expected-note 2 {{'f' declared here}}
class S4 { // expected-note {{'S4' declared here}}
  int a;
  S4();
  S4(const S4 &s4);
  S4 &operator +=(const S4 &arg) {return (*this);}
public:
  S4(int v):a(v) { }
};
S4 &operator &=(S4 &arg1, S4 &arg2) {return arg1;} // expected-note {{candidate function not viable: no known conversion from 'S5' to 'S4 &' for 1st argument}}
class S5 {
  int a;
  S5():a(0) {}
  S5(const S5 &s5):a(s5.a) { }
  S5 &operator +=(const S5 &arg);
public:
  S5(int v):a(v) { }
};
class S6 {
    int a;
  public:
    S6():a(6){ }
    operator int() { return 6; }
} o;

S3 h, k;
#pragma omp threadprivate(h) // expected-note {{defined as threadprivate or thread local}}

int main(int argc, char **argv) {
  const int d = 5; // expected-note 2 {{'d' defined here}}
  const int da[5] = { 0 }; // expected-note {{'da' defined here}}
  int qa[5] = { 0 };
  S4 e(4); // expected-note {{'e' defined here}}
  S5 g(5);
  int i;
  int &j = i; // expected-note 2 {{'j' defined here}}
  S3 &p = k;
  const int &r = da[i]; // expected-note {{'r' defined here}}
  int &q = qa[i]; // expected-note {{'q' defined here}}
  float fl; // expected-note {{'fl' defined here}}
  #pragma omp target
  #pragma omp teams reduction // expected-error {{expected '(' after 'reduction'}} expected-error {{expected unqualified-id}} expected-error {{expected ':' in 'reduction' clause}}
  foo();
  #pragma omp target
  #pragma omp teams reduction + // expected-error {{expected '(' after 'reduction'}} expected-error {{expected ':' in 'reduction' clause}} expected-error {{expected expression}}
  foo();
  #pragma omp target
  #pragma omp teams reduction ( // expected-error {{expected unqualified-id}} expected-error {{expected ':' in 'reduction' clause}} expected-error {{expected ')'}} expected-note {{to match this '('}}
  foo();
  #pragma omp target
  #pragma omp teams reduction (- // expected-error {{expected ':' in 'reduction' clause}} expected-error {{expected expression}} expected-error {{expected ')'}} expected-note {{to match this '('}}
  foo();
  #pragma omp target
  #pragma omp teams reduction () // expected-error {{expected unqualified-id}} expected-error {{expected ':' in 'reduction' clause}}
  foo();
  #pragma omp target
  #pragma omp teams reduction (*) // expected-error {{expected ':' in 'reduction' clause}} expected-error {{expected expression}}
  foo();
  #pragma omp target
  #pragma omp teams reduction (\) // expected-error {{expected unqualified-id}} expected-error {{expected ':' in 'reduction' clause}} expected-error {{expected expression}}
  foo();
  #pragma omp target
  #pragma omp teams reduction (&: argc // expected-error {{expected ')'}} expected-note {{to match this '('}}
  foo();
  #pragma omp target
  #pragma omp teams reduction (| :argc, // expected-error {{expected expression}} expected-error {{expected ')'}} expected-note {{to match this '('}}
  foo();
  #pragma omp target
  #pragma omp teams reduction (|| :argc > 0 ? argv[1] : argv[2]) // expected-error {{expected variable name}}
  foo();
  #pragma omp target
  #pragma omp teams reduction (&& :argc)
  foo();
  #pragma omp target
  #pragma omp teams reduction (^ : S1) // expected-error {{'S1' does not refer to a value}}
  foo();
  #pragma omp target
  #pragma omp teams reduction (+ : a, b, c, d, f) // expected-error {{reduction variable with incomplete type 'S1'}} expected-error {{'operator+=' is a private member of 'S2'}} expected-error 2 {{const-qualified variable cannot be reduction}}
  foo();
  #pragma omp target
  #pragma omp teams reduction (min : a, b, c, d, f) // expected-error {{reduction variable with incomplete type 'S1'}} expected-error 2 {{arguments of OpenMP clause 'reduction' for 'min' and 'max' must be of arithmetic type}} expected-error 2 {{const-qualified variable cannot be reduction}}
  foo();
  #pragma omp target
  #pragma omp teams reduction (max : argv[1]) // expected-error {{expected variable name}}
  foo();
  #pragma omp target
  #pragma omp teams reduction(+ : ba) // expected-error {{arguments of OpenMP clause 'reduction' cannot be of array type}}
  foo();
  #pragma omp target
  #pragma omp teams reduction(* : ca) // expected-error {{arguments of OpenMP clause 'reduction' cannot be of array type}}
  foo();
  #pragma omp target
  #pragma omp teams reduction(- : da) // expected-error {{arguments of OpenMP clause 'reduction' cannot be of array type}}
  foo();
  #pragma omp target
  #pragma omp teams reduction(^ : fl) // expected-error {{arguments of OpenMP clause 'reduction' with bitwise operators cannot be of floating type}}
  foo();
  #pragma omp target
  #pragma omp teams reduction(&& : S2::S2s) // expected-error {{shared variable cannot be reduction}}
  foo();
  #pragma omp target
  #pragma omp teams reduction(&& : S2::S2sc) // expected-error {{const-qualified variable cannot be reduction}}
  foo();
  #pragma omp target
  #pragma omp teams reduction(& : e, g) // expected-error {{reduction variable must have an accessible, unambiguous default constructor}} expected-error {{no viable overloaded '&='}}
  foo();
  #pragma omp target
  #pragma omp teams reduction(+ : h, k) // expected-error {{threadprivate or thread local variable cannot be reduction}}
  foo();
  #pragma omp target
  #pragma omp teams reduction(+ : o) // expected-error {{no viable overloaded '+='}}
  foo();
  #pragma omp target
  #pragma omp teams private(i), reduction(+ : j), reduction(+:q) // expected-error 2 {{argument of OpenMP clause 'reduction' must reference the same object in all threads}}
  foo();
  #pragma omp target
  #pragma omp teams reduction(+ : p), reduction(+ : p) // expected-error {{variable can appear only once in OpenMP 'reduction' clause}} expected-note {{previously referenced here}}
  foo();
  #pragma omp target
  #pragma omp teams reduction(+ : r) // expected-error {{const-qualified variable cannot be reduction}}
  foo();
  #pragma omp parallel shared(i)
  #pragma omp target
  #pragma omp teams reduction(min : i)
  foo();
  #pragma omp parallel reduction(min : i)
  #pragma omp target
  #pragma omp teams reduction(max : j) // expected-error {{argument of OpenMP clause 'reduction' must reference the same object in all threads}}
  foo();

  return 0;
}
