/*===---- qpxintrin.h - PPC QPX intrinsics ---------------------------------===
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef _QPXINTRIN_H_INCLUDED
#define _QPXINTRIN_H_INCLUDED

typedef double vector4double __attribute__((__vector_size__(32)));

#ifndef __VECTOR4DOUBLE__ 
#define __VECTOR4DOUBLE__ 1
#endif

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ld (long a, long *b) {
  return __builtin_qpx_qvlfd((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lda (long a, long *b) {
  return __builtin_qpx_qvlfda((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ld (long a, unsigned long *b) {
  return __builtin_qpx_qvlfd((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lda (long a, unsigned long *b) {
  return __builtin_qpx_qvlfda((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ld (long a, long long *b) {
  return __builtin_qpx_qvlfd((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lda (long a, long long *b) {
  return __builtin_qpx_qvlfda((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ld (long a, unsigned long long *b) {
  return __builtin_qpx_qvlfd((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lda (long a, unsigned long long *b) {
  return __builtin_qpx_qvlfda((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ld (long a, float *b) {
  return __builtin_qpx_qvlfs((float *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lda (long a, float *b) {
  return __builtin_qpx_qvlfsa((float *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ld (long a, float _Complex *b) {
  return __builtin_qpx_qvlfs((float *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lda (long a, float _Complex *b) {
  return __builtin_qpx_qvlfsa((float *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ld (long a, double *b) {
  return __builtin_qpx_qvlfd((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lda (long a, double *b) {
  return __builtin_qpx_qvlfda((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ld (long a, double _Complex *b) {
  return __builtin_qpx_qvlfd((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lda (long a, double _Complex *b) {
  return __builtin_qpx_qvlfda((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_ldia (long a, int *b) {
  return __builtin_qpx_qvlfiwa((int *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_ldiaa (long a, int *b) {
  return __builtin_qpx_qvlfiwaa((int *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_ldiz (long a, unsigned *b) {
  return __builtin_qpx_qvlfiwz((unsigned *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_ldiza (long a, unsigned *b) {
  return __builtin_qpx_qvlfiwza((unsigned *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lds (long a, float *b) {
  vector4double r = (vector4double) { (double) *((float *) &((char *) b)[a]) };
  return __builtin_shufflevector(r, r, 0, 0, 0, 0);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lds (long a, double *b) {
  vector4double r = (vector4double) { *((double *) &((char *) b)[a]) };
  return __builtin_shufflevector(r, r, 0, 0, 0, 0);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lds (long a, float _Complex *b) {
  return __builtin_qpx_qvlfcs((float _Complex *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ldsa (long a, float _Complex *b) {
  return __builtin_qpx_qvlfcsa((float _Complex *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lds (long a, double _Complex *b) {
  return __builtin_qpx_qvlfcd((double _Complex *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ldsa (long a, double _Complex *b) {
  return __builtin_qpx_qvlfcda((double _Complex *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ld2 (long a, float *b) {
  return __builtin_qpx_qvlfcs((float _Complex *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ld2a (long a, float *b) {
  return __builtin_qpx_qvlfcsa((float _Complex *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ld2 (long a, double *b) {
  return __builtin_qpx_qvlfcd((double _Complex *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_ld2a (long a, double *b) {
  return __builtin_qpx_qvlfcda((double _Complex *) &((char *) b)[a]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st (vector4double a, long b, int *c) {
  return __builtin_qpx_qvstfiw(a, (unsigned *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sta (vector4double a, long b, int *c) {
  return __builtin_qpx_qvstfiwa(a, (unsigned *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st (vector4double a, long b, unsigned *c) {
  return __builtin_qpx_qvstfiw(a, (unsigned *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sta (vector4double a, long b, unsigned *c) {
  return __builtin_qpx_qvstfiwa(a, (unsigned *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st (vector4double a, long b, long *c) {
  return __builtin_qpx_qvstfd(a, (double *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sta (vector4double a, long b, long *c) {
  return __builtin_qpx_qvstfda(a, (double *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st (vector4double a, long b, unsigned long *c) {
  return __builtin_qpx_qvstfd(a, (double *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sta (vector4double a, long b, unsigned long *c) {
  return __builtin_qpx_qvstfda(a, (double *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st (vector4double a, long b, long long *c) {
  return __builtin_qpx_qvstfd(a, (double *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sta (vector4double a, long b, long long *c) {
  return __builtin_qpx_qvstfda(a, (double *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st (vector4double a, long b, unsigned long long *c) {
  return __builtin_qpx_qvstfd(a, (double *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sta (vector4double a, long b, unsigned long long *c) {
  return __builtin_qpx_qvstfda(a, (double *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st (vector4double a, long b, float *c) {
  return __builtin_qpx_qvstfs(a, (float *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sta (vector4double a, long b, float *c) {
  return __builtin_qpx_qvstfsa(a, (float *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st (vector4double a, long b, float _Complex *c) {
  return __builtin_qpx_qvstfs(a, (float *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sta (vector4double a, long b, float _Complex *c) {
  return __builtin_qpx_qvstfsa(a, (float *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st (vector4double a, long b, double *c) {
  return __builtin_qpx_qvstfd(a, (double *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sta (vector4double a, long b, double *c) {
  return __builtin_qpx_qvstfda(a, (double *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st (vector4double a, long b, double _Complex *c) {
  return __builtin_qpx_qvstfd(a, (double *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sta (vector4double a, long b, double _Complex *c) {
  return __builtin_qpx_qvstfda(a, (double *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sts (vector4double a, long b, float *c) {
  *((float *) &((char *) c)[b]) = (float) a[0];
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sts (vector4double a, long b, double *c) {
  *((double *) &((char *) c)[b]) = a[0];
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sts (vector4double a, long b, float _Complex *c) {
  return __builtin_qpx_qvstfcs(a, (float _Complex *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_stsa (vector4double a, long b, float _Complex *c) {
  return __builtin_qpx_qvstfcsa(a, (float _Complex *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_sts (vector4double a, long b, double _Complex *c) {
  return __builtin_qpx_qvstfcd(a, (double _Complex *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_stsa (vector4double a, long b, double _Complex *c) {
  return __builtin_qpx_qvstfcda(a, (double _Complex *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st2 (vector4double a, long b, float *c) {
  return __builtin_qpx_qvstfcs(a, (float _Complex *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st2a (vector4double a, long b, float *c) {
  return __builtin_qpx_qvstfcsa(a, (float _Complex *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st2 (vector4double a, long b, double *c) {
  return __builtin_qpx_qvstfcd(a, (double _Complex *) &((char *) c)[b]);
}

static __inline__ void __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_st2a (vector4double a, long b, double *c) {
  return __builtin_qpx_qvstfcda(a, (double _Complex *) &((char *) c)[b]);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_abs (vector4double a) {
  return __builtin_qpx_qvfabs(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_neg (vector4double a) {
  return __builtin_qpx_qvfneg(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_nabs (vector4double a) {
  return __builtin_qpx_qvfnabs(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_re (vector4double a) {
  return __builtin_qpx_qvfre(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_res (vector4double a) {
  return __builtin_qpx_qvfres(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_rsqrte (vector4double a) {
  return __builtin_qpx_qvfrsqrte(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_rsqrtes (vector4double a) {
  return __builtin_qpx_qvfrsqrtes(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_swsqrt (vector4double a) {
  return (vector4double)
         { __builtin_sqrt(a[0]), __builtin_sqrt(a[1]),
           __builtin_sqrt(a[2]), __builtin_sqrt(a[3]) };
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_swsqrt_nochk (vector4double a) {
  return (vector4double)
         { __builtin_sqrt(a[0]), __builtin_sqrt(a[1]),
           __builtin_sqrt(a[2]), __builtin_sqrt(a[3]) };
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_swsqrts (vector4double a) {
  return (vector4double)
         { __builtin_sqrtf((float) a[0]), __builtin_sqrtf((float) a[1]),
           __builtin_sqrtf((float) a[2]), __builtin_sqrtf((float) a[3]) };
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_swsqrts_nochk (vector4double a) {
  return (vector4double)
         { __builtin_sqrtf((float) a[0]), __builtin_sqrtf((float) a[1]),
           __builtin_sqrtf((float) a[2]), __builtin_sqrtf((float) a[3]) };
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_add (vector4double a, vector4double b) {
  return __builtin_qpx_qvfadd(a, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_cpsgn (vector4double a, vector4double b) {
  return __builtin_qpx_qvfcpsgn(a, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_mul (vector4double a, vector4double b) {
  return __builtin_qpx_qvfmul(a, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_sub (vector4double a, vector4double b) {
  return __builtin_qpx_qvfsub(a, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_swdiv (vector4double a, vector4double b) {
  return (vector4double) { a[0]/b[0], a[1]/b[1], a[2]/b[2], a[3]/b[3] };
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_swdiv_nochk (vector4double a, vector4double b) {
  return (vector4double) { a[0]/b[0], a[1]/b[1], a[2]/b[2], a[3]/b[3] };
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_swdivs (vector4double a, vector4double b) {

  return (vector4double)
         { (double) ((float) a[0]/ (float) b[0]),
           (double) ((float) a[1]/ (float) b[1]),
           (double) ((float) a[2]/ (float) b[2]),
           (double) ((float) a[3]/ (float) b[3]) };
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_swdivs_nochk (vector4double a, vector4double b) {
  return (vector4double)
         { (double) ((float) a[0]/ (float) b[0]),
           (double) ((float) a[1]/ (float) b[1]),
           (double) ((float) a[2]/ (float) b[2]),
           (double) ((float) a[3]/ (float) b[3]) };
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_xmul (vector4double a, vector4double b) {
  return __builtin_qpx_qvfxmul(a, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_madd (vector4double a, vector4double b, vector4double c) {
  return __builtin_qpx_qvfmadd(a, c, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_msub (vector4double a, vector4double b, vector4double c) {
  return __builtin_qpx_qvfmsub(a, c, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_nmadd (vector4double a, vector4double b, vector4double c) {
  return __builtin_qpx_qvfnmadd(a, c, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_nmsub (vector4double a, vector4double b, vector4double c) {
  return __builtin_qpx_qvfnmsub(a, c, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_xmadd (vector4double a, vector4double b, vector4double c) {
  return __builtin_qpx_qvfxmadd(a, c, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_xxmadd (vector4double a, vector4double b, vector4double c) {
  return __builtin_qpx_qvfxxmadd(a, c, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_xxcpnmadd (vector4double a, vector4double b, vector4double c) {
  return __builtin_qpx_qvfxxcpnmadd(a, c, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_xxnpmadd (vector4double a, vector4double b, vector4double c) {
  return __builtin_qpx_qvfxxnpmadd(a, c, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_ceil (vector4double a) {
  return __builtin_qpx_qvfrip(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_floor (vector4double a) {
  return __builtin_qpx_qvfrim(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_round (vector4double a) {
  return __builtin_qpx_qvfrin(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_rsp (vector4double a) {
  return __builtin_qpx_qvfrsp(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_trunc (vector4double a) {
  return __builtin_qpx_qvfriz(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_cfid (vector4double a) {
  return __builtin_qpx_qvfcfid(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_cfidu (vector4double a) {
  return __builtin_qpx_qvfcfidu(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_ctid (vector4double a) {
  return __builtin_qpx_qvfctid(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_ctidu (vector4double a) {
  return __builtin_qpx_qvfctidu(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_ctidz (vector4double a) {
  return __builtin_qpx_qvfctidz(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_ctiduz (vector4double a) {
  return __builtin_qpx_qvfctiduz(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_ctiw (vector4double a) {
  return __builtin_qpx_qvfctiw(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_ctiwu (vector4double a) {
  return __builtin_qpx_qvfctiwu(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_ctiwz (vector4double a) {
  return __builtin_qpx_qvfctiwz(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_ctiwuz (vector4double a) {
  return __builtin_qpx_qvfctiwuz(a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_cmpgt (vector4double a, vector4double b) {
  return __builtin_qpx_qvfcmpgt(a, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_cmplt (vector4double a, vector4double b) {
  return __builtin_qpx_qvfcmplt(a, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_cmpeq (vector4double a, vector4double b) {
  return __builtin_qpx_qvfcmpeq(a, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_sel (vector4double a, vector4double b, vector4double c) {
  return __builtin_qpx_qvfsel(c, a, b);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_tstnan (vector4double a, vector4double b) {
  return __builtin_qpx_qvftstnan(a, b);
}

static __inline__ double __attribute__((__always_inline__, __nodebug__))
vec_extract (vector4double a, int b) {
  return a[b & 3];
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_insert (double a, vector4double b, int c) {
  vector4double r = b;
  r[c & 3] = a;
  return r;
}

#define vec_gpci(a) __builtin_qpx_qvgpci(a)

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lvsl (long a, float *b) {
  return __builtin_qpx_qvlpcls((float *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lvsl (long a, float _Complex *b) {
  return __builtin_qpx_qvlpcls((float *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lvsl (long a, double *b) {
  return __builtin_qpx_qvlpcld((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lvsl (long a, double _Complex *b) {
  return __builtin_qpx_qvlpcld((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lvsr (long a, float *b) {
  return __builtin_qpx_qvlpcrs((float *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lvsr (long a, float _Complex *b) {
  return __builtin_qpx_qvlpcrs((float *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lvsr (long a, double *b) {
  return __builtin_qpx_qvlpcrd((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__overloadable__, __always_inline__, __nodebug__))
vec_lvsr (long a, double _Complex *b) {
  return __builtin_qpx_qvlpcrd((double *) &((char *) b)[a]);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_perm (vector4double a, vector4double b, vector4double c) {
  return __builtin_qpx_qvfperm(a, b, c);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_promote (double a, int b) {
  vector4double r;
  r[b & 3] = a;
  return r;
}

#define vec_sldw(a, b, c) __builtin_shufflevector(a, b, c, (c)+1, (c)+2, (c)+3);

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_splat (vector4double a, int b) {
  vector4double r = (vector4double) { a[b] };
  return __builtin_shufflevector(r, r, 0, 0, 0, 0);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_splats (double a) {
  vector4double r = (vector4double) { a };
  return __builtin_shufflevector(r, r, 0, 0, 0, 0);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_and (vector4double a, vector4double b) {
  return __builtin_qpx_qvflogical(a, b, 0x1);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_andc (vector4double a, vector4double b) {
  return __builtin_qpx_qvflogical(a, b, 0x4);
}

#define vec_logical(a, b, c) __builtin_qpx_qvflogical(a, b, c)

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_nand (vector4double a, vector4double b) {
  return __builtin_qpx_qvflogical(a, b, 0xE);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_nor (vector4double a, vector4double b) {
  return __builtin_qpx_qvflogical(a, b, 0x8);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_not (vector4double a) {
  return vec_nor(a, a);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_or (vector4double a, vector4double b) {
  return __builtin_qpx_qvflogical(a, b, 0x7);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_orc (vector4double a, vector4double b) {
  return __builtin_qpx_qvflogical(a, b, 0xD);
}

static __inline__ vector4double __attribute__((__always_inline__, __nodebug__))
vec_xor (vector4double a, vector4double b) {
  return __builtin_qpx_qvflogical(a, b, 0x6);
}

#if !QPXINTRIN_DONT_DEFINE_DCBT
static __inline__ void __attribute__((__always_inline__, __nodebug__))
__dcbt (void *p) {
  __builtin_prefetch(p);
}
#endif

#endif

