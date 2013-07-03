/*===---- fp2intrin.h - PPC FP2 (Double Hummer) intrinsics -----------------===
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

#ifndef _FP2INTRIN_H_INCLUDED
#define _FP2INTRIN_H_INCLUDED

typedef double __v2df __attribute__((__vector_size__(16)));

static __inline__ float _Complex __attribute__((__always_inline__, __nodebug__))
__cmplxf (float a, float b) {
  float _Complex r = { a, b };
  return r;
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__cmplx (double a, double b) {
  double _Complex r = { a, b };
  return r;
}

static __inline__ long double _Complex __attribute__((__always_inline__, __nodebug__))
__cmplxl (long double a, long double b) {
  long double _Complex r = { a, b };
  return r;
}

static __inline__ float __attribute__((__always_inline__, __nodebug__))
__crealf (float _Complex a) {
  return __real__ a;
}

static __inline__ double __attribute__((__always_inline__, __nodebug__))
__creal (double _Complex a) {
  return __real__ a;
}

static __inline__ long double __attribute__((__always_inline__, __nodebug__))
__creall (long double _Complex a) {
  return __real__ a;
}

static __inline__ float __attribute__((__always_inline__, __nodebug__))
__cimagf (float _Complex a) {
  return __imag__ a;
}

static __inline__ double __attribute__((__always_inline__, __nodebug__))
__cimag (double _Complex a) {
  return __imag__ a;
}

static __inline__ long double __attribute__((__always_inline__, __nodebug__))
__cimagl (long double _Complex a) {
  return __imag__ a;
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__lfps (const float * a) {
  __v2df b_ = __builtin_fp2_lfps(a);
  return *((double _Complex *) &b_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__lfxs (const float * a) {
  __v2df b_ = __builtin_fp2_lfxs(a);
  return *((double _Complex *) &b_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__lfpd (const double * a) {
  __v2df b_ = __builtin_fp2_lfpd(a);
  return *((double _Complex *) &b_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__lfxd (const double * a) {
  __v2df b_ = __builtin_fp2_lfxd(a);
  return *((double _Complex *) &b_);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
__stfps (float * b, double _Complex a) {
  __builtin_fp2_stfps(*((__v2df *) &a), b);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
__stfxs (float * b, double _Complex a) {
  __builtin_fp2_stfxs(*((__v2df *) &a), b);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
__stfpd (double * b, double _Complex a) {
  __builtin_fp2_stfpd(*((__v2df *) &a), b);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
__stfxd (double * b, double _Complex a) {
  __builtin_fp2_stfxd(*((__v2df *) &a), b);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__))
__stfpiw (int * b, double _Complex a) {
  __builtin_fp2_stfpiw(*((__v2df *) &a), b);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxmr (double _Complex a) {
  __v2df b_ = __builtin_fp2_fxmr(*((__v2df *) &a));
  return *((double _Complex *) &b_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpctiw (double _Complex a) {
  __v2df b_ = __builtin_fp2_fpctiw(*((__v2df *) &a));
  return *((double _Complex *) &b_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpctiwz (double _Complex a) {
  __v2df b_ = __builtin_fp2_fpctiwz(*((__v2df *) &a));
  return *((double _Complex *) &b_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fprsp (double _Complex a) {
  __v2df b_ = __builtin_fp2_fprsp(*((__v2df *) &a));
  return *((double _Complex *) &b_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpre (double _Complex a) {
  __v2df b_ = __builtin_fp2_fpre(*((__v2df *) &a));
  return *((double _Complex *) &b_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fprsqrte (double _Complex a) {
  __v2df b_ = __builtin_fp2_fprsqrte(*((__v2df *) &a));
  return *((double _Complex *) &b_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpneg (double _Complex a) {
  __v2df b_ = __builtin_fp2_fpneg(*((__v2df *) &a));
  return *((double _Complex *) &b_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpabs (double _Complex a) {
  __v2df b_ = __builtin_fp2_fpabs(*((__v2df *) &a));
  return *((double _Complex *) &b_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpnabs (double _Complex a) {
  __v2df b_ = __builtin_fp2_fpnabs(*((__v2df *) &a));
  return *((double _Complex *) &b_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpadd (double _Complex a, double _Complex b) {
  __v2df c_ = __builtin_fp2_fpadd(*((__v2df *) &a), *((__v2df *) &b));
  return *((double _Complex *) &c_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpsub (double _Complex a, double _Complex b) {
  __v2df c_ = __builtin_fp2_fpsub(*((__v2df *) &a), *((__v2df *) &b));
  return *((double _Complex *) &c_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpmul (double _Complex a, double _Complex b) {
  __v2df c_ = __builtin_fp2_fpmul(*((__v2df *) &a), *((__v2df *) &b));
  return *((double _Complex *) &c_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxmul (double _Complex a, double _Complex b) {
  __v2df c_ = __builtin_fp2_fxmul(*((__v2df *) &a), *((__v2df *) &b));
  return *((double _Complex *) &c_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxpmul (double _Complex a, double _Complex b) {
  __v2df c_ = __builtin_fp2_fxpmul(*((__v2df *) &a), *((__v2df *) &b));
  return *((double _Complex *) &c_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxsmul (double _Complex a, double _Complex b) {
  __v2df c_ = __builtin_fp2_fxsmul(*((__v2df *) &a), *((__v2df *) &b));
  return *((double _Complex *) &c_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpmadd (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fpmadd(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpnmadd (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fpnmadd(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpmsub (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fpmsub(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpnmsub (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fpnmsub(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxmadd (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxmadd(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxnmadd (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxnmadd(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxmsub (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxmsub(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxnmsub (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxnmsub(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcpmadd (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcpmadd(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcsmadd (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcsmadd(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcpnmadd (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcpnmadd(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcsnmadd (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcsnmadd(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcpmsub (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcpmsub(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcsmsub (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcsmsub(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcpnmsub (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcpnmsub(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcsnmsub (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcsnmsub(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcpnpma (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcpnpma(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcsnpma (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcsnpma(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcpnsma (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcpnsma(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcsnsma (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcsnsma(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcxma (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcxma(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcxnms (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcxnms(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcxnpma (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcxnpma(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fxcxnsma (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fxcxnsma(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

static __inline__ double _Complex __attribute__((__always_inline__, __nodebug__))
__fpsel (double _Complex a, double _Complex b, double _Complex c) {
  __v2df d_ = __builtin_fp2_fpsel(*((__v2df *) &a), *((__v2df *) &b), *((__v2df *) &c));
  return *((double _Complex *) &d_);
}

#endif

