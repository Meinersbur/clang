// RUN: %clang_cc1 -triple x86_64-apple-macos10.7.0 -verify -fopenmp -ferror-limit 100 -std=c++11 -o - %s

void foo() {
}

bool foobool(int argc) {
  return argc;
}

struct S1; // expected-note {{declared here}} expected-note {{forward declaration of 'S1'}}

extern S1 v1;

struct S2{
  int f;
  operator int() { return f; } // expected-note {{conversion to integral type 'int'}}
  operator bool() { return f; } // expected-note {{conversion to integral type 'bool'}}
} v2;

struct S3 {
  int f;
  explicit operator int() { return f; } // expected-note {{conversion to integral type 'int'}}
} v3;

int main(int argc, char **argv) {
  #pragma omp target
  #pragma omp teams num_teams // expected-error {{expected '(' after 'num_teams'}} expected-error {{expected expression}}
  foo();
  #pragma omp target
  #pragma omp teams num_teams ( // expected-error {{expected expression}} expected-error {{expected ')'}} expected-note {{to match this '('}}
  foo();
  #pragma omp target
  #pragma omp teams num_teams () // expected-error {{expected expression}}
  foo();
  #pragma omp target
  #pragma omp teams num_teams (argc // expected-error {{expected ')'}} expected-note {{to match this '('}}
  foo();
  #pragma omp target
  #pragma omp teams num_teams (argc > 0 ? argv[1] : argv[2]) // expected-error {{statement requires expression of integer type ('char *' invalid)}}
  foo();
  #pragma omp target
  #pragma omp teams num_teams (foobool(argc)) num_teams(3) // expected-error {{directive '#pragma omp teams' cannot contain more than one 'num_teams' clause}}
  foo();
  #pragma omp target
  #pragma omp teams num_teams (S1) // expected-error {{'S1' does not refer to a value}}
  foo();
  #pragma omp target
  #pragma omp teams num_teams (argv[1]=2) // expected-error {{expected ')'}} expected-note {{to match this '('}} expected-error {{statement requires expression of integer type ('char *' invalid)}}
  foo();
  #pragma omp target
  #pragma omp teams num_teams (v1) // expected-error {{expression has incomplete type 'S1'}}
  foo();
  #pragma omp target
  #pragma omp teams num_teams (v2) // expected-error {{multiple conversions from expression type 'struct S2' to an integral or enumeration type}}
  foo();
  #pragma omp target
  #pragma omp teams num_teams (v3) // expected-error {{expression type 'struct S3' requires explicit conversion to 'int'}}
  foo();
  #pragma omp target
  #pragma omp teams num_teams (0) // expected-error {{expression is not a positive integer value}}
  foo();
  #pragma omp target
  #pragma omp teams num_teams (-1) // expected-error {{expression is not a positive integer value}}
  foo();

  return 0;
}
