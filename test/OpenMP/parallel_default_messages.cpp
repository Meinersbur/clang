// RUN: %clang_cc1 -verify -fopenmp=libiomp5 -ferror-limit 100 -o - %s

void foo();

int main(int argc, char **argv) {
  #pragma omp parallel default // expected-error {{expected '(' after 'default'}} expected-error {{expected 'none' or 'shared' in OpenMP clause 'default'}}
  #pragma omp parallel default ( // expected-error {{expected 'none' or 'shared' in OpenMP clause 'default'}} expected-error {{expected ')'}} expected-note {{to match this '('}}
  #pragma omp parallel default () // expected-error {{expected 'none' or 'shared' in OpenMP clause 'default'}}
  #pragma omp parallel default (none // expected-error {{expected ')'}} expected-note {{to match this '('}}
  #pragma omp parallel default (shared), default(shared) // expected-error {{directive '#pragma omp parallel' cannot contain more than one 'default' clause}}
  #pragma omp parallel default (x) // expected-error {{expected 'none' or 'shared' in OpenMP clause 'default'}}
  foo();

  return 0;
}
