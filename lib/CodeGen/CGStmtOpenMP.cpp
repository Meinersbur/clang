//===--- CGStmtOpenMP.cpp - Emit LLVM Code for declarations ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This contains code to emit Decl nodes as LLVM code.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "CGDebugInfo.h"
#include "CGOpenCLRuntime.h"
#include "CodeGenModule.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/CharUnits.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclOpenMP.h"
#include "clang/Basic/OpenMPKinds.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/ADT/StringExtras.h"
using namespace clang;
using namespace CodeGen;

namespace {
struct ident_t {};
enum sched_type {};
enum kmp_proc_bind_t {};
typedef void (*kmpc_micro)(int32_t *global_tid, int32_t *bound_tid, ...);
typedef void(__kmpc_fork_call)(ident_t *loc, int32_t argc, kmpc_micro microtask,
                               ...);
typedef void(__kmpc_push_num_threads)(ident_t *loc, int32_t global_tid,
                                      int32_t num_threads);
typedef void(__kmpc_push_proc_bind)(ident_t *loc, int32_t global_tid,
                                    kmp_proc_bind_t proc_bind);
//const int KMP_PROC_BIND_FALSE = 0;
//const int KMP_PROC_BIND_TRUE = 1;
const int KMP_PROC_BIND_MASTER = 2;
const int KMP_PROC_BIND_CLOSE = 3;
const int KMP_PROC_BIND_SPREAD = 4;
//const int KMP_PROC_BIND_DISABLED = 5;
//const int KMP_PROC_BIND_INTEL = 6;
//const int KMP_PROC_BIND_DEFAULT = 7;
const int KMP_IDENT_BARRIER_EXPL = 0x20;
const int KMP_IDENT_BARRIER_IMPL = 0x0040;
const int KMP_IDENT_BARRIER_IMPL_FOR = 0x0040;
const int KMP_IDENT_BARRIER_IMPL_SECTIONS = 0x00C0;
const int KMP_IDENT_BARRIER_IMPL_SINGLE = 0x0140;
typedef int32_t(__kmpc_cancel_barrier)(ident_t *loc, int32_t global_tid);
const int KMP_CANCEL_NOREQ = 0;
const int KMP_CANCEL_PARALLEL = 1;
const int KMP_CANCEL_LOOP = 2;
const int KMP_CANCEL_SECTIONS = 3;
const int KMP_CANCEL_TASKGROUP = 4;
typedef int32_t(__kmpc_cancellationpoint)(ident_t *loc, int32_t global_tid,
                                          int32_t cncl_kind);
typedef int32_t(__kmpc_cancel)(ident_t *loc, int32_t global_tid,
                               int32_t cncl_kind);
typedef int32_t kmp_critical_name[8];
typedef int32_t(__kmpc_omp_taskyield)(ident_t *loc, int32_t global_tid,
                                      int32_t end_part);
typedef int32_t(__kmpc_omp_taskwait)(ident_t *loc, int32_t global_tid);
typedef void(__kmpc_flush)(ident_t *loc, ...);
typedef int32_t(__kmpc_master)(ident_t *loc, int32_t global_tid);
typedef void(__kmpc_end_master)(ident_t *loc, int32_t global_tid);
typedef int32_t(__kmpc_single)(ident_t *loc, int32_t global_tid);
typedef void(__kmpc_end_single)(ident_t *loc, int32_t global_tid);
typedef void(__kmpc_critical)(ident_t *loc, int32_t global_tid,
                              kmp_critical_name *lck);
typedef void(__kmpc_end_critical)(ident_t *loc, int32_t global_tid,
                                  kmp_critical_name *lck);
typedef void(__kmpc_ordered)(ident_t *loc, int32_t global_tid);
typedef void(__kmpc_end_ordered)(ident_t *loc, int32_t global_tid);
typedef void (*kmp_copy_func)(void *lhs_data, void *rhs_data);
typedef void(__kmpc_copyprivate)(ident_t *loc, int32_t global_tid,
                                 int32_t cpy_size, void *cpy_data,
                                 kmp_copy_func cpy_func, int32_t didit);
typedef void (*kmp_reduce_func)(void *lhs_data, void *rhs_data);
typedef int32_t(__kmpc_reduce_nowait)(ident_t *loc, int32_t global_tid,
                                      int32_t num_vars, size_t reduce_size,
                                      void *reduce_data,
                                      kmp_reduce_func reduce_func,
                                      kmp_critical_name *lck);
typedef void(__kmpc_end_reduce_nowait)(ident_t *loc, int32_t global_tid,
                                       kmp_critical_name *lck);
typedef int32_t(__kmpc_reduce)(ident_t *loc, int32_t global_tid,
                               int32_t num_vars, size_t reduce_size,
                               void *reduce_data, kmp_reduce_func reduce_func,
                               kmp_critical_name *lck);
typedef void(__kmpc_end_reduce)(ident_t *loc, int32_t global_tid,
                                kmp_critical_name *lck);
const int KMP_IDENT_ATOMIC_REDUCE = 0x10;
typedef void(__kmpc_atomic_start)();
typedef void(__kmpc_atomic_end)();
typedef void(__kmpc_dispatch_init_4)(ident_t *loc, int32_t global_tid,
                                     sched_type schedule, int32_t lb,
                                     int32_t ub, int32_t st, int32_t chunk);
typedef void(__kmpc_dispatch_init_4u)(ident_t *loc, int32_t global_tid,
                                      sched_type schedule, uint32_t lb,
                                      uint32_t ub, uint32_t st, uint32_t chunk);
typedef void(__kmpc_dispatch_init_8)(ident_t *loc, int32_t global_tid,
                                     sched_type schedule, int64_t lb,
                                     int64_t ub, int64_t st, int64_t chunk);
typedef void(__kmpc_dispatch_init_8u)(ident_t *loc, int32_t global_tid,
                                      sched_type schedule, uint64_t lb,
                                      uint64_t ub, uint64_t st, uint64_t chunk);
typedef int(__kmpc_dispatch_next_4)(ident_t *loc, int32_t global_tid,
                                    int32_t *p_last, int32_t *p_lb,
                                    int32_t *p_ub, int32_t *p_st);
typedef int(__kmpc_dispatch_next_4u)(ident_t *loc, int32_t global_tid,
                                     int32_t *p_last, uint32_t *p_lb,
                                     uint32_t *p_ub, int32_t *p_st);
typedef int(__kmpc_dispatch_next_8)(ident_t *loc, int32_t global_tid,
                                    int32_t *p_last, int64_t *p_lb,
                                    int64_t *p_ub, int64_t *p_st);
typedef int(__kmpc_dispatch_next_8u)(ident_t *loc, int32_t global_tid,
                                     int32_t *p_last, uint64_t *p_lb,
                                     uint64_t *p_ub, int64_t *p_st);
typedef void(__kmpc_dispatch_fini_4)(ident_t *loc, int32_t global_tid);
typedef void(__kmpc_dispatch_fini_4u)(ident_t *loc, int32_t global_tid);
typedef void(__kmpc_dispatch_fini_8)(ident_t *loc, int32_t global_tid);
typedef void(__kmpc_dispatch_fini_8u)(ident_t *loc, int32_t global_tid);
typedef void(__kmpc_for_static_init_4)(ident_t *loc, int32_t global_tid,
                                       int32_t schedule, int32_t *pliter,
                                       int32_t *plb, int32_t *pub, int32_t *pst,
                                       int32_t incr, int32_t chunk);
typedef void(__kmpc_for_static_init_4u)(ident_t *loc, int32_t global_tid,
                                        int32_t schedule, int32_t *pliter,
                                        int32_t *plb, int32_t *pub,
                                        int32_t *pst, int32_t incr,
                                        int32_t chunk);
typedef void(__kmpc_for_static_init_8)(ident_t *loc, int32_t global_tid,
                                       int32_t schedule, int32_t *pliter,
                                       int64_t *plb, int64_t *pub, int64_t *pst,
                                       int64_t incr, int64_t chunk);
typedef void(__kmpc_for_static_init_8u)(ident_t *loc, int32_t global_tid,
                                        int32_t schedule, int32_t *pliter,
                                        int64_t *plb, int64_t *pub,
                                        int64_t *pst, int64_t incr,
                                        int64_t chunk);
typedef void(__kmpc_for_static_fini)(ident_t *loc, int32_t global_tid);
const int KMP_SCH_STATIC_CHUNKED = 33;
const int KMP_SCH_STATIC = 34;
const int KMP_SCH_DYNAMIC_CHUNKED = 35;
const int KMP_SCH_GUIDED_CHUNKED = 36;
const int KMP_SCH_RUNTIME = 37;
const int KMP_SCH_AUTO = 38;
const int KMP_ORD_STATIC_CHUNKED = 65;
//const int KMP_ORD_STATIC = 66;
//const int KMP_ORD_DYNAMIC_CHUNKED = 67;
//const int KMP_ORD_GUIDED_CHUNKED = 68;
//const int KMP_ORD_RUNTIME = 69;
//const int KMP_ORD_AUTO = 70;
const int KMP_NM_STATIC_CHUNKED = 161;
//const int KMP_NM_STATIC = 162;
//const int KMP_NM_DYNAMIC_CHUNKED = 163;
//const int KMP_NM_GUIDED_CHUNKED = 164;
//const int KMP_NM_RUNTIME = 165;
//const int KMP_NM_AUTO = 166;
const int KMP_NM_ORD_STATIC_CHUNKED = 193;
//const int KMP_NM_ORD_STATIC = 194;
//const int KMP_NM_ORD_DYNAMIC_CHUNKED = 195;
//const int KMP_NM_ORD_GUIDED_CHUNKED = 196;
//const int KMP_NM_ORD_RUNTIME = 197;
//const int KMP_NM_ORD_AUTO = 198;
const int KMP_SCH_DEFAULT = KMP_SCH_STATIC;
const int SCH_ORD = KMP_ORD_STATIC_CHUNKED - KMP_SCH_STATIC_CHUNKED;
const int SCH_NM = KMP_NM_STATIC_CHUNKED - KMP_SCH_STATIC_CHUNKED;
const int SCH_NM_ORD = KMP_NM_ORD_STATIC_CHUNKED - KMP_SCH_STATIC_CHUNKED;
const int KMP_SCH_DISTRIBUTE_STATIC_CHUNKED = 91;
const int KMP_SCH_DISTRIBUTE_STATIC = 92;
typedef int32_t (*kmp_routine_entry_t)(int32_t, void *);
struct kmp_task_t {};
const int OMP_TASK_UNTIED = 0;
const int OMP_TASK_TIED = 1;
const int OMP_TASK_FINAL = 2;
const int OMP_TASK_CURRENT_QUEUED = 1;
struct kmp_depend_info_t {};
const unsigned char IN = 1;
const unsigned char OUT = 2;
const unsigned char INOUT = 3;
typedef int32_t(__kmpc_omp_task_with_deps)(ident_t *loc, int32_t gtid,
                                           kmp_task_t *task, int32_t ndeps,
                                           kmp_depend_info_t *dep_list,
                                           int32_t ndeps_noalias,
                                           kmp_depend_info_t *noalias_dep_list);
typedef void(__kmpc_omp_wait_deps)(ident_t *loc, int32_t gtid, int32_t ndeps,
                                   kmp_depend_info_t *dep_list,
                                   int32_t ndeps_noalias,
                                   kmp_depend_info_t *noalias_dep_list);
typedef kmp_task_t *(__kmpc_omp_task_alloc)(ident_t *loc, int32_t gtid,
                                            int32_t flags,
                                            size_t sizeof_kmp_task_t,
                                            size_t sizeof_shareds,
                                            kmp_routine_entry_t task_entry);
typedef void(__kmpc_omp_task_begin_if0)(ident_t *loc, int32_t gtid,
                                        kmp_task_t *task);
typedef void(__kmpc_omp_task_complete_if0)(ident_t *loc, int32_t gtid,
                                           kmp_task_t *task);
typedef int32_t(__kmpc_omp_task_parts)(ident_t *loc, int32_t gtid,
                                       kmp_task_t *task);
typedef void(__kmpc_taskgroup)(ident_t *loc, int32_t global_tid);
typedef void(__kmpc_end_taskgroup)(ident_t *loc, int32_t global_tid);
}

namespace llvm {
/// Specializations of llvm::TypeBuilder for:
///   ident_t
template <bool X> class TypeBuilder<ident_t, X> {
public:
  static StructType *get(LLVMContext &C) {
    return StructType::get(
        TypeBuilder<llvm::types::i<32>, X>::get(C),  // reserved_1
        TypeBuilder<llvm::types::i<32>, X>::get(C),  // flags
        TypeBuilder<llvm::types::i<32>, X>::get(C),  // reserved_2
        TypeBuilder<llvm::types::i<32>, X>::get(C),  // reserved_3
        TypeBuilder<llvm::types::i<8> *, X>::get(C), // psource
        NULL);
  }
  enum {
    reserved_1,
    flags,
    reserved_2,
    reserved_3,
    psource
  };
};
///   ident_t
template <bool X> class TypeBuilder<kmp_task_t, X> {
public:
  static StructType *get(LLVMContext &C) {
    return StructType::get(
        TypeBuilder<void *, X>::get(C),              // shareds
        TypeBuilder<kmp_routine_entry_t, X>::get(C), // routine
        TypeBuilder<llvm::types::i<32>, X>::get(C),  // part_id
        TypeBuilder<llvm::types::i<32>, X>::get(C),  // firstprivate_locker
        NULL);
  }
  enum {
    shareds,
    routine,
    part_id,
    firstprivate_locker,
    privates
  };
};
template <typename R, typename A1, typename A2, typename A3, typename A4,
          typename A5, typename A6, typename A7, bool cross>
class TypeBuilder<R(A1, A2, A3, A4, A5, A6, A7), cross> {
public:
  static FunctionType *get(LLVMContext &Context) {
    Type *params[] = { TypeBuilder<A1, cross>::get(Context),
                       TypeBuilder<A2, cross>::get(Context),
                       TypeBuilder<A3, cross>::get(Context),
                       TypeBuilder<A4, cross>::get(Context),
                       TypeBuilder<A5, cross>::get(Context),
                       TypeBuilder<A6, cross>::get(Context),
                       TypeBuilder<A7, cross>::get(Context), };
    return FunctionType::get(TypeBuilder<R, cross>::get(Context), params,
                             false);
  }
};
template <typename R, typename A1, typename A2, typename A3, typename A4,
          typename A5, typename A6, typename A7, typename A8, typename A9,
          bool cross>
class TypeBuilder<R(A1, A2, A3, A4, A5, A6, A7, A8, A9), cross> {
public:
  static FunctionType *get(LLVMContext &Context) {
    Type *params[] = { TypeBuilder<A1, cross>::get(Context),
                       TypeBuilder<A2, cross>::get(Context),
                       TypeBuilder<A3, cross>::get(Context),
                       TypeBuilder<A4, cross>::get(Context),
                       TypeBuilder<A5, cross>::get(Context),
                       TypeBuilder<A6, cross>::get(Context),
                       TypeBuilder<A7, cross>::get(Context),
                       TypeBuilder<A8, cross>::get(Context),
                       TypeBuilder<A9, cross>::get(Context), };
    return FunctionType::get(TypeBuilder<R, cross>::get(Context), params,
                             false);
  }
};
template <typename R, typename A1, typename A2, typename A3, typename A4,
          typename A5, typename A6, bool cross>
class TypeBuilder<R(A1, A2, A3, A4, A5, A6), cross> {
public:
  static FunctionType *get(LLVMContext &Context) {
    Type *params[] = { TypeBuilder<A1, cross>::get(Context),
                       TypeBuilder<A2, cross>::get(Context),
                       TypeBuilder<A3, cross>::get(Context),
                       TypeBuilder<A4, cross>::get(Context),
                       TypeBuilder<A5, cross>::get(Context),
                       TypeBuilder<A6, cross>::get(Context), };
    return FunctionType::get(TypeBuilder<R, cross>::get(Context), params,
                             false);
  }
};
template <bool X> class TypeBuilder<sched_type, X> {
public:
  static IntegerType *get(LLVMContext &C) {
    return TypeBuilder<llvm::types::i<32>, X>::get(C);
  }
};
template <bool X> class TypeBuilder<kmp_proc_bind_t, X> {
public:
  static IntegerType *get(LLVMContext &C) {
    return TypeBuilder<llvm::types::i<32>, X>::get(C);
  }
};

typedef llvm::TypeBuilder<kmp_task_t, false> TaskTBuilder;
typedef llvm::TypeBuilder<kmp_proc_bind_t, false> ProcBindTBuilder;
}

namespace {
// Getters for fields of the loop-like directives. We may want to add a
// common parent to all the loop-like directives to get rid of these.
//

static bool isLoopDirective(const OMPExecutableDirective *ED) {
  return isa<OMPForDirective>(ED) || isa<OMPParallelForDirective>(ED) ||
         isa<OMPParallelForSimdDirective>(ED) || isa<OMPSimdDirective>(ED) ||
         isa<OMPForSimdDirective>(ED);
}


static const Expr *getInitFromLoopDirective(const OMPExecutableDirective *ED) {
  if (const OMPForDirective *D = dyn_cast<OMPForDirective>(ED)) {
    return D->getInit();
  }
  if (const OMPParallelForDirective *D = dyn_cast<OMPParallelForDirective>(ED)) {
    return D->getInit();
  }
  if (const OMPParallelForSimdDirective *D = dyn_cast<OMPParallelForSimdDirective>(ED)) {
    return D->getInit();
  }
  if (const OMPSimdDirective *D = dyn_cast<OMPSimdDirective>(ED)) {
    return D->getInit();
  }
  if (const OMPForSimdDirective *D = dyn_cast<OMPForSimdDirective>(ED)) {
    return D->getInit();
  }
  assert(0 && "bad loop directive");
  return 0;
}

static const Expr *getFinalFromLoopDirective(const OMPExecutableDirective *ED) {
  if (const OMPForDirective *D = dyn_cast<OMPForDirective>(ED)) {
    return D->getFinal();
  }
  if (const OMPParallelForDirective *D = dyn_cast<OMPParallelForDirective>(ED)) {
    return D->getFinal();
  }
  if (const OMPParallelForSimdDirective *D = dyn_cast<OMPParallelForSimdDirective>(ED)) {
    return D->getFinal();
  }
  if (const OMPSimdDirective *D = dyn_cast<OMPSimdDirective>(ED)) {
    return D->getFinal();
  }
  if (const OMPForSimdDirective *D = dyn_cast<OMPForSimdDirective>(ED)) {
    return D->getFinal();
  }
  assert(0 && "bad loop directive");
  return 0;
}

static const Expr *getNewIterVarFromLoopDirective(const OMPExecutableDirective *ED) {
  if (const OMPForDirective *D = dyn_cast<OMPForDirective>(ED)) {
    return D->getNewIterVar();
  }
  if (const OMPParallelForDirective *D = dyn_cast<OMPParallelForDirective>(ED)) {
    return D->getNewIterVar();
  }
  if (const OMPParallelForSimdDirective *D = dyn_cast<OMPParallelForSimdDirective>(ED)) {
    return D->getNewIterVar();
  }
  if (const OMPSimdDirective *D = dyn_cast<OMPSimdDirective>(ED)) {
    return D->getNewIterVar();
  }
  if (const OMPForSimdDirective *D = dyn_cast<OMPForSimdDirective>(ED)) {
    return D->getNewIterVar();
  }
  assert(0 && "bad loop directive");
  return 0;
}

static const Expr *getNewIterEndFromLoopDirective(const OMPExecutableDirective *ED) {
  if (const OMPForDirective *D = dyn_cast<OMPForDirective>(ED)) {
    return D->getNewIterEnd();
  }
  if (const OMPParallelForDirective *D = dyn_cast<OMPParallelForDirective>(ED)) {
    return D->getNewIterEnd();
  }
  if (const OMPParallelForSimdDirective *D = dyn_cast<OMPParallelForSimdDirective>(ED)) {
    return D->getNewIterEnd();
  }
  if (const OMPSimdDirective *D = dyn_cast<OMPSimdDirective>(ED)) {
    return D->getNewIterEnd();
  }
  if (const OMPForSimdDirective *D = dyn_cast<OMPForSimdDirective>(ED)) {
    return D->getNewIterEnd();
  }
  assert(0 && "bad loop directive");
  return 0;
}

static const ArrayRef<Expr *>
getCountersFromLoopDirective(const OMPExecutableDirective *ED) {
  if (const OMPForDirective *D = dyn_cast<OMPForDirective>(ED)) {
    return D->getCounters();
  }
  if (const OMPParallelForDirective *D = dyn_cast<OMPParallelForDirective>(ED)) {
    return D->getCounters();
  }
  if (const OMPParallelForSimdDirective *D = dyn_cast<OMPParallelForSimdDirective>(ED)) {
    return D->getCounters();
  }
  if (const OMPSimdDirective *D = dyn_cast<OMPSimdDirective>(ED)) {
    return D->getCounters();
  }
  if (const OMPForSimdDirective *D = dyn_cast<OMPForSimdDirective>(ED)) {
    return D->getCounters();
  }
  assert(0 && "bad loop directive");
  return 0;
}

static unsigned getCollapsedNumberFromLoopDirective(const OMPExecutableDirective *ED) {
  if (const OMPForDirective *D = dyn_cast<OMPForDirective>(ED)) {
    return D->getCollapsedNumber();
  }
  if (const OMPParallelForDirective *D = dyn_cast<OMPParallelForDirective>(ED)) {
    return D->getCollapsedNumber();
  }
  if (const OMPParallelForSimdDirective *D = dyn_cast<OMPParallelForSimdDirective>(ED)) {
    return D->getCollapsedNumber();
  }
  if (const OMPSimdDirective *D = dyn_cast<OMPSimdDirective>(ED)) {
    return D->getCollapsedNumber();
  }
  if (const OMPForSimdDirective *D = dyn_cast<OMPForSimdDirective>(ED)) {
    return D->getCollapsedNumber();
  }
  assert(0 && "bad loop directive");
  return 0;
}

static const Expr *getLowerBoundFromLoopDirective(const OMPExecutableDirective *ED) {
//  if (const OMPDistributeParallelForDirective *D = dyn_cast<OMPDistributeParallelForDirective>(ED)) {
//    return D->getLowerBound();
//  }
  assert(0 && "bad loop directive");
  return 0;
}

static const Expr *getUpperBoundFromLoopDirective(const OMPExecutableDirective *ED) {
//  if (const OMPDistributeParallelForDirective *D = dyn_cast<OMPDistributeParallelForDirective>(ED)) {
//    return D->getUpperBound();
//  }
  assert(0 && "bad loop directive");
  return 0;
}
}

#define OPENMPRTL_FUNC(name) Get__kmpc_##name(&CGM)
#define OPENMPRTL_ATOMIC_FUNC(QTy, Op) getAtomicFunc(*this, QTy, Op)
#define OPENMPRTL_ATOMIC_FUNC_GENERAL(QTyRes, QTyIn, Aop, Capture, Reverse)    \
  getAtomicFuncGeneral(*this, QTyRes, QTyIn, Aop, Capture, Reverse)

#define DEFAULT_GET_OPENMP_FUNC(name)                                          \
  static llvm::Value *Get__kmpc_##name(clang::CodeGen::CodeGenModule *CGM) {   \
    return CGM->CreateRuntimeFunction(                                         \
        llvm::TypeBuilder<__kmpc_##name, false>::get(CGM->getLLVMContext()),   \
        "__kmpc_" #name);                                                      \
  }

// Special processing for __kmpc_copyprivate
// DEFAULT_GET_OPENMP_FUNC(copyprivate)
static llvm::Value *Get__kmpc_copyprivate(clang::CodeGen::CodeGenModule *CGM) {
  llvm::LLVMContext &C = CGM->getLLVMContext();
  llvm::Type *Params[] = { llvm::TypeBuilder<ident_t *, false>::get(C),
                           llvm::TypeBuilder<int32_t, false>::get(C),
                           CGM->SizeTy,
                           llvm::TypeBuilder<void *, false>::get(C),
                           llvm::TypeBuilder<kmp_reduce_func, false>::get(C),
                           llvm::TypeBuilder<int32_t, false>::get(C) };

  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::TypeBuilder<void, false>::get(C), Params, false);
  return CGM->CreateRuntimeFunction(FT, "__kmpc_copyprivate");
}
// Special processing for __kmpc_reduce_nowait
// DEFAULT_GET_OPENMP_FUNC(reduce_nowait)
static llvm::Value *
Get__kmpc_reduce_nowait(clang::CodeGen::CodeGenModule *CGM) {
  llvm::LLVMContext &C = CGM->getLLVMContext();
  llvm::Type *Params[] = {
    llvm::TypeBuilder<ident_t *, false>::get(C),
    llvm::TypeBuilder<int32_t, false>::get(C),
    llvm::TypeBuilder<int32_t, false>::get(C),
    CGM->SizeTy,
    llvm::TypeBuilder<void *, false>::get(C),
    llvm::TypeBuilder<kmp_copy_func, false>::get(C),
    llvm::TypeBuilder<kmp_critical_name *, false>::get(C)
  };

  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::TypeBuilder<int32_t, false>::get(C), Params, false);
  return CGM->CreateRuntimeFunction(FT, "__kmpc_reduce_nowait");
}
// Special processing for __kmpc_reduce
// DEFAULT_GET_OPENMP_FUNC(reduce)
static llvm::Value *Get__kmpc_reduce(clang::CodeGen::CodeGenModule *CGM) {
  llvm::LLVMContext &C = CGM->getLLVMContext();
  llvm::Type *Params[] = {
    llvm::TypeBuilder<ident_t *, false>::get(C),
    llvm::TypeBuilder<int32_t, false>::get(C),
    llvm::TypeBuilder<int32_t, false>::get(C),
    CGM->SizeTy,
    llvm::TypeBuilder<void *, false>::get(C),
    llvm::TypeBuilder<kmp_copy_func, false>::get(C),
    llvm::TypeBuilder<kmp_critical_name *, false>::get(C)
  };

  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::TypeBuilder<int32_t, false>::get(C), Params, false);
  return CGM->CreateRuntimeFunction(FT, "__kmpc_reduce");
}
// Special processing for __kmpc_omp_task_alloc
// DEFAULT_GET_OPENMP_FUNC(omp_task_alloc)
static llvm::Value *
Get__kmpc_omp_task_alloc(clang::CodeGen::CodeGenModule *CGM) {
  llvm::LLVMContext &C = CGM->getLLVMContext();
  llvm::Type *Params[] = {
    llvm::TypeBuilder<ident_t *, false>::get(C),
    llvm::TypeBuilder<int32_t, false>::get(C),
    llvm::TypeBuilder<int32_t, false>::get(C),
    CGM->SizeTy,
    CGM->SizeTy,
    llvm::TypeBuilder<kmp_routine_entry_t, false>::get(C)
  };

  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::TypeBuilder<kmp_task_t *, false>::get(C), Params, false);
  return CGM->CreateRuntimeFunction(FT, "__kmpc_omp_task_alloc");
}
static llvm::Type *getKMPDependInfoType(CodeGenModule *CGM) {
  llvm::Type *Ty = CGM->OpenMPSupport.getKMPDependInfoType();
  if (Ty)
    return Ty;
  IdentifierInfo *II = &CGM->getContext().Idents.get("__kmp_depend_info_t");
  DeclContext *DC = CGM->getContext().getTranslationUnitDecl();
  RecordDecl *RD = RecordDecl::Create(CGM->getContext(), TTK_Struct, DC,
                                      SourceLocation(), SourceLocation(), II);
  RD->startDefinition();
  DC->addHiddenDecl(RD);
  II = &CGM->getContext().Idents.get("base_addr");
  FieldDecl *FD = FieldDecl::Create(
      CGM->getContext(), RD, SourceLocation(), SourceLocation(), II,
      CGM->getContext().getIntPtrType(),
      CGM->getContext().getTrivialTypeSourceInfo(
          CGM->getContext().getIntPtrType(), SourceLocation()),
      0, false, ICIS_NoInit);
  FD->setAccess(AS_public);
  RD->addDecl(FD);
  II = &CGM->getContext().Idents.get("len");
  FD = FieldDecl::Create(CGM->getContext(), RD, SourceLocation(),
                         SourceLocation(), II, CGM->getContext().getSizeType(),
                         CGM->getContext().getTrivialTypeSourceInfo(
                             CGM->getContext().getSizeType(), SourceLocation()),
                         0, false, ICIS_NoInit);
  FD->setAccess(AS_public);
  RD->addDecl(FD);
  II = &CGM->getContext().Idents.get("flags");
  FD = FieldDecl::Create(CGM->getContext(), RD, SourceLocation(),
                         SourceLocation(), II, CGM->getContext().BoolTy,
                         CGM->getContext().getTrivialTypeSourceInfo(
                             CGM->getContext().BoolTy, SourceLocation()),
                         0, false, ICIS_NoInit);
  FD->setAccess(AS_public);
  RD->addDecl(FD);
  RD->completeDefinition();
  QualType QTy = CGM->getContext().getRecordType(RD);
  Ty = CGM->getTypes().ConvertTypeForMem(QTy);
  CGM->OpenMPSupport.setKMPDependInfoType(Ty,
                                          CGM->getContext().getTypeAlignInChars(QTy).getQuantity());
  return Ty;
}
// Special processing for __kmpc_omp_task_with_deps
// DEFAULT_GET_OPENMP_FUNC(omp_task_with_deps)
static llvm::Value *
Get__kmpc_omp_task_with_deps(clang::CodeGen::CodeGenModule *CGM) {
  llvm::LLVMContext &C = CGM->getLLVMContext();
  llvm::Type *Params[] = { llvm::TypeBuilder<ident_t *, false>::get(C),
                           llvm::TypeBuilder<int32_t, false>::get(C),
                           llvm::TypeBuilder<kmp_task_t *, false>::get(C),
                           llvm::TypeBuilder<int32_t, false>::get(C),
                           getKMPDependInfoType(CGM)->getPointerTo(),
                           llvm::TypeBuilder<int32_t, false>::get(C),
                           getKMPDependInfoType(CGM)->getPointerTo() };

  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::TypeBuilder<int32_t, false>::get(C), Params, false);
  return CGM->CreateRuntimeFunction(FT, "__kmpc_omp_task_with_deps");
}

// Special processing for __kmpc_omp_wait_deps
// DEFAULT_GET_OPENMP_FUNC(omp_wait_deps)
static llvm::Value *
Get__kmpc_omp_wait_deps(clang::CodeGen::CodeGenModule *CGM) {
  llvm::LLVMContext &C = CGM->getLLVMContext();
  llvm::Type *Params[] = { llvm::TypeBuilder<ident_t *, false>::get(C),
                           llvm::TypeBuilder<int32_t, false>::get(C),
                           llvm::TypeBuilder<int32_t, false>::get(C),
                           getKMPDependInfoType(CGM)->getPointerTo(),
                           llvm::TypeBuilder<int32_t, false>::get(C),
                           getKMPDependInfoType(CGM)->getPointerTo() };

  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::TypeBuilder<void, false>::get(C), Params, false);
  return CGM->CreateRuntimeFunction(FT, "__kmpc_omp_wait_deps");
}

enum EAtomicOperation {
  OMP_Atomic_add,
  OMP_Atomic_sub,
  OMP_Atomic_mul,
  OMP_Atomic_div,
  OMP_Atomic_andb,
  OMP_Atomic_shl,
  OMP_Atomic_shr,
  OMP_Atomic_orb,
  OMP_Atomic_xor,
  OMP_Atomic_andl,
  OMP_Atomic_orl,
  OMP_Atomic_max,
  OMP_Atomic_min,
  OMP_Atomic_eqv,
  OMP_Atomic_neqv,
  OMP_Atomic_rd,
  OMP_Atomic_wr,
  OMP_Atomic_swp,
  OMP_Atomic_assign,
  OMP_Atomic_invalid
};

static QualType getAtomicType(CodeGenFunction &CGF, QualType QTy) {
  if (!QTy->isArithmeticType())
    return QualType();
  if (QTy->isRealFloatingType())
    return QTy
        ->getCanonicalTypeUnqualified(); // CGF.ConvertTypeForMem(QTy->getCanonicalTypeUnqualified());
  uint64_t TySize = CGF.getContext().getTypeSize(QTy);
  if (CGF.getContext().getTypeSize(CGF.getContext().CharTy) == TySize)
    return QTy->isUnsignedIntegerOrEnumerationType()
               ? CGF.getContext().UnsignedCharTy
               : CGF.getContext().SignedCharTy;
  else if (CGF.getContext().getTypeSize(CGF.getContext().ShortTy) == TySize)
    return QTy->isUnsignedIntegerOrEnumerationType()
               ? CGF.getContext().UnsignedShortTy
               : CGF.getContext().ShortTy;
  else if (CGF.getContext().getTypeSize(CGF.getContext().IntTy) == TySize)
    return QTy->isUnsignedIntegerOrEnumerationType()
               ? CGF.getContext().UnsignedIntTy
               : CGF.getContext().IntTy;
  else if (CGF.getContext().getTypeSize(CGF.getContext().LongTy) == TySize)
    return QTy->isUnsignedIntegerOrEnumerationType()
               ? CGF.getContext().UnsignedLongTy
               : CGF.getContext().LongTy;
  else if (CGF.getContext().getTypeSize(CGF.getContext().LongLongTy) == TySize)
    return QTy->isUnsignedIntegerOrEnumerationType()
               ? CGF.getContext().UnsignedLongLongTy
               : CGF.getContext().LongLongTy;
  else if (CGF.getContext().getTypeSize(CGF.getContext().Int128Ty) == TySize)
    return QTy->isUnsignedIntegerOrEnumerationType()
               ? CGF.getContext().UnsignedInt128Ty
               : CGF.getContext().Int128Ty;
  return QualType();
}

static llvm::Value *getAtomicFuncGeneral(CodeGenFunction &CGF, QualType QTyRes,
                                         QualType QTyIn, EAtomicOperation Aop,
                                         bool Capture, bool Reverse) {
  SmallString<40> Str;
  llvm::raw_svector_ostream OS(Str);
  int64_t TySize = CGF.CGM.GetTargetTypeStoreSize(CGF.ConvertTypeForMem(QTyRes))
                       .getQuantity();
  if (QTyRes->isRealFloatingType()) {
    OS << "__kmpc_atomic_float";
    if (TySize != 4 && TySize != 8 && TySize != 10 && TySize != 16)
      return 0;
  } else if (QTyRes->isScalarType()) {
    OS << "__kmpc_atomic_fixed";
    if (TySize != 1 && TySize != 2 && TySize != 4 && TySize != 8)
      return 0;
  } else
    return 0;
  OS << TySize;
  switch (Aop) {
  case OMP_Atomic_orl:
    OS << "_orl";
    break;
  case OMP_Atomic_orb:
    OS << "_orb";
    break;
  case OMP_Atomic_andl:
    OS << "_andl";
    break;
  case OMP_Atomic_andb:
    OS << "_andb";
    break;
  case OMP_Atomic_xor:
    OS << "_xor";
    break;
  case OMP_Atomic_sub:
    OS << "_sub";
    break;
  case OMP_Atomic_add:
    OS << "_add";
    break;
  case OMP_Atomic_mul:
    OS << "_mul";
    break;
  case OMP_Atomic_div:
    if (QTyRes->hasUnsignedIntegerRepresentation() || QTyRes->isPointerType()) {
      if (!CGF.getContext().hasSameType(QTyIn, QTyRes))
        return 0;
      OS << "u";
    }
    OS << "_div";
    break;
  case OMP_Atomic_min:
    OS << "_min";
    break;
  case OMP_Atomic_max:
    OS << "_max";
    break;
  case OMP_Atomic_shl:
    OS << "_shl";
    break;
  case OMP_Atomic_shr:
    if (QTyRes->hasUnsignedIntegerRepresentation() || QTyRes->isPointerType()) {
      if (!CGF.getContext().hasSameType(QTyIn, QTyRes))
        return 0;
      OS << "u";
    }
    OS << "_shr";
    break;
  case OMP_Atomic_wr:
    OS << "_wr";
    break;
  case OMP_Atomic_rd:
    OS << "_rd";
    break;
  case OMP_Atomic_assign:
    return 0;
  case OMP_Atomic_invalid:
  default:
    llvm_unreachable("Unknown atomic operation.");
  }
  if (Capture) {
    OS << "_cpt";
    if (!CGF.getContext().hasSameType(QTyIn, QTyRes))
      return 0;
  }
  if (Reverse && (Aop == OMP_Atomic_sub || Aop == OMP_Atomic_div ||
                  Aop == OMP_Atomic_shr || Aop == OMP_Atomic_shl)) {
    OS << "_rev";
    if (!CGF.getContext().hasSameType(QTyIn, QTyRes))
      return 0;
  }
  int64_t TyInSize = CGF.CGM.GetTargetTypeStoreSize(
                                 CGF.ConvertTypeForMem(QTyIn)).getQuantity();
  if (!CGF.getContext().hasSameType(QTyIn, QTyRes)) {
    if (QTyRes->isScalarType() && QTyIn->isRealFloatingType() && TyInSize == 8)
      OS << "_float8";
    else
      return 0;
  }
  SmallVector<llvm::Type *, 5> Params;
  Params.push_back(llvm::TypeBuilder<ident_t, false>::get(
      CGF.CGM.getLLVMContext())->getPointerTo());
  Params.push_back(CGF.Int32Ty);
  llvm::Type *Ty = CGF.ConvertTypeForMem(getAtomicType(CGF, QTyRes));
  Params.push_back(Ty->getPointerTo());
  if (Aop != OMP_Atomic_rd)
    Params.push_back(CGF.ConvertTypeForMem(getAtomicType(CGF, QTyIn)));
  if (Capture) {
    Params.push_back(CGF.Int32Ty);
  }
  llvm::Type *RetTy = CGF.VoidTy;
  if (Capture || Aop == OMP_Atomic_rd)
    RetTy = Ty;
  llvm::FunctionType *FunTy = llvm::FunctionType::get(RetTy, Params, false);
  return CGF.CGM.CreateRuntimeFunction(FunTy, OS.str());
}

static llvm::Value *getAtomicFunc(CodeGenFunction &CGF, QualType QTy,
                                  OpenMPReductionClauseOperator Op) {

  EAtomicOperation Aop = OMP_Atomic_invalid;
  switch (Op) {
  case OMPC_REDUCTION_or:
    Aop = OMP_Atomic_orl;
    break;
  case OMPC_REDUCTION_bitor:
    Aop = OMP_Atomic_orb;
    break;
  case OMPC_REDUCTION_and:
    Aop = OMP_Atomic_andl;
    break;
  case OMPC_REDUCTION_bitand:
    Aop = OMP_Atomic_andb;
    break;
  case OMPC_REDUCTION_bitxor:
    Aop = OMP_Atomic_xor;
    break;
  case OMPC_REDUCTION_sub:
    Aop = OMP_Atomic_add;
    break;
  case OMPC_REDUCTION_add:
    Aop = OMP_Atomic_add;
    break;
  case OMPC_REDUCTION_mult:
    Aop = OMP_Atomic_mul;
    break;
  case OMPC_REDUCTION_min:
    Aop = OMP_Atomic_min;
    break;
  case OMPC_REDUCTION_max:
    Aop = OMP_Atomic_max;
    break;
  case OMPC_REDUCTION_custom:
    return 0;
  case OMPC_REDUCTION_unknown:
  case NUM_OPENMP_REDUCTION_OPERATORS:
    llvm_unreachable("Unknown reduction operation.");
  }
  return getAtomicFuncGeneral(CGF, QTy, QTy, Aop, false, false);
}

DEFAULT_GET_OPENMP_FUNC(fork_call)
DEFAULT_GET_OPENMP_FUNC(push_num_threads)
DEFAULT_GET_OPENMP_FUNC(push_proc_bind)
DEFAULT_GET_OPENMP_FUNC(cancel_barrier)
DEFAULT_GET_OPENMP_FUNC(cancellationpoint)
DEFAULT_GET_OPENMP_FUNC(cancel)
DEFAULT_GET_OPENMP_FUNC(omp_taskyield)
DEFAULT_GET_OPENMP_FUNC(omp_taskwait)
DEFAULT_GET_OPENMP_FUNC(flush)
DEFAULT_GET_OPENMP_FUNC(master)
DEFAULT_GET_OPENMP_FUNC(end_master)
DEFAULT_GET_OPENMP_FUNC(single)
DEFAULT_GET_OPENMP_FUNC(end_single)
DEFAULT_GET_OPENMP_FUNC(critical)
DEFAULT_GET_OPENMP_FUNC(end_critical)
DEFAULT_GET_OPENMP_FUNC(ordered)
DEFAULT_GET_OPENMP_FUNC(end_ordered)
DEFAULT_GET_OPENMP_FUNC(end_reduce_nowait)
DEFAULT_GET_OPENMP_FUNC(end_reduce)
DEFAULT_GET_OPENMP_FUNC(atomic_start)
DEFAULT_GET_OPENMP_FUNC(atomic_end)
DEFAULT_GET_OPENMP_FUNC(dispatch_init_4)
DEFAULT_GET_OPENMP_FUNC(dispatch_init_4u)
DEFAULT_GET_OPENMP_FUNC(dispatch_init_8)
DEFAULT_GET_OPENMP_FUNC(dispatch_init_8u)
DEFAULT_GET_OPENMP_FUNC(dispatch_next_4)
DEFAULT_GET_OPENMP_FUNC(dispatch_next_4u)
DEFAULT_GET_OPENMP_FUNC(dispatch_next_8)
DEFAULT_GET_OPENMP_FUNC(dispatch_next_8u)
DEFAULT_GET_OPENMP_FUNC(dispatch_fini_4)
DEFAULT_GET_OPENMP_FUNC(dispatch_fini_4u)
DEFAULT_GET_OPENMP_FUNC(dispatch_fini_8)
DEFAULT_GET_OPENMP_FUNC(dispatch_fini_8u)
DEFAULT_GET_OPENMP_FUNC(for_static_init_4)
DEFAULT_GET_OPENMP_FUNC(for_static_init_4u)
DEFAULT_GET_OPENMP_FUNC(for_static_init_8)
DEFAULT_GET_OPENMP_FUNC(for_static_init_8u)
DEFAULT_GET_OPENMP_FUNC(for_static_fini)
DEFAULT_GET_OPENMP_FUNC(omp_task_begin_if0)
DEFAULT_GET_OPENMP_FUNC(omp_task_complete_if0)
DEFAULT_GET_OPENMP_FUNC(omp_task_parts)
DEFAULT_GET_OPENMP_FUNC(taskgroup)
DEFAULT_GET_OPENMP_FUNC(end_taskgroup)

namespace {
/// \brief RAII object that save current insert position and then restores it.
class BuilderInsertPositionRAII {
  CGBuilderTy &Builder;
  CGBuilderTy::InsertPoint SavedIP;

public:
  BuilderInsertPositionRAII(CGBuilderTy &Builder,
                            llvm::Instruction *NewInsertPoint)
      : Builder(Builder), SavedIP(Builder.saveIP()) {
    assert(SavedIP.isSet() && "No insertion point is set!");
    Builder.SetInsertPoint(NewInsertPoint);
  }
  ~BuilderInsertPositionRAII() { Builder.restoreIP(SavedIP); }
};

/// \brief RAII object for OpenMP region.
class OpenMPRegionRAII {
  CodeGenFunction &CGF;

public:
  OpenMPRegionRAII(CodeGenFunction &CGF, llvm::Value *Context, const CapturedStmt &CS)
      : CGF(CGF) {
    CGF.InitOpenMPFunction(Context, CS);
  }
  ~OpenMPRegionRAII() { delete CGF.CapturedStmtInfo; }
};

static void SetFirstprivateInsertPt(CodeGenFunction &CGF) {
  if (CGF.FirstprivateInsertPt) {
    llvm::Instruction *Ptr = CGF.FirstprivateInsertPt;
    CGF.FirstprivateInsertPt = 0;
    Ptr->eraseFromParent();
  }
  llvm::Value *Undef = llvm::UndefValue::get(CGF.Int32Ty);
  CGF.FirstprivateInsertPt =
      new llvm::BitCastInst(Undef, CGF.Int32Ty, "",
                            CGF.Builder.GetInsertBlock());
}

static void EmitFirstprivateInsert(CodeGenFunction &CGF, SourceLocation Loc) {
  if (CGF.FirstprivateInsertPt) {
    BuilderInsertPositionRAII PosRAII(CGF.Builder, CGF.FirstprivateInsertPt);
    CodeGenModule &CGM = CGF.CGM;
    CGF.EmitOMPCallWithLocAndTidHelper(OPENMPRTL_FUNC(cancel_barrier),
                                       Loc, KMP_IDENT_BARRIER_IMPL);
  }
}

}

static llvm::GlobalVariable *CreateRuntimeVariable(CodeGenModule &CGM,
                                                   StringRef MangledName,
                                                   llvm::Type *Ty) {
  llvm::PointerType *PtrTy = llvm::PointerType::getUnqual(Ty);
  unsigned AddrSpace = PtrTy->getAddressSpace();
  return new llvm::GlobalVariable(
      CGM.getModule(), Ty, false, llvm::GlobalValue::PrivateLinkage,
      llvm::Constant::getNullValue(Ty), MangledName, 0,
      llvm::GlobalVariable::NotThreadLocal, AddrSpace);
}

void
CodeGenFunction::EmitOMPDirectiveWithParallel(OpenMPDirectiveKind DKind,
                                              OpenMPDirectiveKind SKind,
                                              const OMPExecutableDirective &S) {
  // Generate shared args for captured stmt.
  CapturedStmt *CS = cast<CapturedStmt>(S.getAssociatedStmt());
  llvm::Value *Arg = GenerateCapturedStmtArgument(*CS);

  // Init list of private globals in the stack.
  CGM.OpenMPSupport.startOpenMPRegion(true);
  CGM.OpenMPSupport.setMergeable(false);
  CGM.OpenMPSupport.setOrdered(false);
  CGM.OpenMPSupport.setScheduleChunkSize(KMP_SCH_DEFAULT, 0);

  // CodeGen for clauses (task init).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && !isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitInitOMPClause(*(*I), S);

  // CodeGen for clauses (task init).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && !isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitAfterInitOMPClause(*(*I), S);

  // Generate microtask.
  // void .omp_microtask.(int32_t *, int32_t *, void */*AutoGenRecord **/arg3) {
  //  captured_stmt(arg3);
  // }
  IdentifierInfo *Id = &getContext().Idents.get(".omp_microtask.");
  QualType PtrIntTy = getContext().getPointerType(getContext().IntTy);
  SmallVector<QualType, 4> FnArgTypes;
  FnArgTypes.push_back(PtrIntTy);
  FnArgTypes.push_back(PtrIntTy);
  FnArgTypes.push_back(getContext().VoidPtrTy);
  FunctionProtoType::ExtProtoInfo EPI;
  EPI.ExceptionSpecType = EST_BasicNoexcept;
  QualType FnTy =
      getContext().getFunctionType(getContext().VoidTy, FnArgTypes, EPI);
  TypeSourceInfo *TI =
      getContext().getTrivialTypeSourceInfo(FnTy, SourceLocation());
  FunctionDecl *FD = FunctionDecl::Create(
      getContext(), getContext().getTranslationUnitDecl(), CS->getLocStart(),
      SourceLocation(), Id, FnTy, TI, SC_Static, false, false, false);
  TypeSourceInfo *PtrIntTI =
      getContext().getTrivialTypeSourceInfo(PtrIntTy, SourceLocation());
  TypeSourceInfo *PtrVoidTI = getContext().getTrivialTypeSourceInfo(
      getContext().VoidPtrTy, SourceLocation());
  ParmVarDecl *Arg1 =
      ParmVarDecl::Create(getContext(), FD, SourceLocation(), SourceLocation(),
                          0, PtrIntTy, PtrIntTI, SC_Auto, 0);
  ParmVarDecl *Arg2 =
      ParmVarDecl::Create(getContext(), FD, SourceLocation(), SourceLocation(),
                          0, PtrIntTy, PtrIntTI, SC_Auto, 0);
  ParmVarDecl *Arg3 =
      ParmVarDecl::Create(getContext(), FD, SourceLocation(), SourceLocation(),
                          0, getContext().VoidPtrTy, PtrVoidTI, SC_Auto, 0);
  CodeGenFunction CGF(CGM, true);
  const CGFunctionInfo &FI = getTypes().arrangeFunctionDeclaration(FD);
  llvm::Function *Fn = llvm::Function::Create(getTypes().GetFunctionType(FI),
                                              llvm::GlobalValue::PrivateLinkage,
                                              FD->getName(), &CGM.getModule());
  CGM.SetInternalFunctionAttributes(CurFuncDecl, Fn, FI);
  FunctionArgList FnArgs;
  FnArgs.push_back(Arg1);
  FnArgs.push_back(Arg2);
  FnArgs.push_back(Arg3);
  CGF.OpenMPRoot = OpenMPRoot ? OpenMPRoot : this;
  CGF.StartFunction(FD, getContext().VoidTy, Fn, FI, FnArgs, SourceLocation());
  CGF.Builder.CreateLoad(CGF.GetAddrOfLocalVar(Arg1),
                         ".__kmpc_global_thread_num.");

  // Emit call to the helper function.
  llvm::Value *Arg3Val =
      CGF.Builder.CreateLoad(CGF.GetAddrOfLocalVar(Arg3), "arg3");
  QualType QTy = getContext().getRecordType(CS->getCapturedRecordDecl());
  llvm::Type *ConvertedType =
      CGF.getTypes().ConvertTypeForMem(QTy)->getPointerTo();
  llvm::Value *RecArg =
      CGF.Builder.CreatePointerCast(Arg3Val, ConvertedType, "(anon)arg3");

  // CodeGen for clauses (call start).
  {
    OpenMPRegionRAII OMPRegion(CGF, RecArg, *CS);
    for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                         E = S.clauses().end();
         I != E; ++I)
      if (*I && (!isAllowedClauseForDirective(SKind, (*I)->getClauseKind()) ||
                 (*I)->getClauseKind() == OMPC_firstprivate))
        CGF.EmitPreOMPClause(*(*I), S);
    switch (DKind) {
    case OMPD_parallel:
      CGF.EmitStmt(CS->getCapturedStmt());
      break;
    case OMPD_parallel_sections:
      CGF.EmitOMPSectionsDirective(DKind, SKind, S);
      break;
    case OMPD_parallel_for:
    case OMPD_parallel_for_simd:
      CGF.EmitOMPDirectiveWithLoop(DKind, SKind, S);
      break;
    default:
      break;
    }
    CGF.EnsureInsertPoint();

    // CodeGen for clauses (call end).
    for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                         E = S.clauses().end();
         I != E; ++I)
      if (*I && !isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
        CGF.EmitPostOMPClause(*(*I), S);

    // CodeGen for clauses (closing steps).
    for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                         E = S.clauses().end();
         I != E; ++I)
      if (*I && !isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
        CGF.EmitCloseOMPClause(*(*I), S);
  }

  CGF.EnsureInsertPoint();
  // Implicit barrier for simple parallel region only.
  // Others (combined) directives already has implicit barriers.
  if (DKind == OMPD_parallel) {
    CGF.EmitOMPCallWithLocAndTidHelper(OPENMPRTL_FUNC(cancel_barrier),
                                       S.getLocEnd(), KMP_IDENT_BARRIER_IMPL);
  }

  EmitFirstprivateInsert(CGF, S.getLocStart());

  CGF.FinishFunction();

  // CodeGen for "omp parallel {Associated statement}".
  {
    RunCleanupsScope MainBlock(*this);

    llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
    llvm::Type *KmpcMicroTy =
        llvm::TypeBuilder<kmpc_micro, false>::get(getLLVMContext());
    llvm::Value *RealArgs[] = { Loc, Builder.getInt32(2),
                                CGF.Builder.CreateBitCast(
                                    Fn, KmpcMicroTy, "(kmpc_micro_ty)helper"),
                                Arg };
    // __kmpc_fork_call(&loc, argc/*2*/, microtask, arg);
    EmitRuntimeCall(OPENMPRTL_FUNC(fork_call), makeArrayRef(RealArgs));
  }

  // CodeGen for clauses (task finalize).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && !isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitFinalOMPClause(*(*I), S);

  // Remove list of private globals from the stack.
  CGM.OpenMPSupport.endOpenMPRegion();
}

/// Generate an instructions for '#pragma omp parallel' directive.
void CodeGenFunction::EmitOMPParallelDirective(const OMPParallelDirective &S) {
  EmitOMPDirectiveWithParallel(OMPD_parallel, OMPD_unknown, S);
}

/// Generate an instructions for '#pragma omp parallel for' directive.
void CodeGenFunction::EmitOMPParallelForDirective(const OMPParallelForDirective &S) {
  EmitOMPDirectiveWithParallel(OMPD_parallel_for, OMPD_for, S);
}

/// Generate an instructions for '#pragma omp parallel for simd' directive.
void CodeGenFunction::EmitOMPParallelForSimdDirective(const OMPParallelForSimdDirective &S) {
  EmitOMPDirectiveWithParallel(OMPD_parallel_for_simd, OMPD_for_simd, S);
}

/// Generate an instructions for '#pragma omp parallel sections' directive.
void CodeGenFunction::EmitOMPParallelSectionsDirective(const OMPParallelSectionsDirective &S) {
  EmitOMPDirectiveWithParallel(OMPD_parallel_sections, OMPD_sections, S);
}

/// Generate instruction for OpenMP loop-like directives.
void
CodeGenFunction::EmitOMPDirectiveWithLoop(OpenMPDirectiveKind DKind,
                                          OpenMPDirectiveKind SKind,
                                          const OMPExecutableDirective &S) {
  // Several Simd-specific vars are declared here.
  bool HasSimd = DKind == OMPD_parallel_for_simd || DKind == OMPD_for_simd;
  CGPragmaOmpSimd SimdWrapper(&S);
  llvm::Function *BodyFunction = 0;
  bool SeparateLastIter = false;
  LValue CapStruct;

  // Init list of private globals in the stack.
  CGM.OpenMPSupport.startOpenMPRegion(false);
  CGM.OpenMPSupport.setNoWait(false);
  CGM.OpenMPSupport.setMergeable(true);
  CGM.OpenMPSupport.setOrdered(false);

  // CodeGen for clauses (task init).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitInitOMPClause(*(*I), S);

  // CodeGen for clauses (task init).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitAfterInitOMPClause(*(*I), S);

  bool IsDistributeLoop = false;
//  bool IsDistributeLoop = DKind == OMPD_distribute ||
//                          DKind == OMPD_distribute_simd ||
//                          DKind == OMPD_distribute_parallel_for;
  int Schedule = KMP_SCH_DEFAULT;
  if (!IsDistributeLoop) {
    bool Ordered = CGM.OpenMPSupport.getOrdered();
    bool Merge = CGM.OpenMPSupport.getMergeable();
    int Offset = 0;
    if (Ordered && Merge)
      Offset = SCH_ORD;
    else if (!Ordered && !Merge)
      Offset = SCH_NM;
    else if (Ordered && !Merge)
      Offset = SCH_NM_ORD;
    Schedule += Offset;
  } else {
    Schedule = KMP_SCH_DISTRIBUTE_STATIC;
  }
  CGM.OpenMPSupport.setScheduleChunkSize(Schedule, 0);

  // CodeGen for clauses (call start).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitPreOMPClause(*(*I), S);

  const Expr *ChunkSize;
  CGM.OpenMPSupport.getScheduleChunkSize(Schedule, ChunkSize);
  OpenMPDirectiveKind Kind = S.getDirectiveKind();
//  bool IsComplexParallelLoop = Kind == OMPD_distribute_parallel_for;
  bool IsComplexParallelLoop = false;
  bool IsInnerLoopGen = IsComplexParallelLoop && DKind != Kind;
  bool IsStaticSchedule = Schedule == KMP_SCH_STATIC_CHUNKED ||
                          Schedule == KMP_SCH_STATIC ||
                          Schedule == KMP_SCH_DISTRIBUTE_STATIC_CHUNKED ||
                          Schedule == KMP_SCH_DISTRIBUTE_STATIC;

  // CodeGen for "omp for {Associated statement}".
  {
    llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
    llvm::Value *GTid = CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), *this);
    const Expr *IterVar = getNewIterVarFromLoopDirective(&S);
    QualType QTy = IterVar->getType();
    uint64_t TypeSize = 32;
    if (getContext().getTypeSize(QTy) > TypeSize)
      TypeSize = 64;
    bool isSigned = true;
    if (QTy->hasUnsignedIntegerRepresentation())
      isSigned = false;
    llvm::Type *VarTy = TypeSize == 32 ? Int32Ty : Int64Ty;
    llvm::Value *LB = 0;
    llvm::Value *UB = 0;
    llvm::Value *GlobalUB = 0;
    // Generate loop for inner 'for' directive
    if (IsInnerLoopGen) {
      LB = EmitScalarExpr(getLowerBoundFromLoopDirective(&S));
      UB = EmitScalarExpr(getUpperBoundFromLoopDirective(&S));
    } else {
      LB = llvm::Constant::getNullValue(VarTy);
      UB = EmitScalarExpr(getNewIterEndFromLoopDirective(&S));
    }
    GlobalUB = UB;
#ifdef DEBUG
    llvm::AllocaInst *DebugUB = CreateMemTemp(
        getNewIterEndFromLoopDirective(&S)->getType(), "debug.ub");
    Builder.CreateStore(UB, DebugUB);
#endif
    UB = Builder.CreateIntCast(UB, VarTy, isSigned);
    llvm::Value *Chunk;
    if (ChunkSize) {
      Chunk = EmitScalarExpr(ChunkSize);
      Chunk = Builder.CreateIntCast(Chunk, VarTy, true);
    } else {
      Chunk = llvm::Constant::getNullValue(VarTy);
    }
    llvm::BasicBlock *EndBB = createBasicBlock("omp.loop.end");
    llvm::BasicBlock *OMPLoopBB = 0; // createBasicBlock("omp.loop.begin");
    llvm::AllocaInst *PLast = CreateTempAlloca(Int32Ty, "last");
    PLast->setAlignment(CGM.getDataLayout().getPrefTypeAlignment(Int32Ty));
    InitTempAlloca(PLast,
                   IsStaticSchedule ? Builder.getInt32(1) : Builder.getInt32(0));
    llvm::AllocaInst *PLB = CreateTempAlloca(VarTy, "lb");
    PLB->setAlignment(CGM.getDataLayout().getPrefTypeAlignment(VarTy));
    Builder.CreateStore(LB, PLB);
    llvm::AllocaInst *PUB = CreateTempAlloca(VarTy, "ub");
    PUB->setAlignment(CGM.getDataLayout().getPrefTypeAlignment(VarTy));
    Builder.CreateStore(UB, PUB);
    llvm::AllocaInst *PSt = CreateTempAlloca(VarTy, "st");
    PSt->setAlignment(CGM.getDataLayout().getPrefTypeAlignment(VarTy));
    InitTempAlloca(PSt,
                   TypeSize == 32 ? Builder.getInt32(1) : Builder.getInt64(1));
    llvm::AllocaInst *Private = CreateMemTemp(QTy, ".idx.");
    llvm::Type *IdxTy =
        cast<llvm::PointerType>(Private->getType())->getElementType();
    llvm::BasicBlock *MainBB;
    llvm::BasicBlock *FiniBB = 0;

    const Stmt *Body = S.getAssociatedStmt();
    ArrayRef<Expr *> Arr = getCountersFromLoopDirective(&S);

    if (const CapturedStmt *CS = dyn_cast_or_null<CapturedStmt>(Body))
      Body = CS->getCapturedStmt();
    for (unsigned I = 0; I < getCollapsedNumberFromLoopDirective(&S); ++I) {
      const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(Arr[I])->getDecl());
      bool SkippedContainers = false;
      while (!SkippedContainers) {
        if (const AttributedStmt *AS = dyn_cast_or_null<AttributedStmt>(Body))
          Body = AS->getSubStmt();
        else if (const CompoundStmt *CS =
                     dyn_cast_or_null<CompoundStmt>(Body)) {
          if (CS->size() != 1) {
            SkippedContainers = true;
          } else {
            Body = CS->body_back();
          }
        } else
          SkippedContainers = true;
      }
      const ForStmt *For = dyn_cast_or_null<ForStmt>(Body);
      Body = For->getBody();
      if (CGM.OpenMPSupport.getTopOpenMPPrivateVar(VD))
        continue;
      QualType QTy = Arr[I]->getType();
      llvm::AllocaInst *Private =
          CreateMemTemp(QTy, CGM.getMangledName(VD) + ".private.");
      CGM.OpenMPSupport.addOpenMPPrivateVar(VD, Private);
    }
    while (const CapturedStmt *CS = dyn_cast_or_null<CapturedStmt>(Body))
      Body = CS->getCapturedStmt();
    const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(IterVar)->getDecl());
    CGM.OpenMPSupport.addOpenMPPrivateVar(VD, Private);

    if (IsStaticSchedule) {
      llvm::Value *RealArgs[] = { Loc, GTid, Builder.getInt32(Schedule), PLast,
                                  PLB, PUB, PSt,
                                  TypeSize == 32 ? Builder.getInt32(1)
                                                : Builder.getInt64(1),
                                  Chunk };
      if (TypeSize == 32 && isSigned)
        EmitRuntimeCall(OPENMPRTL_FUNC(for_static_init_4), RealArgs);
      else if (TypeSize == 32 && !isSigned)
        EmitRuntimeCall(OPENMPRTL_FUNC(for_static_init_4u), RealArgs);
      else if (TypeSize == 64 && isSigned)
        EmitRuntimeCall(OPENMPRTL_FUNC(for_static_init_8), RealArgs);
      else
        EmitRuntimeCall(OPENMPRTL_FUNC(for_static_init_8u), RealArgs);
      OMPLoopBB = createBasicBlock("omp.loop.begin");
      EmitBlock(OMPLoopBB);
      LB = Builder.CreateLoad(PLB);
      Builder.CreateStore(LB, Private);
      UB = Builder.CreateLoad(PUB);
      llvm::Value *Cond = Builder.CreateICmp(isSigned ? llvm::CmpInst::ICMP_SLT
                                                      : llvm::CmpInst::ICMP_ULT,
                                             UB, GlobalUB);
      UB = Builder.CreateSelect(Cond, UB, GlobalUB);
      Builder.CreateStore(UB, PUB);
      MainBB = createBasicBlock("omp.loop.main");
      FiniBB = createBasicBlock("omp.loop.fini");
    } else {
      llvm::IntegerType *SchedTy =
          llvm::TypeBuilder<sched_type, false>::get(getLLVMContext());
      llvm::Value *RealArgs[] = { Loc, GTid,
                                  llvm::ConstantInt::get(SchedTy, Schedule),
                                  LB, UB,
                                  TypeSize == 32 ? Builder.getInt32(1)
                                                    : Builder.getInt64(1),
                                  Chunk };
      // __kmpc_dispatch_init{4, 8}(&loc, gtid, sched_type, lb, ub, st, chunk);
      if (TypeSize == 32 && isSigned)
        EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_init_4), RealArgs);
      else if (TypeSize == 32 && !isSigned)
        EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_init_4u), RealArgs);
      else if (TypeSize == 64 && isSigned)
        EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_init_8), RealArgs);
      else
        EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_init_8u), RealArgs);
      llvm::Value *RealArgsNext[] = { Loc, GTid, PLast, PLB, PUB, PSt };
      OMPLoopBB = createBasicBlock("omp.loop.begin");
      EmitBlock(OMPLoopBB);
      llvm::Value *CallRes;
      if (TypeSize == 32 && isSigned)
        CallRes =
            EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_next_4), RealArgsNext);
      else if (TypeSize == 32 && !isSigned)
        CallRes =
            EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_next_4u), RealArgsNext);
      else if (TypeSize == 64 && isSigned)
        CallRes =
            EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_next_8), RealArgsNext);
      else
        CallRes =
            EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_next_8u), RealArgsNext);
      llvm::BasicBlock *OMPInitBB = createBasicBlock("omp.loop.init");
      llvm::SwitchInst *Switch = Builder.CreateSwitch(
          Builder.CreateIntCast(CallRes, Int32Ty, false), EndBB, 1);
      Switch->addCase(llvm::ConstantInt::get(Int32Ty, 1), OMPInitBB);
      EmitBranch(OMPInitBB);
      EmitBlock(OMPInitBB);
      LB = Builder.CreateLoad(PLB);
      UB = Builder.CreateLoad(PUB);
      Builder.CreateStore(LB, Private);
      MainBB = createBasicBlock("omp.loop.main");
      FiniBB = createBasicBlock("omp.loop.fini");
    }
    if (HasSimd) {
      // Update vectorizer width on the loop stack.
      SeparateLastIter = SimdWrapper.emitSafelen(this);

      if (SeparateLastIter) {
        // Emit the following for the lastprivate vars update:
        //   --UB;
        // It is unclear if putting it under "if (*PLast)" will be
        // more or less efficient, this needs to be investigated.
        UB = Builder.CreateSub(UB, llvm::ConstantInt::get(UB->getType(), 1));
        Builder.CreateStore(UB, PUB);
      }

      // Initialize the captured struct.
      CapStruct = InitCapturedStruct(*SimdWrapper.getAssociatedStmt());
    }

    EmitBranch(MainBB);
    EmitBlock(MainBB);

    if (IsStaticSchedule) {
      llvm::Value *Cond = Builder.CreateICmp(isSigned ? llvm::CmpInst::ICMP_SLE
                                                      : llvm::CmpInst::ICMP_ULE,
                                             LB, GlobalUB);
      llvm::BasicBlock *ContBB  = createBasicBlock("omp.lb.le.global_ub.");
      Builder.CreateCondBr(Cond, ContBB, EndBB);
      EmitBlock(ContBB);
    }

    if (HasSimd) {
      // Push current LoopInfo onto the LoopStack.
      LoopStack.Push(MainBB);
    }

    {
      RunCleanupsScope ThenScope(*this);
      EmitStmt(getInitFromLoopDirective(&S));
#ifdef DEBUG
      // CodeGen for clauses (call start).
      for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                           E = S.clauses().end();
           I != E; ++I)
        if (const OMPLastPrivateClause *Clause =
                dyn_cast_or_null<OMPLastPrivateClause>(*I)) {
          for (OMPLastPrivateClause::varlist_const_iterator
                   I1 = Clause->varlist_begin(),
                   E1 = Clause->varlist_end();
               I1 != E1; ++I1) {
            const VarDecl *VD =
                cast<VarDecl>(cast<DeclRefExpr>(*I1)->getDecl());
            if (VD->getName() == "IDX")
              CGM.OpenMPSupport.addOpenMPPrivateVar(VD, Private);
            else if (VD->getName() == "UB")
              CGM.OpenMPSupport.addOpenMPPrivateVar(VD, DebugUB);
            else if (VD->getName() == "LUB")
              CGM.OpenMPSupport.addOpenMPPrivateVar(VD, PUB);
            else if (VD->getName() == "LLB")
              CGM.OpenMPSupport.addOpenMPPrivateVar(VD, PLB);
          }
        }
#endif
      llvm::Value *Idx = Builder.CreateLoad(Private, ".idx.");
      llvm::BasicBlock *UBLBCheckBB = createBasicBlock("omp.lb_ub.check_pass");
      UB = Builder.CreateLoad(PUB);
      llvm::Value *UBLBCheck =
          isSigned ? Builder.CreateICmpSLE(Idx, UB, "omp.idx.le.ub")
                   : Builder.CreateICmpULE(Idx, UB, "omp.idx.le.ub");
      // llvm::BasicBlock *PrevBB = Builder.GetInsertBlock();
      Builder.CreateCondBr(UBLBCheck, UBLBCheckBB, FiniBB);
      EmitBlock(UBLBCheckBB);
      llvm::BasicBlock *ContBlock = createBasicBlock("omp.cont.block");

      BreakContinueStack.push_back(
          BreakContinue(getJumpDestInCurrentScope(EndBB),
                        getJumpDestInCurrentScope(ContBlock)));
      if (HasSimd) {
        RunCleanupsScope Scope(*this);
        BodyFunction = EmitSimdFunction(SimdWrapper);
        EmitSIMDForHelperCall(BodyFunction, CapStruct, Private, false);
      } else {
        RunCleanupsScope Scope(*this);
        if (IsInnerLoopGen || !IsComplexParallelLoop) {
          EmitStmt(Body);
        } else {
          const Expr *LowerBound = getLowerBoundFromLoopDirective(&S);
          const Expr *UpperBound = getUpperBoundFromLoopDirective(&S);
          EmitStoreOfScalar(Builder.CreateLoad(PLB), EmitLValue(LowerBound));
          EmitStoreOfScalar(Builder.CreateLoad(PUB), EmitLValue(UpperBound));
          // Special codegen for distribute parallel for [simd] constructs
          EmitOMPDirectiveWithParallel(OMPD_parallel_for, OMPD_for, S);
        }
      }
      BreakContinueStack.pop_back();
      EnsureInsertPoint();
      EmitBranch(ContBlock);
      EmitBlock(ContBlock);
      Idx = Builder.CreateLoad(Private, ".idx.");
      llvm::Value *NextIdx = Builder.CreateAdd(
          Idx, llvm::ConstantInt::get(IdxTy, 1), ".next.idx.",
          false, QTy->isSignedIntegerOrEnumerationType());
      Builder.CreateStore(NextIdx, Private);
      //      for(llvm::SmallVector<const Expr *, 16>::const_iterator II =
      // Incs.begin(),
      //                                                              EE =
      // Incs.end();
      //          II != EE; ++II) {
      //        EmitIgnoredExpr(*II);
      //        EnsureInsertPoint();
      //      }
      EmitBranch(MainBB);
      if (HasSimd) {
        LoopStack.Pop();
      }
      EmitBlock(FiniBB);
      if (!IsStaticSchedule) {
        llvm::Value *RealArgsFini[] = { Loc, GTid };
        if (TypeSize == 32 && isSigned)
          EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_fini_4), RealArgsFini);
        else if (TypeSize == 32 && !isSigned)
          EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_fini_4u), RealArgsFini);
        else if (TypeSize == 64 && isSigned)
          EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_fini_8), RealArgsFini);
        else
          EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_fini_8u), RealArgsFini);

      } else {
        if (ChunkSize != 0) {
          llvm::Value *St = Builder.CreateLoad(PSt);
          LB = Builder.CreateLoad(PLB);
          LB = Builder.CreateAdd(LB, St);
          Builder.CreateStore(LB, PLB);
          UB = Builder.CreateLoad(PUB);
          UB = Builder.CreateAdd(UB, St);
          Builder.CreateStore(UB, PUB);
        }
      }
      if (SeparateLastIter) {
        // Emit the following for the lastprivate vars update:
        //   call __simd_helper(cs, idx, 1)
        //
        EmitSIMDForHelperCall(BodyFunction, CapStruct, Private, true);
      }
      EmitBranch(!IsStaticSchedule || ChunkSize != 0 ? OMPLoopBB : EndBB);
      // EmitStmt(getInitFromLoopDirective(&S));
      // EnsureInsertPoint();
      // UBLBCheck = isSigned ?
      //                     Builder.CreateICmpSLE(NextIdx, UB, "omp.idx.le.ub")
      // :
      //                     Builder.CreateICmpULE(NextIdx, UB,
      // "omp.idx.le.ub");
      // PrevBB = Builder.GetInsertBlock();
      // Builder.CreateCondBr(UBLBCheck, UBLBCheckBB, OMPLoopBB);
    }
    EmitBlock(EndBB, true);
    if (IsStaticSchedule) {
      llvm::Value *RealArgsFini[] = { Loc, GTid };
      EmitRuntimeCall(OPENMPRTL_FUNC(for_static_fini), RealArgsFini);
    }
    CGM.OpenMPSupport.setLastIterVar(PLast);
  }

  if (!IsDistributeLoop &&
      (CGM.OpenMPSupport.hasLastPrivate() || !CGM.OpenMPSupport.getNoWait()))
    EmitOMPCallWithLocAndTidHelper(OPENMPRTL_FUNC(cancel_barrier),
                                   S.getLocEnd(), KMP_IDENT_BARRIER_IMPL_FOR);
  // CodeGen for clauses (call end).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitPostOMPClause(*(*I), S);

  // CodeGen for clauses (closing steps).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitCloseOMPClause(*(*I), S);

  // CodeGen for clauses (task finalize).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitFinalOMPClause(*(*I), S);

  EnsureInsertPoint();

  // Remove list of private globals from the stack.
  CGM.OpenMPSupport.endOpenMPRegion();

  if (HasSimd) {
    // Emit the final values of 'linear' variables.
    SimdWrapper.emitLinearFinal(*this);
  }
}

/// Generate an instructions for '#pragma omp for' directive.
void CodeGenFunction::EmitOMPForDirective(const OMPForDirective &S) {
  EmitOMPDirectiveWithLoop(OMPD_for, OMPD_for, S);
}

static void EmitUntiedPartIdInc(CodeGenFunction &CGF) {
  if (CGF.CGM.OpenMPSupport.getUntied()) {
    llvm::Value *PartIdAddr;
    llvm::Value *UntiedSwitch;
    llvm::BasicBlock *UntiedEnd;
    unsigned UntiedCounter;
    CGF.CGM.OpenMPSupport.getUntiedData(PartIdAddr, UntiedSwitch, UntiedEnd,
                                        UntiedCounter);
    ++UntiedCounter;
    CGF.Builder.CreateStore(CGF.Builder.getInt32(UntiedCounter), PartIdAddr);
    CGF.CGM.OpenMPSupport.setUntiedData(PartIdAddr, UntiedSwitch, UntiedEnd,
                                        UntiedCounter);
  }
}

static void EmitUntiedBranchEnd(CodeGenFunction &CGF) {
  if (CGF.CGM.OpenMPSupport.getUntied()) {
    llvm::Value *PartIdAddr;
    llvm::Value *UntiedSwitch;
    llvm::BasicBlock *UntiedEnd;
    unsigned UntiedCounter;
    CGF.CGM.OpenMPSupport.getUntiedData(PartIdAddr, UntiedSwitch, UntiedEnd,
                                        UntiedCounter);
    CGF.EmitBranch(UntiedEnd);
  }
}

static void EmitUntiedTaskSwitch(CodeGenFunction &CGF, bool EmitBranch) {
  if (CGF.CGM.OpenMPSupport.getUntied()) {
    llvm::Value *PartIdAddr;
    llvm::Value *UntiedSwitch;
    llvm::BasicBlock *UntiedEnd;
    unsigned UntiedCounter;
    CGF.CGM.OpenMPSupport.getUntiedData(PartIdAddr, UntiedSwitch, UntiedEnd,
                                        UntiedCounter);
    llvm::BasicBlock *NextBlock = CGF.createBasicBlock("untied.sw.next");
    cast<llvm::SwitchInst>(UntiedSwitch)
        ->addCase(CGF.Builder.getInt32(UntiedCounter), NextBlock);
    if (EmitBranch)
      CGF.EmitBranch(NextBlock);
    CGF.EmitBlock(NextBlock);
  }
}

namespace {
  struct CallStackRestore : EHScopeStack::Cleanup {
    llvm::Value *Stack;
    CallStackRestore(llvm::Value *Stack) : Stack(Stack) {}
    void Emit(CodeGenFunction &CGF, Flags flags) {
      llvm::Value *V = CGF.Builder.CreateLoad(Stack);
      llvm::Value *F = CGF.CGM.getIntrinsic(llvm::Intrinsic::stackrestore);
      CGF.Builder.CreateCall(F, V);
    }
  };
}

static std::pair<llvm::Value *, unsigned>
ProcessDependAddresses(CodeGenFunction &CGF, const OMPTaskDirective &S) {
  CodeGenModule &CGM = CGF.CGM;

  llvm::Value *DependenceAddresses = 0;
  unsigned ArraySize = 0;

  SmallVector<const OMPDependClause *, 16> DependClauses;
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I) {
    if (OMPDependClause *ODC = dyn_cast_or_null<OMPDependClause>(*I)) {
      ArraySize += ODC->varlist_size();
      DependClauses.push_back(ODC);
    }
  }
  if (ArraySize > 0) {
    llvm::Type *IntPtrTy =
        CGF.ConvertTypeForMem(CGF.getContext().getIntPtrType());
    llvm::Type *BoolTy = CGF.ConvertTypeForMem(CGF.getContext().BoolTy);
    llvm::Type *DepTy = getKMPDependInfoType(&CGM);
    llvm::ArrayType *DepListTy = llvm::ArrayType::get(DepTy, ArraySize);

    llvm::AllocaInst *Addresses = CGF.CreateTempAlloca(DepListTy, ".dep.list.");
    Addresses->setAlignment(CGM.OpenMPSupport.getKMPDependInfoTypeAlign());
    DependenceAddresses = CGF.Builder.CreateConstInBoundsGEP2_32(Addresses, 0, 0);

    unsigned FieldCounter = 0;
    for (SmallVectorImpl<const OMPDependClause *>::iterator
             I = DependClauses.begin(),
             E = DependClauses.end();
         I != E; ++I) {
      unsigned DepType = IN;
      switch ((*I)->getType()) {
      case OMPC_DEPEND_in:
        DepType = IN;
        break;
      case OMPC_DEPEND_out:
        DepType = OUT;
        break;
      case OMPC_DEPEND_inout:
        DepType = INOUT;
        break;
      case OMPC_DEPEND_unknown:
      case NUM_OPENMP_DEPENDENCE_TYPE:
        llvm_unreachable("Unknown kind of dependency");
        break;
      }
      for (unsigned i = 0, e = (*I)->varlist_size(); i < e; ++i, ++FieldCounter) {
        llvm::Value *DepElPtr =
            CGF.Builder.CreateConstInBoundsGEP2_32(Addresses, 0, FieldCounter);
        // [CounterVal].base_addr = &expr;
        llvm::Value *DepBaseAddr =
            CGF.Builder.CreateConstGEP2_32(DepElPtr, 0, 0);
        llvm::Value *BaseAddr =
            CGF.EmitAnyExpr((*I)->getBegins(i)).getScalarVal();
        BaseAddr = CGF.Builder.CreatePointerCast(BaseAddr, IntPtrTy);
        CGF.Builder.CreateStore(BaseAddr, DepBaseAddr);
        // [CounterVal].len = size;
        llvm::Value *DepLen =
            CGF.Builder.CreateConstGEP2_32(DepElPtr, 0, 1);
        const Expr *Size = (*I)->getSizeInBytes(i);
        if (Size->getType()->isAnyPointerType()) {
          // Size is not a size, but the ending pointer
          // Calculate the real size
          llvm::Value *EndAddr = CGF.EmitScalarExpr(Size);
          llvm::Value *BaseVal = CGF.Builder.CreatePtrToInt(BaseAddr, CGF.SizeTy);
          llvm::Value *EndVal = CGF.Builder.CreatePtrToInt(EndAddr, CGF.SizeTy);
          llvm::Value *Cond = CGF.Builder.CreateICmpUGT(EndVal, BaseVal);
          llvm::Value *Res = CGF.Builder.CreateSelect(Cond,
                                                      CGF.Builder.CreateSub(EndVal, BaseVal),
                                                      llvm::Constant::getNullValue(CGF.SizeTy));
          CGF.Builder.CreateStore(Res, DepLen);
        } else {
          CGF.Builder.CreateStore(CGF.EmitScalarExpr(Size), DepLen);
        }
        // [CounterVal].flags = size;
        llvm::Value *DepFlags =
            CGF.Builder.CreateConstGEP2_32(DepElPtr, 0, 2);
        CGF.Builder.CreateStore(llvm::ConstantInt::get(BoolTy, DepType),
                                DepFlags);
      }
    }
  } else {
    llvm::Type *DepTy = getKMPDependInfoType(&CGM);
    DependenceAddresses = llvm::Constant::getNullValue(DepTy->getPointerTo());
  }
  return std::make_pair(DependenceAddresses, ArraySize);
}

/// Generate an instructions for '#pragma omp task' directive.
void CodeGenFunction::EmitOMPTaskDirective(const OMPTaskDirective &S) {
  // Generate shared args for captured stmt.
  CapturedStmt *CS = cast<CapturedStmt>(S.getAssociatedStmt());
  llvm::Value *Arg = GenerateCapturedStmtArgument(*CS);

  // Init list of private globals in the stack.
  CGM.OpenMPSupport.startOpenMPRegion(true);
  CGM.OpenMPSupport.setMergeable(false);
  CGM.OpenMPSupport.setOrdered(false);
  CGM.OpenMPSupport.setUntied(false);
  CGM.OpenMPSupport.setScheduleChunkSize(KMP_SCH_DEFAULT, 0);

  RecordDecl *RD;
  if (!getContext().getLangOpts().CPlusPlus)
    RD = RecordDecl::Create(getContext(), TTK_Struct,
                            getContext().getTranslationUnitDecl(),
                            SourceLocation(), SourceLocation(),
                            &getContext().Idents.get(".omp.task.priv."));
  else
    RD = CXXRecordDecl::Create(getContext(), TTK_Struct,
                               getContext().getTranslationUnitDecl(),
                               SourceLocation(), SourceLocation(),
                               &getContext().Idents.get(".omp.task.priv."));
  RD->startDefinition();
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I) {
    if (OMPPrivateClause *C = dyn_cast_or_null<OMPPrivateClause>(*I)) {
      for (OMPPrivateClause::varlist_const_iterator II = C->varlist_begin(),
                                                    EE = C->varlist_end();
           II != EE; ++II) {
        const ValueDecl *D = cast<DeclRefExpr>(*II)->getDecl();
        FieldDecl *FD = FieldDecl::Create(
            getContext(), RD, SourceLocation(), SourceLocation(),
            D->getIdentifier(), (*II)->getType(), 0, 0, false, ICIS_NoInit);
        FD->setAccess(AS_public);
        RD->addDecl(FD);
        CGM.OpenMPSupport.getTaskFields()[D] = FD;
      }
    } else if (OMPFirstPrivateClause *C =
                   dyn_cast_or_null<OMPFirstPrivateClause>(*I)) {
      for (OMPFirstPrivateClause::varlist_const_iterator
               II = C->varlist_begin(),
               EE = C->varlist_end();
           II != EE; ++II) {
        const ValueDecl *D = cast<DeclRefExpr>(*II)->getDecl();
        FieldDecl *FD = FieldDecl::Create(
            getContext(), RD, SourceLocation(), SourceLocation(),
            D->getIdentifier(), (*II)->getType(), 0, 0, false, ICIS_NoInit);
        FD->setAccess(AS_public);
        RD->addDecl(FD);
        CGM.OpenMPSupport.getTaskFields()[D] = FD;
      }
    }
  }
  RD->completeDefinition();
  QualType PrivateRecord = getContext().getRecordType(RD);
  llvm::Type *LPrivateTy = getTypes().ConvertTypeForMem(PrivateRecord);

  //  llvm::Type *PTaskFnTy = llvm::TypeBuilder<kmp_routine_entry_t,
  // false>::get(getLLVMContext());
  //  llvm::AllocaInst *FnPtr = CreateTempAlloca(PTaskFnTy);
  //  FnPtr->setAlignment(llvm::ConstantExpr::getAlignOf(PTaskFnTy));

  // CodeGen for clauses (task init).
  llvm::AllocaInst *Flags = CreateMemTemp(getContext().IntTy, ".flags.addr");
  CGM.OpenMPSupport.setTaskFlags(Flags);

  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I)
      EmitInitOMPClause(*(*I), S);

  InitTempAlloca(Flags, Builder.getInt32(CGM.OpenMPSupport.getUntied()
                                             ? OMP_TASK_UNTIED
                                             : OMP_TASK_TIED));

  // Generate microtask.
  // int32 .omp_ptask.(int32_t arg1, void */*kmp_task_t **/arg2) {
  //  captured_stmt(arg2->shareds);
  // }
  IdentifierInfo *Id = &getContext().Idents.get(".omp_ptask.");
  SmallVector<QualType, 2> FnArgTypes;
  FnArgTypes.push_back(getContext().IntTy);
  FnArgTypes.push_back(getContext().VoidPtrTy);
  FunctionProtoType::ExtProtoInfo EPI;
  EPI.ExceptionSpecType = EST_BasicNoexcept;
  QualType FnTy =
      getContext().getFunctionType(getContext().IntTy, FnArgTypes, EPI);
  TypeSourceInfo *TI =
      getContext().getTrivialTypeSourceInfo(FnTy, SourceLocation());
  FunctionDecl *FD = FunctionDecl::Create(
      getContext(), getContext().getTranslationUnitDecl(), CS->getLocStart(),
      SourceLocation(), Id, FnTy, TI, SC_Static, false, false, false);
  TypeSourceInfo *IntTI = getContext().getTrivialTypeSourceInfo(
      getContext().IntTy, SourceLocation());
  TypeSourceInfo *PtrVoidTI = getContext().getTrivialTypeSourceInfo(
      getContext().VoidPtrTy, SourceLocation());
  ParmVarDecl *Arg1 =
      ParmVarDecl::Create(getContext(), FD, SourceLocation(), SourceLocation(),
                          0, getContext().IntTy, IntTI, SC_Auto, 0);
  ParmVarDecl *Arg2 =
      ParmVarDecl::Create(getContext(), FD, SourceLocation(), SourceLocation(),
                          0, getContext().VoidPtrTy, PtrVoidTI, SC_Auto, 0);
  CodeGenFunction CGF(CGM, true);
  const CGFunctionInfo &FI = getTypes().arrangeFunctionDeclaration(FD);
  llvm::Function *Fn = llvm::Function::Create(getTypes().GetFunctionType(FI),
                                              llvm::GlobalValue::PrivateLinkage,
                                              FD->getName(), &CGM.getModule());
  CGM.SetInternalFunctionAttributes(CurFuncDecl, Fn, FI);
  FunctionArgList FnArgs;
  FnArgs.push_back(Arg1);
  FnArgs.push_back(Arg2);
  CGF.OpenMPRoot = OpenMPRoot ? OpenMPRoot : this;
  CGF.StartFunction(FD, getContext().IntTy, Fn, FI, FnArgs, SourceLocation());
  llvm::AllocaInst *GTid =
      CGF.CreateMemTemp(getContext().IntTy, ".__kmpc_global_thread_num.");
  CGF.EmitStoreOfScalar(CGF.Builder.CreateLoad(CGF.GetAddrOfLocalVar(Arg1)),
                        MakeNaturalAlignAddrLValue(GTid, getContext().IntTy),
                        false);
  llvm::Type *TaskTTy = llvm::TaskTBuilder::get(getLLVMContext());
  llvm::Value *TaskTPtr = CGF.Builder.CreatePointerCast(
      CGF.GetAddrOfLocalVar(Arg2), TaskTTy->getPointerTo()->getPointerTo());

  // Emit call to the helper function.
  llvm::Value *Addr = CGF.Builder.CreateConstInBoundsGEP2_32(
      CGF.Builder.CreateLoad(TaskTPtr, ".arg2.shareds"), 0,
      llvm::TaskTBuilder::shareds, ".arg2.shareds.addr");
  llvm::Value *Arg2Val = CGF.Builder.CreateLoad(Addr, ".arg2.shareds.");
  QualType QTy = getContext().getRecordType(CS->getCapturedRecordDecl());
  llvm::Type *ConvertedType =
      CGF.getTypes().ConvertTypeForMem(QTy)->getPointerTo();
  llvm::Value *RecArg =
      CGF.Builder.CreatePointerCast(Arg2Val, ConvertedType, "(anon)shared");

  llvm::Value *Locker = CGF.Builder.CreateConstGEP1_32(
      CGF.Builder.CreateLoad(TaskTPtr), 1);
  CGM.OpenMPSupport.setPTask(Fn, Arg2Val, LPrivateTy, PrivateRecord, Locker);
  // CodeGen for clauses (call start).
  {
    OpenMPRegionRAII OMPRegion(CGF, RecArg, *CS);
    for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                         E = S.clauses().end();
         I != E; ++I)
      if (*I)
        CGF.EmitPreOMPClause(*(*I), S);

    if (CGM.OpenMPSupport.getParentUntied()) {
      // CodeGen for 'depend' clause.
      llvm::Value *DependenceAddresses = 0;
      unsigned ArraySize = 0;

      std::tie(DependenceAddresses, ArraySize) =
          ProcessDependAddresses(CGF, S);
      if (ArraySize != 0) {
        llvm::Type *PtrDepTy = getKMPDependInfoType(&CGM)->getPointerTo();
        llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), CGF);
        llvm::Value *GTid = CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), CGF);
        llvm::Value *RealArgs1[] = {
          Loc, GTid,
          llvm::ConstantInt::get(Int32Ty, ArraySize),
          DependenceAddresses,
          llvm::ConstantInt::get(Int32Ty, 0),
          llvm::Constant::getNullValue(PtrDepTy)
        };
        CGF.EmitRuntimeCall(OPENMPRTL_FUNC(omp_wait_deps), RealArgs1);
      }
    }
    llvm::BasicBlock *UntiedEnd = 0;
    if (CGM.OpenMPSupport.getUntied()) {
      llvm::Value *Addr = CGF.Builder.CreateConstInBoundsGEP2_32(
          CGF.Builder.CreateLoad(TaskTPtr, ".arg2.part_id."), 0,
          llvm::TaskTBuilder::part_id, ".part_id.addr");
      llvm::Value *PartId = CGF.Builder.CreateLoad(Addr, ".part_id.");
      UntiedEnd = CGF.createBasicBlock("untied.sw.end");
      llvm::SwitchInst *UntiedSwitch =
          CGF.Builder.CreateSwitch(PartId, UntiedEnd);
      llvm::BasicBlock *InitBlock = CGF.createBasicBlock("untied.sw.init");
      CGF.EmitBlock(InitBlock);
      UntiedSwitch->addCase(CGF.Builder.getInt32(0), InitBlock);
      CGM.OpenMPSupport.setUntiedData(Addr, UntiedSwitch, UntiedEnd, 0);
    }
    CGF.EmitStmt(CS->getCapturedStmt());
    CGF.EnsureInsertPoint();
    if (UntiedEnd)
      CGF.EmitBlock(UntiedEnd);

    // CodeGen for clauses (call end).
    for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                         E = S.clauses().end();
         I != E; ++I)
      if (*I)
        CGF.EmitPostOMPClause(*(*I), S);

    // CodeGen for clauses (closing steps).
    for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                         E = S.clauses().end();
         I != E; ++I)
      if (*I)
        CGF.EmitCloseOMPClause(*(*I), S);
  }

  CGF.FinishFunction();

  llvm::DenseMap<const ValueDecl *, FieldDecl *> SavedFields =
      CGM.OpenMPSupport.getTaskFields();
  CGM.OpenMPSupport.endOpenMPRegion();

  // CodeGen for 'depend' clause.
  llvm::Value *DependenceAddresses = 0;
  unsigned ArraySize = 0;
  if (!CGM.OpenMPSupport.getUntied()) {
    std::tie(DependenceAddresses, ArraySize) =
        ProcessDependAddresses(*this, S);
  }
  // CodeGen for "omp task {Associated statement}".
  CGM.OpenMPSupport.startOpenMPRegion(false);
  CGM.OpenMPSupport.getTaskFields() = SavedFields;
  {
    RunCleanupsScope MainBlock(*this);

    EmitUntiedPartIdInc(*this);

    llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
    llvm::Value *GTid = CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), *this);
    llvm::Value *RealArgs[] = {
      Loc, GTid, Builder.CreateLoad(Flags, ".flags."),
      Builder.CreateAdd(
          Builder.CreateIntCast(llvm::ConstantExpr::getSizeOf(TaskTTy), SizeTy,
                                false),
          llvm::ConstantInt::get(
              SizeTy,
              getContext().getTypeSizeInChars(PrivateRecord).getQuantity())),
      llvm::ConstantInt::get(
          SizeTy, getContext().getTypeSizeInChars(QTy).getQuantity()),
      Fn
    };
    // kmpc_task_t val = __kmpc_omp_task_alloc(&loc, gtid, flags,
    // sizeof(kmpc_task_t), sizeof(shareds), task_entry);
    llvm::Value *TaskTVal = EmitRuntimeCall(OPENMPRTL_FUNC(omp_task_alloc),
                                            RealArgs, ".task_t.val.addr");
    llvm::Value *SharedAddr = Builder.CreateConstInBoundsGEP2_32(
        TaskTVal, 0, llvm::TaskTBuilder::shareds, ".shared.addr");
    EmitAggregateAssign(Builder.CreateLoad(SharedAddr), Arg, QTy);
    llvm::Value *Locker = Builder.CreateConstGEP1_32(TaskTVal, 1);
    CGM.OpenMPSupport.setPTask(Fn, TaskTVal, LPrivateTy, PrivateRecord, Locker);
    // Skip firstprivate sync for tasks.
    for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                         E = S.clauses().end();
         I != E; ++I)
      if (*I && (isa<OMPPrivateClause>(*I) || isa<OMPFirstPrivateClause>(*I)))
        EmitPreOMPClause(*(*I), S);

    for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                         E = S.clauses().end();
         I != E; ++I)
      if (*I)
        EmitAfterInitOMPClause(*(*I), S);

    if (CGM.OpenMPSupport.getUntied()) {
      llvm::Value *RealArgs1[] = { Loc, GTid, TaskTVal };
      llvm::Value *Res = EmitRuntimeCall(OPENMPRTL_FUNC(omp_task_parts),
                                         RealArgs1, ".task.res.");
      llvm::Value *Cond =
          Builder.CreateICmpEQ(Res, Builder.getInt32(OMP_TASK_CURRENT_QUEUED));
      llvm::BasicBlock *ThenBB = createBasicBlock("task.parts.then");
      llvm::BasicBlock *EndBB = createBasicBlock("task.parts.end");
      Builder.CreateCondBr(Cond, ThenBB, EndBB);
      EmitBlock(ThenBB);
      EmitUntiedBranchEnd(*this);
      EmitBlock(EndBB, true);
    } else {
      llvm::Type *PtrDepTy = getKMPDependInfoType(&CGM)->getPointerTo();
      llvm::Value *RealArgs1[] = {
        Loc, GTid, TaskTVal,
        llvm::ConstantInt::get(Int32Ty, ArraySize),
        DependenceAddresses,
        llvm::ConstantInt::get(Int32Ty, 0),
        llvm::Constant::getNullValue(PtrDepTy)
      };
      EmitRuntimeCall(OPENMPRTL_FUNC(omp_task_with_deps), RealArgs1,
                      ".task.res.");
    }
    EmitUntiedTaskSwitch(*this, true);
  }

  // CodeGen for clauses (task finalize).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I)
      EmitFinalOMPClause(*(*I), S);

  // Remove list of private globals from the stack.
  CGM.OpenMPSupport.endOpenMPRegion();
}

/// Generate an instructions for '#pragma omp sections' directive.
void CodeGenFunction::EmitOMPSectionsDirective(OpenMPDirectiveKind DKind,
                                               OpenMPDirectiveKind SKind,
                                               const OMPExecutableDirective &S) {
  // Init list of private globals in the stack.
  CGM.OpenMPSupport.startOpenMPRegion(false);
  CGM.OpenMPSupport.setNoWait(false);
  CGM.OpenMPSupport.setMergeable(true);
  CGM.OpenMPSupport.setOrdered(false);

  // Generate shared args for captured stmt.
  // CapturedStmt *CS = cast<CapturedStmt>(S.getAssociatedStmt());
  // llvm::Value *Arg = GenerateCapturedStmtArgument(*CS);

  // CodeGen for clauses (task init).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitInitOMPClause(*(*I), S);

  // CodeGen for clauses (task init).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitAfterInitOMPClause(*(*I), S);

  int Schedule = KMP_SCH_DEFAULT;
  bool Ordered = CGM.OpenMPSupport.getOrdered();
  bool Merge = CGM.OpenMPSupport.getMergeable();
  int Offset = 0;
  if (Ordered && Merge)
    Offset = SCH_ORD;
  else if (!Ordered && !Merge)
    Offset = SCH_NM;
  else if (Ordered && !Merge)
    Offset = SCH_NM_ORD;
  Schedule += Offset;
  CGM.OpenMPSupport.setScheduleChunkSize(Schedule, 0);

  // CodeGen for clauses (call start).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitPreOMPClause(*(*I), S);

  // CodeGen for "omp sections {Associated statement}".
  // Calculate number of sections.
  CompoundStmt *AStmt = cast<CompoundStmt>(
      cast<CapturedStmt>(S.getAssociatedStmt())->getCapturedStmt());
  unsigned NumberOfSections = AStmt->size() - 1;

  llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
  llvm::Value *GTid = CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), *this);
  int64_t TypeSize =
      getContext().getTypeSizeInChars(getContext().UnsignedIntTy).getQuantity();
  llvm::IntegerType *UnsignedTy =
      cast<llvm::IntegerType>(ConvertTypeForMem(getContext().UnsignedIntTy));
  llvm::AllocaInst *IterVar =
      CreateMemTemp(getContext().UnsignedIntTy, ".idx.addr");
  InitTempAlloca(IterVar, llvm::Constant::getNullValue(UnsignedTy));
  const Expr *ChunkSize;
  CGM.OpenMPSupport.getScheduleChunkSize(Schedule, ChunkSize);
  llvm::Value *Chunk;
  if (ChunkSize) {
    Chunk = EmitScalarExpr(ChunkSize);
    Chunk = Builder.CreateIntCast(Chunk, TypeSize == 4 ? Builder.getInt32Ty()
                                                       : Builder.getInt64Ty(),
                                  true);
  } else {
    Chunk = (TypeSize == 4) ? Builder.getInt32(0) : Builder.getInt64(0);
  }
  llvm::Value *UBVal = llvm::ConstantInt::get(UnsignedTy, NumberOfSections);
  llvm::IntegerType *SchedTy =
      llvm::TypeBuilder<sched_type, false>::get(getLLVMContext());
  llvm::Value *RealArgs[] = { Loc,
                              GTid,
                              llvm::ConstantInt::get(SchedTy, Schedule),
                              llvm::ConstantInt::get(UnsignedTy, 0),
                              UBVal,
                              llvm::ConstantInt::get(UnsignedTy, 1),
                              Chunk };
  // __kmpc_dispatch_init{4, 8}(&loc, gtid, sched_type, lb, ub, st, chunk);
  if (TypeSize == 4)
    EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_init_4u), RealArgs);
  else
    EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_init_8u), RealArgs);
  llvm::AllocaInst *PLast = CreateTempAlloca(Int32Ty, "last");
  PLast->setAlignment(CGM.getDataLayout().getPrefTypeAlignment(Int32Ty));
  InitTempAlloca(PLast, Builder.getInt32(0));
  llvm::AllocaInst *PLB = CreateMemTemp(getContext().UnsignedIntTy, "lb");
  InitTempAlloca(PLB, llvm::ConstantInt::get(UnsignedTy, 0));
  llvm::AllocaInst *PUB = CreateMemTemp(getContext().UnsignedIntTy, "ub");
  InitTempAlloca(PUB, UBVal);
  llvm::AllocaInst *PSt = CreateMemTemp(getContext().UnsignedIntTy, "st");
  InitTempAlloca(PSt, llvm::ConstantInt::get(UnsignedTy, 1));
  llvm::Value *RealArgsNext[] = { Loc, GTid, PLast, PLB, PUB, PSt };
  llvm::Value *CallRes;
  llvm::BasicBlock *OMPSectionsBB = createBasicBlock("omp.sections.begin");
  EmitBlock(OMPSectionsBB);
  if (TypeSize == 4)
    CallRes = EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_next_4u), RealArgsNext);
  else
    CallRes = EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_next_8u), RealArgsNext);
  llvm::BasicBlock *MainBB = createBasicBlock("omp.sections.main");
  llvm::BasicBlock *EndBB = createBasicBlock("omp.sections.end");
  llvm::SwitchInst *Switch = Builder.CreateSwitch(
      Builder.CreateIntCast(CallRes, Int32Ty, false), EndBB, 1);
  Switch->addCase(llvm::ConstantInt::get(Int32Ty, 1), MainBB);
  EmitBlock(MainBB);

  llvm::Value *UB = Builder.CreateLoad(PUB);
  llvm::Value *LB = Builder.CreateLoad(PLB);
  llvm::BasicBlock *UBLBCheckBB = createBasicBlock("omp.lb_ub.check_pass");
  llvm::Value *UBLBCheck = Builder.CreateICmpULE(LB, UB, "omp.lb.le.ub");
  llvm::BasicBlock *PrevBB = Builder.GetInsertBlock();
  Builder.CreateCondBr(UBLBCheck, UBLBCheckBB, EndBB);
  EmitBlock(UBLBCheckBB);

  llvm::PHINode *Phi = Builder.CreatePHI(UnsignedTy, 2, ".idx.");
  Phi->addIncoming(LB, PrevBB);
  Builder.CreateStore(Phi, IterVar);

  llvm::Value *Idx = Phi; // Builder.CreateLoad(IterVar, ".idx.");
  llvm::BasicBlock *SectionEndBB = createBasicBlock("omp.section.fini");
  llvm::SwitchInst *SectionSwitch =
      Builder.CreateSwitch(Idx, SectionEndBB, NumberOfSections + 1);
  CompoundStmt::const_body_iterator I = AStmt->body_begin();
  for (unsigned i = 0; i <= NumberOfSections; ++i, ++I) {
    RunCleanupsScope ThenScope(*this);
    llvm::BasicBlock *SectionBB = createBasicBlock("omp.section");
    SectionSwitch->addCase(llvm::ConstantInt::get(UnsignedTy, i), SectionBB);
    EmitBlock(SectionBB);
    EmitStmt(*I);
    EnsureInsertPoint();
    EmitBranch(SectionEndBB);
  }
  EmitBlock(SectionEndBB, true);

  llvm::Value *NextIdx =
      Builder.CreateAdd(Builder.CreateLoad(IterVar, ".idx."),
                        llvm::ConstantInt::get(UnsignedTy, 1), ".next.idx.");
  llvm::Value *RealArgsFini[] = { Loc, GTid };
  if (TypeSize == 4)
    EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_fini_4u), RealArgsFini);
  else
    EmitRuntimeCall(OPENMPRTL_FUNC(dispatch_fini_8u), RealArgsFini);
  UBLBCheck = Builder.CreateICmpULE(NextIdx, UB, "omp.idx.le.ub");
  PrevBB = Builder.GetInsertBlock();
  Builder.CreateCondBr(UBLBCheck, UBLBCheckBB, OMPSectionsBB);
  Phi->addIncoming(NextIdx, PrevBB);

  EmitBlock(EndBB, true);
  CGM.OpenMPSupport.setLastIterVar(PLast);

  if (CGM.OpenMPSupport.hasLastPrivate() || !CGM.OpenMPSupport.getNoWait())
    EmitOMPCallWithLocAndTidHelper(OPENMPRTL_FUNC(cancel_barrier), S.getLocEnd(),
                                   KMP_IDENT_BARRIER_IMPL_SECTIONS);

  // CodeGen for clauses (call end).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitPostOMPClause(*(*I), S);

  // CodeGen for clauses (closing steps).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitCloseOMPClause(*(*I), S);

  // CodeGen for clauses (task finalize).
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I)
    if (*I && isAllowedClauseForDirective(SKind, (*I)->getClauseKind()))
      EmitFinalOMPClause(*(*I), S);

  EnsureInsertPoint();

  // Remove list of private globals from the stack.
  CGM.OpenMPSupport.endOpenMPRegion();
}

/// Generate an instructions for '#pragma omp sections' directive.
void CodeGenFunction::EmitOMPSectionsDirective(const OMPSectionsDirective &S) {
  EmitOMPSectionsDirective(OMPD_sections, OMPD_sections, S);
}

/// Generate an instructions for '#pragma omp section' directive.
void CodeGenFunction::EmitOMPSectionDirective(const OMPSectionDirective &S) {
  EmitStmt(cast<CapturedStmt>(S.getAssociatedStmt())->getCapturedStmt());
}

void CodeGenFunction::EmitInitOMPClause(const OMPClause &C,
                                        const OMPExecutableDirective &S) {
  switch (C.getClauseKind()) {
  default:
    llvm_unreachable("Unknown clause kind!");
  case OMPC_num_threads:
    EmitInitOMPNumThreadsClause(cast<OMPNumThreadsClause>(C), S);
    break;
  case OMPC_proc_bind:
    EmitInitOMPProcBindClause(cast<OMPProcBindClause>(C), S);
    break;
  case OMPC_reduction:
    EmitInitOMPReductionClause(cast<OMPReductionClause>(C), S);
    break;
  case OMPC_nowait:
    EmitInitOMPNowaitClause(cast<OMPNowaitClause>(C), S);
    break;
  case OMPC_ordered:
    EmitInitOMPOrderedClause(cast<OMPOrderedClause>(C), S);
    break;
  case OMPC_untied:
    EmitInitOMPUntiedClause(cast<OMPUntiedClause>(C), S);
    break;
  case OMPC_final:
    EmitInitOMPFinalClause(cast<OMPFinalClause>(C), S);
    break;
  case OMPC_mergeable:
    EmitInitOMPMergeableClause(cast<OMPMergeableClause>(C), S);
    break;
  case OMPC_default:
  case OMPC_schedule:
  case OMPC_copyin:
  case OMPC_copyprivate:
  case OMPC_shared:
  case OMPC_private:
  case OMPC_firstprivate:
  case OMPC_lastprivate:
  case OMPC_collapse:
  case OMPC_if:
  case OMPC_read:
  case OMPC_write:
  case OMPC_capture:
  case OMPC_update:
  case OMPC_seq_cst:
  case OMPC_linear:
  case OMPC_aligned:
  case OMPC_safelen:
  case OMPC_depend:
    break;
  }
}

void CodeGenFunction::EmitAfterInitOMPClause(const OMPClause &C,
                                             const OMPExecutableDirective &S) {
  switch (C.getClauseKind()) {
  default:
    llvm_unreachable("Unknown clause kind!");
  case OMPC_if:
    EmitAfterInitOMPIfClause(cast<OMPIfClause>(C), S);
    break;
  case OMPC_reduction:
  case OMPC_nowait:
  case OMPC_ordered:
  case OMPC_untied:
  case OMPC_final:
  case OMPC_mergeable:
  case OMPC_default:
  case OMPC_proc_bind:
  case OMPC_num_threads:
  case OMPC_schedule:
  case OMPC_copyin:
  case OMPC_shared:
  case OMPC_private:
  case OMPC_firstprivate:
  case OMPC_lastprivate:
  case OMPC_collapse:
  case OMPC_read:
  case OMPC_write:
  case OMPC_capture:
  case OMPC_update:
  case OMPC_seq_cst:
  case OMPC_linear:
  case OMPC_aligned:
  case OMPC_safelen:
  case OMPC_depend:
    break;
  }
}

void CodeGenFunction::EmitPreOMPClause(const OMPClause &C,
                                       const OMPExecutableDirective &S) {
  switch (C.getClauseKind()) {
  default:
    llvm_unreachable("Unknown clause kind!");
  case OMPC_num_threads:
  case OMPC_if:
  case OMPC_default:
  case OMPC_proc_bind:
  case OMPC_shared:
  case OMPC_collapse:
  case OMPC_nowait:
  case OMPC_ordered:
  case OMPC_copyprivate:
  case OMPC_untied:
  case OMPC_final:
  case OMPC_mergeable:
  case OMPC_read:
  case OMPC_write:
  case OMPC_capture:
  case OMPC_update:
  case OMPC_seq_cst:
  case OMPC_linear:
  case OMPC_aligned:
  case OMPC_safelen:
  case OMPC_depend:
    break;
  case OMPC_copyin:
    EmitPreOMPCopyinClause(cast<OMPCopyinClause>(C), S);
    break;
  case OMPC_private:
    EmitPreOMPPrivateClause(cast<OMPPrivateClause>(C), S);
    break;
  case OMPC_firstprivate:
    EmitPreOMPFirstPrivateClause(cast<OMPFirstPrivateClause>(C), S);
    break;
  case OMPC_lastprivate:
    EmitPreOMPLastPrivateClause(cast<OMPLastPrivateClause>(C), S);
    break;
  case OMPC_reduction:
    EmitPreOMPReductionClause(cast<OMPReductionClause>(C), S);
    break;
  case OMPC_schedule:
    EmitPreOMPScheduleClause(cast<OMPScheduleClause>(C), S);
    break;
  }
}

void CodeGenFunction::EmitPostOMPClause(const OMPClause &C,
                                        const OMPExecutableDirective &S) {
  switch (C.getClauseKind()) {
  default:
    llvm_unreachable("Unknown clause kind!");
  case OMPC_num_threads:
  case OMPC_if:
  case OMPC_copyin:
  case OMPC_copyprivate:
  case OMPC_default:
  case OMPC_proc_bind:
  case OMPC_shared:
  case OMPC_collapse:
  case OMPC_nowait:
  case OMPC_ordered:
  case OMPC_schedule:
  case OMPC_untied:
  case OMPC_final:
  case OMPC_mergeable:
  case OMPC_read:
  case OMPC_write:
  case OMPC_capture:
  case OMPC_update:
  case OMPC_seq_cst:
  case OMPC_linear:
  case OMPC_aligned:
  case OMPC_safelen:
  case OMPC_depend:
    break;
  case OMPC_private:
    EmitPostOMPPrivateClause(cast<OMPPrivateClause>(C), S);
    break;
  case OMPC_firstprivate:
    EmitPostOMPFirstPrivateClause(cast<OMPFirstPrivateClause>(C), S);
    break;
  case OMPC_lastprivate:
    EmitPostOMPLastPrivateClause(cast<OMPLastPrivateClause>(C), S);
    break;
  case OMPC_reduction:
    EmitPostOMPReductionClause(cast<OMPReductionClause>(C), S);
    break;
  }
}

void CodeGenFunction::EmitCloseOMPClause(const OMPClause &C,
                                         const OMPExecutableDirective &S) {
  switch (C.getClauseKind()) {
  default:
    llvm_unreachable("Unknown clause kind!");
  case OMPC_num_threads:
  case OMPC_if:
  case OMPC_copyin:
  case OMPC_copyprivate:
  case OMPC_default:
  case OMPC_proc_bind:
  case OMPC_shared:
  case OMPC_private:
  case OMPC_firstprivate:
  case OMPC_collapse:
  case OMPC_nowait:
  case OMPC_ordered:
  case OMPC_schedule:
  case OMPC_untied:
  case OMPC_final:
  case OMPC_mergeable:
  case OMPC_read:
  case OMPC_write:
  case OMPC_capture:
  case OMPC_update:
  case OMPC_seq_cst:
  case OMPC_linear:
  case OMPC_aligned:
  case OMPC_safelen:
  case OMPC_depend:
    break;
  case OMPC_lastprivate:
    EmitCloseOMPLastPrivateClause(cast<OMPLastPrivateClause>(C), S);
    break;
  case OMPC_reduction:
    EmitCloseOMPReductionClause(cast<OMPReductionClause>(C), S);
    break;
  }
}

void CodeGenFunction::EmitFinalOMPClause(const OMPClause &C,
                                         const OMPExecutableDirective &S) {
  switch (C.getClauseKind()) {
  default:
    llvm_unreachable("Unknown clause kind!");
  case OMPC_num_threads:
  case OMPC_copyin:
  case OMPC_copyprivate:
  case OMPC_default:
  case OMPC_proc_bind:
  case OMPC_shared:
  case OMPC_private:
  case OMPC_firstprivate:
  case OMPC_lastprivate:
  case OMPC_collapse:
  case OMPC_nowait:
  case OMPC_ordered:
  case OMPC_schedule:
  case OMPC_untied:
  case OMPC_final:
  case OMPC_mergeable:
  case OMPC_read:
  case OMPC_write:
  case OMPC_capture:
  case OMPC_update:
  case OMPC_seq_cst:
  case OMPC_linear:
  case OMPC_aligned:
  case OMPC_safelen:
  case OMPC_depend:
    break;
  case OMPC_if:
    EmitFinalOMPIfClause(cast<OMPIfClause>(C), S);
    break;
  case OMPC_reduction:
    EmitFinalOMPReductionClause(cast<OMPReductionClause>(C), S);
    break;
  }
}

void CodeGenFunction::EmitInitOMPNowaitClause(const OMPNowaitClause &C,
                                              const OMPExecutableDirective &S) {
  CGM.OpenMPSupport.setNoWait(true);
}

void
CodeGenFunction::EmitInitOMPOrderedClause(const OMPOrderedClause &C,
                                          const OMPExecutableDirective &S) {
  CGM.OpenMPSupport.setOrdered(true);
}

void CodeGenFunction::EmitInitOMPUntiedClause(const OMPUntiedClause &C,
                                              const OMPExecutableDirective &S) {
  CGM.OpenMPSupport.setUntied(true);
}

void
CodeGenFunction::EmitInitOMPMergeableClause(const OMPMergeableClause &C,
                                            const OMPExecutableDirective &S) {
  CGM.OpenMPSupport.setMergeable(true);
}

void CodeGenFunction::EmitInitOMPFinalClause(const OMPFinalClause &C,
                                             const OMPExecutableDirective &S) {
  llvm::Value *Flags = CGM.OpenMPSupport.getTaskFlags();
  llvm::BasicBlock *ThenBlock = createBasicBlock("task.final.then");
  llvm::BasicBlock *EndBlock = createBasicBlock("task.final.end");
  RegionCounter Cnt = getPGORegionCounter(&S);
  EmitBranchOnBoolExpr(C.getCondition(), ThenBlock, EndBlock, Cnt.getCount());
  EmitBlock(ThenBlock);
  llvm::Value *Val =
      Builder.CreateOr(Builder.CreateLoad(Flags, ".flags."), OMP_TASK_FINAL);
  Builder.CreateStore(Val, Flags);
  EmitBranch(EndBlock);
  EmitBlock(EndBlock, true);
}

void
CodeGenFunction::EmitInitOMPNumThreadsClause(const OMPNumThreadsClause &C,
                                             const OMPExecutableDirective &S) {
  // __kmpc_push_num_threads(&loc, global_tid, num_threads);
  // ident_t loc = {...};
  llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(C.getLocStart(), *this);
  // global_tid = __kmpc_global_thread_num(...);
  llvm::Value *GTid = CGM.CreateOpenMPGlobalThreadNum(C.getLocStart(), *this);
  // num_threads = num_threads...;
  llvm::Value *NumThreads = EmitScalarExpr(C.getNumThreads(), true);
  llvm::Value *RealArgs[] = { Loc, GTid, NumThreads };
  EmitRuntimeCall(OPENMPRTL_FUNC(push_num_threads), RealArgs);
}

void
CodeGenFunction::EmitInitOMPProcBindClause(const OMPProcBindClause &C,
                                           const OMPExecutableDirective &S) {
  // __kmpc_push_proc_bind(&loc, global_tid, proc_bind);
  // ident_t loc = {...};
  llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(C.getLocStart(), *this);
  // global_tid = __kmpc_global_thread_num(...);
  llvm::Value *GTid = CGM.CreateOpenMPGlobalThreadNum(C.getLocStart(), *this);
  // proc_bind = proc_bind...;
  llvm::Value *ProcBind = 0;
  switch (C.getThreadAffinity()) {
  case OMPC_PROC_BIND_master:
    ProcBind = llvm::ConstantInt::get(
        llvm::ProcBindTBuilder::get(CGM.getLLVMContext()),
        KMP_PROC_BIND_MASTER);
    break;
  case OMPC_PROC_BIND_close:
    ProcBind = llvm::ConstantInt::get(
        llvm::ProcBindTBuilder::get(CGM.getLLVMContext()), KMP_PROC_BIND_CLOSE);
    break;
  case OMPC_PROC_BIND_spread:
    ProcBind = llvm::ConstantInt::get(
        llvm::ProcBindTBuilder::get(CGM.getLLVMContext()),
        KMP_PROC_BIND_SPREAD);
    break;
  case OMPC_PROC_BIND_unknown:
  case NUM_OPENMP_PROC_BIND_KINDS:
    llvm_unreachable("Unknown thread affinity");
  }
  llvm::Value *RealArgs[] = { Loc, GTid, ProcBind };
  EmitRuntimeCall(OPENMPRTL_FUNC(push_proc_bind), RealArgs);
}

void CodeGenFunction::EmitAfterInitOMPIfClause(const OMPIfClause &C,
                                               const OMPExecutableDirective &S) {
  RegionCounter Cnt = getPGORegionCounter(&S);
  if (isa<OMPTaskDirective>(&S)) {
    llvm::BasicBlock *ThenBlock = createBasicBlock("omp.if.then");
    llvm::BasicBlock *ElseBlock = createBasicBlock("omp.if.else");
    EmitBranchOnBoolExpr(C.getCondition(), ThenBlock, ElseBlock, Cnt.getCount());
    EmitBlock(ThenBlock);
    CGM.OpenMPSupport.setIfDest(ElseBlock);
  } else {
    // if (Cond) {
    llvm::BasicBlock *ThenBlock = createBasicBlock("omp.if.then");
    llvm::BasicBlock *ElseBlock = createBasicBlock("omp.if.else");
    llvm::BasicBlock *ContBlock = createBasicBlock("omp.if.end");
    EmitBranchOnBoolExpr(C.getCondition(), ThenBlock, ElseBlock, Cnt.getCount());
    EmitBlock(ElseBlock);
    {
      RunCleanupsScope ElseScope(*this);
      EmitStmt(cast<CapturedStmt>(S.getAssociatedStmt())->getCapturedStmt());
      EnsureInsertPoint();
    }
    EmitBranch(ContBlock);
    EmitBlock(ThenBlock);
    CGM.OpenMPSupport.setIfDest(ContBlock);
  }
}

void CodeGenFunction::EmitFinalOMPIfClause(const OMPIfClause &C,
                                           const OMPExecutableDirective &S) {
  if (isa<OMPTaskDirective>(&S)) {
    llvm::BasicBlock *ContBlock = createBasicBlock("omp.if.end");
    EmitBranch(ContBlock);
    EmitBlock(CGM.OpenMPSupport.takeIfDest());
    {
      llvm::Value *PTask;
      llvm::Value *TaskTVal;
      llvm::Type *PrivateTy;
      QualType PrivateQTy;
      llvm::Value *Base;
      CGM.OpenMPSupport.getPTask(PTask, TaskTVal, PrivateTy, PrivateQTy, Base);
      llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
      llvm::Value *GTid =
          CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), *this);
      llvm::Value *RealArgs[] = { Loc, GTid, TaskTVal };
      EmitRuntimeCall(OPENMPRTL_FUNC(omp_task_begin_if0),
                      makeArrayRef(RealArgs));
      llvm::Value *RealArgs1[] = { GTid, Builder.CreatePointerCast(TaskTVal,
                                                                   VoidPtrTy) };
      EmitCallOrInvoke(PTask, makeArrayRef(RealArgs1));
      EmitRuntimeCall(OPENMPRTL_FUNC(omp_task_complete_if0),
                      makeArrayRef(RealArgs));
    }
    EmitBranch(ContBlock);
    EmitBlock(ContBlock, true);
  } else {
    llvm::BasicBlock *ContBlock = CGM.OpenMPSupport.takeIfDest();
    EmitBranch(ContBlock);
    EmitBlock(ContBlock, true);
  }
}

void
CodeGenFunction::EmitPreOMPScheduleClause(const OMPScheduleClause &C,
                                          const OMPExecutableDirective &S) {
  int Schedule = KMP_SCH_DEFAULT;
  bool Ordered = CGM.OpenMPSupport.getOrdered();
  bool Merge = CGM.OpenMPSupport.getMergeable();
  int Offset = 0;
  if (Ordered && Merge)
    Offset = SCH_ORD;
  else if (!Ordered && !Merge)
    Offset = SCH_NM;
  else if (Ordered && !Merge)
    Offset = SCH_NM_ORD;
  const Expr *ChunkSize = C.getChunkSize();

  switch (C.getScheduleKind()) {
  case OMPC_SCHEDULE_static:
    Schedule = ChunkSize ? KMP_SCH_STATIC_CHUNKED : KMP_SCH_STATIC;
    break;
  case OMPC_SCHEDULE_dynamic:
    Schedule = KMP_SCH_DYNAMIC_CHUNKED;
    break;
  case OMPC_SCHEDULE_guided:
    Schedule = KMP_SCH_GUIDED_CHUNKED;
    break;
  case OMPC_SCHEDULE_auto:
    Schedule = KMP_SCH_AUTO;
    break;
  case OMPC_SCHEDULE_runtime:
    Schedule = KMP_SCH_RUNTIME;
    break;
  case OMPC_SCHEDULE_unknown:
  case NUM_OPENMP_SCHEDULE_KINDS:
    llvm_unreachable("Unknown schedule kind.");
  }
  Schedule += Offset;
  CGM.OpenMPSupport.setScheduleChunkSize(Schedule, ChunkSize);
}

void CodeGenFunction::EmitUniversalStore(LValue Dst, llvm::Value *Src,
                                         QualType ExprTy) {
  switch (getEvaluationKind(ExprTy)) {
  case TEK_Complex: {
    RValue Val = convertTempToRValue(Src, ExprTy, SourceLocation());
    EmitStoreOfComplex(Val.getComplexVal(), Dst, false);
    break;
  }
  case TEK_Aggregate:
    EmitAggregateAssign(Dst.getAddress(), Src, ExprTy);
    break;
  case TEK_Scalar:
    RValue Val = convertTempToRValue(Src, ExprTy, SourceLocation());
    EmitStoreThroughLValue(Val, Dst, false);
    break;
  }
}

void CodeGenFunction::EmitUniversalStore(llvm::Value *Dst, llvm::Value *Src,
                                         QualType ExprTy) {
  EmitUniversalStore(MakeNaturalAlignAddrLValue(Dst, ExprTy), Src, ExprTy);
}

// This helper is used for emitting copy-assignments for copyin clause and
// for copy_function generated for copyprivate clause.
void
CodeGenFunction::EmitCopyAssignment(ArrayRef<const Expr *>::iterator I,
                                    ArrayRef<const Expr *>::iterator AssignIter,
                                    ArrayRef<const Expr *>::iterator VarIter1,
                                    ArrayRef<const Expr *>::iterator VarIter2,
                                    llvm::Value *Dst, llvm::Value *Src) {
  // This is called at each iteration of the loop through the clauses.
  {
    // Get element type.
    QualType QTy = (*I)->getType();
    const Type *MainTy = QTy.getTypePtr();
    // const Type *Ty = MainTy->getArrayElementTypeNoTypeQual();
    // const Type *PrevTy = MainTy;
    // while (Ty != 0) {
    //  PrevTy = Ty;
    //  Ty = Ty->getArrayElementTypeNoTypeQual();
    //}
    // Ty = PrevTy;

    if (!*AssignIter) {
      // For trivial assignment operator copy by memcpy.
      llvm::Value *VDAddr = Src;
      EmitUniversalStore(Builder.CreatePointerCast(Dst, VDAddr->getType()),
                         VDAddr, QTy);
    } else {
      RunCleanupsScope InitBlock(*this);
      // Copy elements one by one.
      if (const ArrayType *ArrayTy = MainTy->getAsArrayTypeUnsafe()) {
        // Copy array.
        QualType ElementTy;
        llvm::Value *SharedVar = Dst;
        llvm::Value *NumElements =
            emitArrayLength(ArrayTy, ElementTy, SharedVar);
        llvm::Value *ArrayEnd =
            Builder.CreateGEP(SharedVar, NumElements);
        llvm::Value *MasterArray = Src;
        unsigned AddrSpace = MasterArray->getType()->getPointerAddressSpace();
        llvm::Type *BaseType = ConvertType(ElementTy)->getPointerTo(AddrSpace);
        llvm::Value *MasterArrayBegin = Builder.CreatePointerCast(
            MasterArray, BaseType, "master.array.begin");
        llvm::Value *MasterArrayEnd =
            Builder.CreateGEP(MasterArrayBegin, NumElements);
        // The basic structure here is a do-while loop, because we don't
        // need to check for the zero-element case.
        llvm::BasicBlock *BodyBB = createBasicBlock("omp.arraycpy.body");
        llvm::BasicBlock *DoneBB = createBasicBlock("omp.arraycpy.done");
        llvm::Value *IsEmpty =
            Builder.CreateICmpEQ(SharedVar, ArrayEnd, "omp.arraycpy.isempty");
        Builder.CreateCondBr(IsEmpty, DoneBB, BodyBB);

        // Enter the loop body, making that address the current address.
        llvm::BasicBlock *EntryBB = Builder.GetInsertBlock();
        EmitBlock(BodyBB);
        llvm::PHINode *ElementPast = Builder.CreatePHI(
            SharedVar->getType(), 2, "omp.arraycpy.elementPast");
        ElementPast->addIncoming(ArrayEnd, EntryBB);
        llvm::PHINode *MasterElementPast = Builder.CreatePHI(
            MasterArrayBegin->getType(), 2, "omp.arraycpy.masterElementPast");
        MasterElementPast->addIncoming(MasterArrayEnd, EntryBB);

        // Shift the address back by one element.
        llvm::Value *NegativeOne = llvm::ConstantInt::get(SizeTy, -1, true);
        llvm::Value *Element = Builder.CreateGEP(
            ElementPast, NegativeOne, "omp.arraycpy.element");
        llvm::Value *MasterElement = Builder.CreateGEP(
            MasterElementPast, NegativeOne, "omp.arraycpy.master.element");

        const VarDecl *PseudoVar1 =
            cast<VarDecl>(cast<DeclRefExpr>(*VarIter1)->getDecl());
        const VarDecl *PseudoVar2 =
            cast<VarDecl>(cast<DeclRefExpr>(*VarIter2)->getDecl());
        CGM.OpenMPSupport.addOpenMPPrivateVar(PseudoVar1, Element);
        CGM.OpenMPSupport.addOpenMPPrivateVar(PseudoVar2, MasterElement);
        EmitIgnoredExpr(*AssignIter);
        CGM.OpenMPSupport.delOpenMPPrivateVar(PseudoVar1);
        CGM.OpenMPSupport.delOpenMPPrivateVar(PseudoVar2);

        // Check whether we've reached the end.
        llvm::Value *Done =
            Builder.CreateICmpEQ(Element, SharedVar, "omp.arraycpy.done");
        Builder.CreateCondBr(Done, DoneBB, BodyBB);
        ElementPast->addIncoming(Element, Builder.GetInsertBlock());
        MasterElementPast->addIncoming(MasterElement, Builder.GetInsertBlock());

        // Done.
        EmitBlock(DoneBB, true);
      } else {
        // Copy single object.
        const VarDecl *PseudoVar1 =
            cast<VarDecl>(cast<DeclRefExpr>(*VarIter1)->getDecl());
        const VarDecl *PseudoVar2 =
            cast<VarDecl>(cast<DeclRefExpr>(*VarIter2)->getDecl());
        CGM.OpenMPSupport.addOpenMPPrivateVar(PseudoVar1, Dst);
        CGM.OpenMPSupport.addOpenMPPrivateVar(PseudoVar2, Src);
        EmitIgnoredExpr(*AssignIter);
        CGM.OpenMPSupport.delOpenMPPrivateVar(PseudoVar1);
        CGM.OpenMPSupport.delOpenMPPrivateVar(PseudoVar2);
      }
    }
  }
}

void CodeGenFunction::EmitPreOMPCopyinClause(const OMPCopyinClause &C,
                                             const OMPExecutableDirective &S) {
  // copy_data(var1);
  // copy_data(var2);
  // ...
  // __kmpc_barrier(&loc, global_tid);
  ArrayRef<const Expr *>::iterator AssignIter = C.getAssignments().begin();
  ArrayRef<const Expr *>::iterator VarIter1 = C.getPseudoVars1().begin();
  ArrayRef<const Expr *>::iterator VarIter2 = C.getPseudoVars2().begin();

  for (OMPCopyinClause::varlist_const_iterator I = C.varlist_begin(),
                                               E = C.varlist_end();
       I != E; ++I, ++AssignIter, ++VarIter1, ++VarIter2) {
    const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(*I)->getDecl());
    EmitCopyAssignment(I, AssignIter, VarIter1, VarIter2,
                       CGM.CreateOpenMPThreadPrivateCached(
                           VD, (*I)->getExprLoc(), *this, true),
                       VD->isStaticLocal() ? CGM.getStaticLocalDeclAddress(VD)
                                           : CGM.GetAddrOfGlobal(VD));
  }
  SetFirstprivateInsertPt(*this);
}

/// \brief Determine whether the given initializer is trivial in the sense
/// that it requires no code to be generated.
static bool isTrivialInitializer(const Expr *Init) {
  if (!Init)
    return true;

  if (const CXXConstructExpr *Construct = dyn_cast<CXXConstructExpr>(Init))
    if (CXXConstructorDecl *Constructor = Construct->getConstructor())
      if (Constructor->isTrivial() && Constructor->isDefaultConstructor() &&
          !Construct->requiresZeroInitialization())
        return true;

  return false;
}

void CodeGenFunction::EmitPreOMPPrivateClause(const OMPPrivateClause &C,
                                              const OMPExecutableDirective &S) {
  // Type1 tmp1;
  // anon.field1 = &tmp1;
  // Type2 tmp2;
  // anon.field2 = &tmp2;
  // ...
  //
  ArrayRef<const Expr *>::iterator InitIter = C.getDefaultInits().begin();
  for (OMPPrivateClause::varlist_const_iterator I = C.varlist_begin(),
                                                E = C.varlist_end();
       I != E; ++I, ++InitIter) {
    // Get element type.
    const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(*I)->getDecl());
    if (CGM.OpenMPSupport.getTopOpenMPPrivateVar(VD))
      continue;
    // if (VD->hasLocalStorage() &&
    //    (!CapturedStmtInfo ||
    //     !CapturedStmtInfo->lookup(VD))) {
    //  LocalDeclMap[VD] = CreateMemTemp(VD->getType(), CGM.getMangledName(VD));
    //}
    QualType QTy = (*I)->getType();
    const Type *MainTy = QTy.getTypePtr();
    // const Type *Ty = MainTy->getArrayElementTypeNoTypeQual();
    // const Type *PrevTy = MainTy;
    // while (Ty != 0) {
    //  PrevTy = Ty;
    //  Ty = Ty->getArrayElementTypeNoTypeQual();
    //}
    // Ty = PrevTy;
    llvm::Value *Private;
    llvm::Value *PTask;
    llvm::Value *TaskTVal;
    llvm::Type *PrivateTy;
    QualType PrivateQTy;
    llvm::Value *Base;
    CGM.OpenMPSupport.getPTask(PTask, TaskTVal, PrivateTy, PrivateQTy, Base);
    if (PTask) {
      Base = Builder.CreatePointerCast(Base, PrivateTy->getPointerTo());
      Private = EmitLValueForField(MakeNaturalAlignAddrLValue(Base, PrivateQTy),
                                   CGM.OpenMPSupport.getTaskFields()[VD])
                    .getAddress();
    } else {
      LocalVarsDeclGuard Grd(*this, true);
      AutoVarEmission Emission = EmitAutoVarAlloca(*VD);
      Private = Emission.getAllocatedAddress();
    }
    // CodeGen for classes with the default constructor.
    if (((!PTask || CurFn != PTask) && !isTrivialInitializer(*InitIter)) ||
        (MainTy->isVariablyModifiedType() && !MainTy->isPointerType())) {
      RunCleanupsScope InitBlock(*this);
      if (const ArrayType *ArrayTy = MainTy->getAsArrayTypeUnsafe()) {
        // Create array.
        QualType ElementTy;
        llvm::Value *ArrayBeg = Private;
        llvm::Value *NumElements =
            emitArrayLength(ArrayTy, ElementTy, ArrayBeg);
        llvm::Value *ArrayEnd = Builder.CreateGEP(ArrayBeg, NumElements,
                                                          "omp.arrayctor.end");
        // The basic structure here is a do-while loop, because we don't
        // need to check for the zero-element case.
        llvm::BasicBlock *BodyBB = createBasicBlock("omp.arrayctor.body");
        llvm::BasicBlock *DoneBB = createBasicBlock("omp.arrayctor.done");
        llvm::Value *IsEmpty =
            Builder.CreateICmpEQ(ArrayBeg, ArrayEnd, "omp.arrayctor.isempty");
        Builder.CreateCondBr(IsEmpty, DoneBB, BodyBB);

        // Enter the loop body, making that address the current address.
        llvm::BasicBlock *EntryBB = Builder.GetInsertBlock();
        EmitBlock(BodyBB);
        llvm::PHINode *ElementPast = Builder.CreatePHI(
            ArrayBeg->getType(), 2, "omp.arrayctor.elementPast");
        ElementPast->addIncoming(ArrayEnd, EntryBB);

        // Shift the address back by one element.
        llvm::Value *NegativeOne = llvm::ConstantInt::get(SizeTy, -1, true);
        llvm::Value *Element = Builder.CreateGEP(
            ElementPast, NegativeOne, "omp.arrayctor.element");
        EmitAnyExprToMem(*InitIter, Element,
                         (*InitIter)->getType().getQualifiers(), false);
        //// Check whether we've reached the end.
        llvm::Value *Done =
            Builder.CreateICmpEQ(Element, ArrayBeg, "omp.arrayctor.done");
        Builder.CreateCondBr(Done, DoneBB, BodyBB);
        ElementPast->addIncoming(Element, Builder.GetInsertBlock());

        // Done.
        EmitBlock(DoneBB, true);
      } else {
        EmitAnyExprToMem(*InitIter, Private,
                         (*InitIter)->getType().getQualifiers(), false);
      }
    }
    CGM.OpenMPSupport.addOpenMPPrivateVar(VD, Private);
  }
}

void
CodeGenFunction::EmitPostOMPPrivateClause(const OMPPrivateClause &C,
                                          const OMPExecutableDirective &S) {
  // ~Type1(tmp1);
  // ~Type2(tmp2);
  // ...
  //
  for (OMPPrivateClause::varlist_const_iterator I = C.varlist_begin(),
                                                E = C.varlist_end();
       I != E; ++I) {
    // Get element type.
    const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(*I)->getDecl());
    QualType QTy = (*I)->getType();
    const Type *MainTy = QTy.getTypePtr();
    const Type *Ty = MainTy->getArrayElementTypeNoTypeQual();
    const Type *PrevTy = MainTy;
    while (Ty != 0) {
      PrevTy = Ty;
      Ty = Ty->getArrayElementTypeNoTypeQual();
    }
    Ty = PrevTy;
    llvm::Value *Private = CGM.OpenMPSupport.getTopOpenMPPrivateVar(VD);
    if (!Private)
      continue;
    CGM.OpenMPSupport.delOpenMPPrivateVar(VD);
    // CodeGen for classes with the default constructor.
    if (CXXRecordDecl *RD = Ty->getAsCXXRecordDecl()) {
      // Find destructor.
      CXXDestructorDecl *Dtor = RD->getDestructor();
      if (Dtor && !Dtor->isTrivial()) {
        if (MainTy->getAsArrayTypeUnsafe()) {
          // Destroy array.
          emitDestroy(Private, QTy, getDestroyer(QualType::DK_cxx_destructor),
                      false);
        } else {
          // Destroy single object.
          EmitCXXDestructorCall(Dtor, Dtor_Complete, false, false, Private);
        }
      }
    }
  }
}

void
CodeGenFunction::EmitPreOMPFirstPrivateClause(const OMPFirstPrivateClause &C,
                                              const OMPExecutableDirective &S) {
  // Type1 tmp1(var1);
  // anon.field1 = &tmp1;
  // Type2 tmp2(var2);
  // anon.field2 = &tmp2;
  // ...
  //
  llvm::Value *PTask;
  llvm::Value *TaskTVal;
  llvm::Type *PrivateTy;
  QualType PrivateQTy;
  llvm::Value *Base;
  CGM.OpenMPSupport.getPTask(PTask, TaskTVal, PrivateTy, PrivateQTy, Base);

  ArrayRef<const Expr *>::iterator InitIter = C.getInits().begin();
  ArrayRef<const Expr *>::iterator VarIter = C.getPseudoVars().begin();
  for (OMPFirstPrivateClause::varlist_const_iterator I = C.varlist_begin(),
                                                     E = C.varlist_end();
       I != E; ++I, ++InitIter, ++VarIter) {
    // Get element type.
    const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(*I)->getDecl());
    if (CGM.OpenMPSupport.getTopOpenMPPrivateVar(VD))
      continue;
    // if (VD->hasLocalStorage() &&
    //    (!CapturedStmtInfo ||
    //     !CapturedStmtInfo->lookup(VD))) {
    //  LocalDeclMap[VD] = CreateMemTemp(VD->getType(), CGM.getMangledName(VD));
    //}
    QualType QTy = (*I)->getType();
    const Type *MainTy = QTy.getTypePtr();
    // const Type *Ty = MainTy->getArrayElementTypeNoTypeQual();
    // const Type *PrevTy = MainTy;
    // while (Ty != 0) {
    //  PrevTy = Ty;
    //  Ty = Ty->getArrayElementTypeNoTypeQual();
    //}
    llvm::Value *Private = 0;
    if (!CGM.OpenMPSupport.isNewTask() && !PTask) {
      if (llvm::AllocaInst *Val = dyn_cast_or_null<llvm::AllocaInst>(
              CGM.OpenMPSupport.getPrevOpenMPPrivateVar(VD))) {
        Private = Val;
        CGM.OpenMPSupport.delPrevOpenMPPrivateVar(VD);
        CGM.OpenMPSupport.addOpenMPPrivateVar(VD, Private);
        continue;
      }
    }
    if (PTask) {
      Base = Builder.CreatePointerCast(Base, PrivateTy->getPointerTo());
      Private = EmitLValueForField(MakeNaturalAlignAddrLValue(Base, PrivateQTy),
                                   CGM.OpenMPSupport.getTaskFields()[VD])
                    .getAddress();
    } else {
      LocalVarsDeclGuard Grd(*this, true);
      AutoVarEmission Emission = EmitAutoVarAlloca(*VD);
      Private = Emission.getAllocatedAddress();
    }
    // CodeGen for classes with the copy constructor.
    RunCleanupsScope InitBlock(*this);
    if (((!PTask || CurFn != PTask) && !isTrivialInitializer(*InitIter)) ||
        (MainTy->isVariablyModifiedType() && !MainTy->isPointerType())) {
      if (const ArrayType *ArrayTy = MainTy->getAsArrayTypeUnsafe()) {
        // Create array.
        QualType ElementTy;
        llvm::Value *ArrayBeg = Private;
        llvm::Value *NumElements =
            emitArrayLength(ArrayTy, ElementTy, ArrayBeg);
        llvm::Value *ArrayEnd =
            Builder.CreateGEP(ArrayBeg, NumElements);
        llvm::Value *MasterArray = EmitLValue(*I).getAddress();
        unsigned AddrSpace = MasterArray->getType()->getPointerAddressSpace();
        llvm::Type *BaseType = ConvertType(ElementTy)->getPointerTo(AddrSpace);
        llvm::Value *MasterArrayBegin = Builder.CreatePointerCast(
            MasterArray, BaseType, "master.array.begin");
        llvm::Value *MasterArrayEnd =
            Builder.CreateGEP(MasterArrayBegin, NumElements);
        // The basic structure here is a do-while loop, because we don't
        // need to check for the zero-element case.
        llvm::BasicBlock *BodyBB = createBasicBlock("omp.arraycpy.body");
        llvm::BasicBlock *DoneBB = createBasicBlock("omp.arraycpy.done");
        llvm::Value *IsEmpty =
            Builder.CreateICmpEQ(ArrayBeg, ArrayEnd, "omp.arraycpy.isempty");
        Builder.CreateCondBr(IsEmpty, DoneBB, BodyBB);

        // Enter the loop body, making that address the current address.
        llvm::BasicBlock *EntryBB = Builder.GetInsertBlock();
        EmitBlock(BodyBB);
        llvm::PHINode *ElementPast = Builder.CreatePHI(
            ArrayBeg->getType(), 2, "omp.arraycpy.elementPast");
        ElementPast->addIncoming(ArrayEnd, EntryBB);
        llvm::PHINode *MasterElementPast = Builder.CreatePHI(
            MasterArrayBegin->getType(), 2, "omp.arraycpy.masterElementPast");
        MasterElementPast->addIncoming(MasterArrayEnd, EntryBB);

        // Shift the address back by one element.
        llvm::Value *NegativeOne = llvm::ConstantInt::get(SizeTy, -1, true);
        llvm::Value *Element = Builder.CreateGEP(
            ElementPast, NegativeOne, "omp.arraycpy.element");
        llvm::Value *MasterElement = Builder.CreateGEP(
            MasterElementPast, NegativeOne, "omp.arraycpy.master.element");

        const VarDecl *PseudoVar =
            cast<VarDecl>(cast<DeclRefExpr>(*VarIter)->getDecl());
        CGM.OpenMPSupport.addOpenMPPrivateVar(PseudoVar, MasterElement);
        EmitAnyExprToMem(*InitIter, Element,
                         (*InitIter)->getType().getQualifiers(), false);
        CGM.OpenMPSupport.delOpenMPPrivateVar(PseudoVar);

        // Check whether we've reached the end.
        llvm::Value *Done =
            Builder.CreateICmpEQ(Element, ArrayBeg, "omp.arraycpy.done");
        Builder.CreateCondBr(Done, DoneBB, BodyBB);
        ElementPast->addIncoming(Element, Builder.GetInsertBlock());
        MasterElementPast->addIncoming(MasterElement, Builder.GetInsertBlock());

        // Done.
        EmitBlock(DoneBB, true);
      } else {
        // Create single object.
        llvm::Value *RealAddr = EmitLValue(*I).getAddress();
        const VarDecl *PseudoVar =
            cast<VarDecl>(cast<DeclRefExpr>(*VarIter)->getDecl());
        CGM.OpenMPSupport.addOpenMPPrivateVar(PseudoVar, RealAddr);
        EmitAnyExprToMem(*InitIter, Private,
                         (*InitIter)->getType().getQualifiers(), false);
        CGM.OpenMPSupport.delOpenMPPrivateVar(PseudoVar);
      }
    } else if (!PTask || CurFn != PTask) {
      EmitAnyExprToMem(*I, Private, QTy.getQualifiers(), false);
    }
    CGM.OpenMPSupport.addOpenMPPrivateVar(VD, Private);
  }
  // Disable marking for tasks.
  if (!PTask || PTask == CurFn)
    SetFirstprivateInsertPt(*this);
}

void CodeGenFunction::EmitPostOMPFirstPrivateClause(
    const OMPFirstPrivateClause &C, const OMPExecutableDirective &S) {
  // ~Type1(tmp1);
  // ~Type2(tmp2);
  // ...
  //
  for (OMPFirstPrivateClause::varlist_const_iterator I = C.varlist_begin(),
                                                     E = C.varlist_end();
       I != E; ++I) {
    // Get element type.
    const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(*I)->getDecl());
    llvm::Value *Private = CGM.OpenMPSupport.getTopOpenMPPrivateVar(VD);
    if (!Private)
      continue;
    bool LastPrivateFound = false;
    for (ArrayRef<OMPClause *>::iterator LI = S.clauses().begin(),
                                         LE = S.clauses().end();
         LI != LE; ++LI) {
      if (const OMPLastPrivateClause *LC =
              dyn_cast<OMPLastPrivateClause>(*LI)) {
        for (OMPFirstPrivateClause::varlist_const_iterator
                 VI = LC->varlist_begin(),
                 VE = LC->varlist_end();
             VI != VE; ++VI) {
          if (VD == cast<DeclRefExpr>(*VI)->getDecl()) {
            LastPrivateFound = true;
            break;
          }
        }
      }
      if (LastPrivateFound)
        break;
    }
    // Lastprivate cleanup is processed by lastprivate clause.
    if (LastPrivateFound)
      continue;
    QualType QTy = (*I)->getType();
    const Type *MainTy = QTy.getTypePtr();
    const Type *Ty = MainTy->getArrayElementTypeNoTypeQual();
    const Type *PrevTy = MainTy;
    while (Ty != 0) {
      PrevTy = Ty;
      Ty = Ty->getArrayElementTypeNoTypeQual();
    }
    Ty = PrevTy;
    CGM.OpenMPSupport.delOpenMPPrivateVar(VD);
    // CodeGen for classes with the destructor.
    if (CXXRecordDecl *RD = Ty->getAsCXXRecordDecl()) {
      // Find destructor.
      CXXDestructorDecl *Dtor = RD->getDestructor();
      if (Dtor && !Dtor->isTrivial()) {
        if (MainTy->getAsArrayTypeUnsafe()) {
          // Destroy array.
          emitDestroy(Private, QTy, getDestroyer(QualType::DK_cxx_destructor),
                      false);
        } else {
          // Destroy single object.
          EmitCXXDestructorCall(Dtor, Dtor_Complete, false, false, Private);
        }
      }
    }
  }
}

void
CodeGenFunction::EmitPreOMPLastPrivateClause(const OMPLastPrivateClause &C,
                                             const OMPExecutableDirective &S) {
  // Type1 tmp1;
  // Type2 tmp2;
  // ...
  //
  CGM.OpenMPSupport.setHasLastPrivate(true);
  ArrayRef<const Expr *>::iterator InitIter = C.getDefaultInits().begin();
  for (OMPLastPrivateClause::varlist_const_iterator I = C.varlist_begin(),
                                                    E = C.varlist_end();
       I != E; ++I, ++InitIter) {
    // Get element type.
    const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(*I)->getDecl());
    bool FirstPrivateFound = false;
    for (ArrayRef<OMPClause *>::iterator FI = S.clauses().begin(),
                                         FE = S.clauses().end();
         FI != FE; ++FI) {
      if (const OMPFirstPrivateClause *FC =
              dyn_cast<OMPFirstPrivateClause>(*FI)) {
        for (OMPFirstPrivateClause::varlist_const_iterator
                 VI = FC->varlist_begin(),
                 VE = FC->varlist_end();
             VI != VE; ++VI) {
          if (VD == cast<DeclRefExpr>(*VI)->getDecl()) {
            FirstPrivateFound = true;
            break;
          }
        }
      }
      if (FirstPrivateFound)
        break;
    }
    // Lastprivate init is processed by firstprivate clause.
    if (FirstPrivateFound || CGM.OpenMPSupport.getTopOpenMPPrivateVar(VD))
      continue;
    // if (VD->hasLocalStorage() &&
    //    (!CapturedStmtInfo ||
    //     !CapturedStmtInfo->lookup(VD))) {
    //  LocalDeclMap[VD] = CreateMemTemp(VD->getType(), CGM.getMangledName(VD));
    //}
    QualType QTy = (*I)->getType();
    const Type *MainTy = QTy.getTypePtr();
    // const Type *Ty = MainTy->getArrayElementTypeNoTypeQual();
    // const Type *PrevTy = MainTy;
    // while (Ty != 0) {
    //  PrevTy = Ty;
    //  Ty = Ty->getArrayElementTypeNoTypeQual();
    //}
    // Ty = PrevTy;
    llvm::Value *Private = 0;
    {
      LocalVarsDeclGuard Grd(*this, true);
      AutoVarEmission Emission = EmitAutoVarAlloca(*VD);
      Private = Emission.getAllocatedAddress();
    }
    // CodeGen for classes with the default constructor.
    if (!isTrivialInitializer(*InitIter) ||
        (MainTy->isVariablyModifiedType() && !MainTy->isPointerType())) {
      RunCleanupsScope InitBlock(*this);
      if (const ArrayType *ArrayTy = MainTy->getAsArrayTypeUnsafe()) {
        // Create array.
        QualType ElementTy;
        llvm::Value *ArrayBeg = Private;
        llvm::Value *NumElements =
            emitArrayLength(ArrayTy, ElementTy, ArrayBeg);
        llvm::Value *ArrayEnd = Builder.CreateGEP(ArrayBeg, NumElements,
                                                          "omp.arrayctor.end");
        // The basic structure here is a do-while loop, because we don't
        // need to check for the zero-element case.
        llvm::BasicBlock *BodyBB = createBasicBlock("omp.arrayctor.body");
        llvm::BasicBlock *DoneBB = createBasicBlock("omp.arrayctor.done");
        llvm::Value *IsEmpty =
            Builder.CreateICmpEQ(ArrayBeg, ArrayEnd, "omp.arrayctor.isempty");
        Builder.CreateCondBr(IsEmpty, DoneBB, BodyBB);

        // Enter the loop body, making that address the current address.
        llvm::BasicBlock *EntryBB = Builder.GetInsertBlock();
        EmitBlock(BodyBB);
        llvm::PHINode *ElementPast = Builder.CreatePHI(
            ArrayBeg->getType(), 2, "omp.arrayctor.elementPast");
        ElementPast->addIncoming(ArrayEnd, EntryBB);

        // Shift the address back by one element.
        llvm::Value *NegativeOne = llvm::ConstantInt::get(SizeTy, -1, true);
        llvm::Value *Element = Builder.CreateGEP(
            ElementPast, NegativeOne, "omp.arrayctor.element");
        EmitAnyExprToMem(*InitIter, Element,
                         (*InitIter)->getType().getQualifiers(), false);
        //// Check whether we've reached the end.
        llvm::Value *Done =
            Builder.CreateICmpEQ(Element, ArrayBeg, "omp.arrayctor.done");
        Builder.CreateCondBr(Done, DoneBB, BodyBB);
        ElementPast->addIncoming(Element, Builder.GetInsertBlock());

        // Done.
        EmitBlock(DoneBB, true);
      } else {
        EmitAnyExprToMem(*InitIter, Private,
                         (*InitIter)->getType().getQualifiers(), false);
      }
    }
    CGM.OpenMPSupport.addOpenMPPrivateVar(VD, Private);
  }
}

void
CodeGenFunction::EmitPostOMPLastPrivateClause(const OMPLastPrivateClause &C,
                                              const OMPExecutableDirective &S) {
  // ~Type1(tmp1);
  // ~Type2(tmp2);
  // ...
  //

  llvm::BasicBlock *LPBB, *LPEndBB;
  llvm::Instruction *LPIP;
  CGM.OpenMPSupport.getLastprivateIP(LPBB, LPIP, LPEndBB);
  if (!LPBB && !LPIP && !LPEndBB) {
    LPBB = createBasicBlock("omp.if.liter.start", CurFn);
    LPEndBB = createBasicBlock("omp.if.liter.end", CurFn);
    llvm::Value *LiterVal =
        Builder.CreateLoad(CGM.OpenMPSupport.getLastIterVar(), "liter");
    Builder.CreateCondBr(Builder.CreateIsNull(LiterVal), LPEndBB, LPBB);
    LPIP = LPBB->end();
    if (isLoopDirective(&S)) {
      Builder.SetInsertPoint(LPBB);
      EmitStmt(getFinalFromLoopDirective(&S));
      EnsureInsertPoint();
      LPBB = Builder.GetInsertBlock();
      LPIP = Builder.GetInsertPoint();
    }
    Builder.SetInsertPoint(LPEndBB);
    if (!CGM.OpenMPSupport.getNoWait())
      EmitOMPCallWithLocAndTidHelper(OPENMPRTL_FUNC(cancel_barrier), S.getLocEnd(),
                                     KMP_IDENT_BARRIER_IMPL);
  }
  ArrayRef<const Expr *>::iterator AssignIter = C.getAssignments().begin();
  ArrayRef<const Expr *>::iterator VarIter1 = C.getPseudoVars1().begin();
  ArrayRef<const Expr *>::iterator VarIter2 = C.getPseudoVars2().begin();
  for (OMPLastPrivateClause::varlist_const_iterator I = C.varlist_begin(),
                                                    E = C.varlist_end();
       I != E; ++I, ++AssignIter, ++VarIter1, ++VarIter2) {
    // Get element type.
    const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(*I)->getDecl());
    llvm::Value *Private = CGM.OpenMPSupport.getTopOpenMPPrivateVar(VD);
    if (!Private)
      continue;
    QualType QTy = (*I)->getType();
    const Type *MainTy = QTy.getTypePtr();
    const Type *Ty = MainTy->getArrayElementTypeNoTypeQual();
    const Type *PrevTy = MainTy;
    while (Ty != 0) {
      PrevTy = Ty;
      Ty = Ty->getArrayElementTypeNoTypeQual();
    }
    Ty = PrevTy;
    CGM.OpenMPSupport.delOpenMPPrivateVar(VD);
    CGBuilderTy::InsertPoint SavedIP = Builder.saveIP();
    Builder.SetInsertPoint(LPBB, LPIP);
    // CodeGen for classes with the copy assignment operator.
    if (!*AssignIter) {
      // For trivial assignment operator copy by memcpy.
      // EmitAnyExprToMem(*I, Private, QTy.getQualifiers(), false);
      // EmitAggregateAssign(EmitLValue(*I).getAddress(), Private,
      // VD->getType());
      EmitUniversalStore(EmitLValue(*I), Private, QTy);
    } else {
      RunCleanupsScope InitBlock(*this);
      // Copy elements one by one.
      if (const ArrayType *ArrayTy = MainTy->getAsArrayTypeUnsafe()) {
        // Copy array.
        QualType ElementTy;
        llvm::Value *SharedVar = EmitLValue(*I).getAddress();
        llvm::Value *NumElements =
            emitArrayLength(ArrayTy, ElementTy, SharedVar);
        llvm::Value *ArrayEnd =
            Builder.CreateGEP(SharedVar, NumElements);
        llvm::Value *MasterArray = Private;
        unsigned AddrSpace = MasterArray->getType()->getPointerAddressSpace();
        llvm::Type *BaseType = ConvertType(ElementTy)->getPointerTo(AddrSpace);
        llvm::Value *MasterArrayBegin = Builder.CreatePointerCast(
            MasterArray, BaseType, "master.array.begin");
        llvm::Value *MasterArrayEnd =
            Builder.CreateGEP(MasterArrayBegin, NumElements);
        // The basic structure here is a do-while loop, because we don't
        // need to check for the zero-element case.
        llvm::BasicBlock *BodyBB = createBasicBlock("omp.arraycpy.body");
        llvm::BasicBlock *DoneBB = createBasicBlock("omp.arraycpy.done");
        llvm::Value *IsEmpty =
            Builder.CreateICmpEQ(SharedVar, ArrayEnd, "omp.arraycpy.isempty");
        Builder.CreateCondBr(IsEmpty, DoneBB, BodyBB);

        // Enter the loop body, making that address the current address.
        llvm::BasicBlock *EntryBB = Builder.GetInsertBlock();
        EmitBlock(BodyBB);
        llvm::PHINode *ElementPast = Builder.CreatePHI(
            SharedVar->getType(), 2, "omp.arraycpy.elementPast");
        ElementPast->addIncoming(ArrayEnd, EntryBB);
        llvm::PHINode *MasterElementPast = Builder.CreatePHI(
            MasterArrayBegin->getType(), 2, "omp.arraycpy.masterElementPast");
        MasterElementPast->addIncoming(MasterArrayEnd, EntryBB);

        // Shift the address back by one element.
        llvm::Value *NegativeOne = llvm::ConstantInt::get(SizeTy, -1, true);
        llvm::Value *Element = Builder.CreateGEP(
            ElementPast, NegativeOne, "omp.arraycpy.element");
        llvm::Value *MasterElement = Builder.CreateGEP(
            MasterElementPast, NegativeOne, "omp.arraycpy.master.element");

        const VarDecl *PseudoVar1 =
            cast<VarDecl>(cast<DeclRefExpr>(*VarIter1)->getDecl());
        const VarDecl *PseudoVar2 =
            cast<VarDecl>(cast<DeclRefExpr>(*VarIter2)->getDecl());
        CGM.OpenMPSupport.addOpenMPPrivateVar(PseudoVar1, MasterElement);
        CGM.OpenMPSupport.addOpenMPPrivateVar(PseudoVar2, Element);
        EmitIgnoredExpr(*AssignIter);
        CGM.OpenMPSupport.delOpenMPPrivateVar(PseudoVar1);
        CGM.OpenMPSupport.delOpenMPPrivateVar(PseudoVar2);

        // Check whether we've reached the end.
        llvm::Value *Done =
            Builder.CreateICmpEQ(Element, SharedVar, "omp.arraycpy.done");
        Builder.CreateCondBr(Done, DoneBB, BodyBB);
        ElementPast->addIncoming(Element, Builder.GetInsertBlock());
        MasterElementPast->addIncoming(MasterElement, Builder.GetInsertBlock());

        // Done.
        EmitBlock(DoneBB, true);
      } else {
        // Copy single object.
        const VarDecl *PseudoVar1 =
            cast<VarDecl>(cast<DeclRefExpr>(*VarIter1)->getDecl());
        const VarDecl *PseudoVar2 =
            cast<VarDecl>(cast<DeclRefExpr>(*VarIter2)->getDecl());
        CGM.OpenMPSupport.addOpenMPPrivateVar(PseudoVar1,
                                              EmitLValue(*I).getAddress());
        CGM.OpenMPSupport.addOpenMPPrivateVar(PseudoVar2, Private);
        EmitIgnoredExpr(*AssignIter);
        CGM.OpenMPSupport.delOpenMPPrivateVar(PseudoVar1);
        CGM.OpenMPSupport.delOpenMPPrivateVar(PseudoVar2);
      }
    }
    LPBB = Builder.GetInsertBlock();
    LPIP = Builder.GetInsertPoint();
    Builder.restoreIP(SavedIP);
    if (!CGM.OpenMPSupport.isNewTask() &&
        CGM.OpenMPSupport.getPrevOpenMPPrivateVar(VD) == Private)
      continue;
    // CodeGen for classes with the destructor.
    if (CXXRecordDecl *RD = Ty->getAsCXXRecordDecl()) {
      // Find destructor.
      CXXDestructorDecl *Dtor = RD->getDestructor();
      if (Dtor && !Dtor->isTrivial()) {
        if (MainTy->getAsArrayTypeUnsafe()) {
          // Destroy array.
          emitDestroy(Private, QTy, getDestroyer(QualType::DK_cxx_destructor),
                      false);
        } else {
          // Destroy single object.
          EmitCXXDestructorCall(Dtor, Dtor_Complete, false, false, Private);
        }
      }
    }
  }
  CGM.OpenMPSupport.setLastprivateIP(LPBB, LPIP, LPEndBB);
}

void CodeGenFunction::EmitCloseOMPLastPrivateClause(
    const OMPLastPrivateClause &C, const OMPExecutableDirective &S) {
  // ~Type1(tmp1);
  // ~Type2(tmp2);
  // ...
  //

  llvm::BasicBlock *LPBB, *LPEndBB;
  llvm::Instruction *LPIP;
  CGM.OpenMPSupport.getLastprivateIP(LPBB, LPIP, LPEndBB);
  if (LPBB || LPIP || LPEndBB) {
    CGBuilderTy::InsertPoint SavedIP = Builder.saveIP();
    Builder.SetInsertPoint(LPBB, LPIP);
    EmitBranch(LPEndBB);
    Builder.restoreIP(SavedIP);
    CGM.OpenMPSupport.setLastprivateIP(0, 0, 0);
  }
}

void
CodeGenFunction::EmitInitOMPReductionClause(const OMPReductionClause &C,
                                            const OMPExecutableDirective &S) {
  assert(!isa<OMPSimdDirective>(S)); // Not yet supported
  // Type1 tmp1(var1);
  // anon.field1 = &tmp1;
  // Type2 tmp2(var2);
  // anon.field2 = &tmp2;
  // ...
  //
  // CodeGen for reduction clause.
  CodeGenFunction &CGF = CGM.OpenMPSupport.getCGFForReductionFunction();
  llvm::Function *ReductionFunc = CGF.CurFn;
  if (!ReductionFunc) {
    FunctionArgList Args;
    ImplicitParamDecl Arg1(0, SourceLocation(), 0, getContext().VoidPtrTy);
    ImplicitParamDecl Arg2(0, SourceLocation(), 0, getContext().VoidPtrTy);
    Args.push_back(&Arg1);
    Args.push_back(&Arg2);
    const CGFunctionInfo &FI = CGF.getTypes().arrangeFreeFunctionDeclaration(
        getContext().VoidTy, Args, FunctionType::ExtInfo(), false);
    llvm::FunctionType *FTy = CGF.getTypes().GetFunctionType(FI);
    llvm::Function *Fn = llvm::Function::Create(
        FTy, llvm::GlobalValue::InternalLinkage,
        StringRef(".omp_reduction_op."), &CGM.getModule());
    CGM.SetInternalFunctionAttributes(CurFuncDecl, Fn, FI);
    CGF.StartFunction(GlobalDecl(), getContext().VoidTy, Fn, FI, Args,
                      SourceLocation());
    ReductionFunc = CGF.CurFn;
  }

  for (OMPReductionClause::varlist_const_iterator I = C.varlist_begin(),
                                                  E = C.varlist_end();
       I != E; ++I) {
    // Get element type.
    const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(*I)->getDecl());
    QualType QTy = (*I)->getType();
    // if (!QTy->isScalarType())
    //  llvm_unreachable("Reduction variables with aggregate"
    //                   "types are not supported yet!");
    llvm::Type *PtrType = ConvertType(getContext().getPointerType(QTy));
    CGM.OpenMPSupport.registerReductionVar(VD, PtrType);
  }
}

void
CodeGenFunction::EmitPreOMPReductionClause(const OMPReductionClause &C,
                                           const OMPExecutableDirective &S) {
  assert(!isa<OMPSimdDirective>(S)); // Not yet supported
  // Type1 tmp1(var1);
  // anon.field1 = &tmp1;
  // Type2 tmp2(var2);
  // anon.field2 = &tmp2;
  // ...
  //
  llvm::Value *ReductionRecVar = CGM.OpenMPSupport.getReductionRecVar(*this);
  ArrayRef<const Expr *>::iterator InitIter = C.getDefaultInits().begin();
  for (OMPReductionClause::varlist_const_iterator I = C.varlist_begin(),
                                                  E = C.varlist_end();
       I != E; ++I, ++InitIter) {
    // Get element type.
    const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(*I)->getDecl());
    // if (VD->hasLocalStorage() &&
    //    (!CapturedStmtInfo ||
    //     !CapturedStmtInfo->lookup(VD))) {
    //  LocalDeclMap[VD] = CreateMemTemp(VD->getType(), CGM.getMangledName(VD));
    //}
    QualType QTy = (*I)->getType();
    llvm::AllocaInst *Private = 0;
    {
      LocalVarsDeclGuard Grd(*this, true);
      AutoVarEmission Emission = EmitAutoVarAlloca(*VD);
      Private = cast<llvm::AllocaInst>(Emission.getAllocatedAddress());
    }
    //         CreateMemTemp(QTy, CGM.getMangledName(VD) + ".reduction.");

    // CodeGen for classes with the constructor.
    // const Type *Ty = QTy.getTypePtr();
    if (!isTrivialInitializer(*InitIter)) {
      RunCleanupsScope InitBlock(*this);
      const FunctionDecl *FD = 0;
      if (const DeclRefExpr *DRE = dyn_cast_or_null<DeclRefExpr>(*InitIter)) {
        if (const FunctionDecl *D =
                dyn_cast_or_null<FunctionDecl>(DRE->getDecl()))
          FD = D;
      }
      if (FD && isa<OMPDeclareReductionDecl>(FD->getDeclContext())) {
        llvm::Value *RegularAddr = EmitLValue(*I).getAddress();
        llvm::Value *Args[] = { Private, RegularAddr };
        EmitCallOrInvoke(CGM.GetAddrOfGlobal(FD), Args);
        SetFirstprivateInsertPt(*this);
      } else {
        EmitAnyExprToMem(*InitIter, Private,
                         (*InitIter)->getType().getQualifiers(), false);
      }
    } else if (*InitIter) {
      switch (C.getOperator()) {
      case OMPC_REDUCTION_or:
      case OMPC_REDUCTION_bitxor:
      case OMPC_REDUCTION_bitor:
      case OMPC_REDUCTION_sub:
      case OMPC_REDUCTION_add: {
        llvm::Value *Zero =
            llvm::Constant::getNullValue(Private->getAllocatedType());
        InitTempAlloca(Private, Zero);
        break;
      }
      case OMPC_REDUCTION_and:
      case OMPC_REDUCTION_mult:
      case OMPC_REDUCTION_bitand: {
        llvm::Value *AllOnes =
            llvm::Constant::getAllOnesValue(Private->getAllocatedType());
        InitTempAlloca(Private, AllOnes);
        break;
      }
      case OMPC_REDUCTION_min:
      case OMPC_REDUCTION_max:
      case OMPC_REDUCTION_custom:
        llvm_unreachable("Operator kind not allowed.");
      case OMPC_REDUCTION_unknown:
      case NUM_OPENMP_REDUCTION_OPERATORS:
        llvm_unreachable("Unknown operator kind.");
      }
    } else {
      llvm::Type *Ty = ConvertTypeForMem(QTy);
      switch (C.getOperator()) {
      case OMPC_REDUCTION_or:
      case OMPC_REDUCTION_bitxor:
      case OMPC_REDUCTION_bitor:
      case OMPC_REDUCTION_sub:
      case OMPC_REDUCTION_add: {
        if (QTy->isIntegralOrEnumerationType()) {
          llvm::APInt InitVal = llvm::APInt::getNullValue(
              CGM.getDataLayout().getTypeStoreSizeInBits(Ty));
          llvm::Value *Init =
              llvm::ConstantInt::get(CGM.getLLVMContext(), InitVal);
          InitTempAlloca(Private, Init);
        } else if (QTy->isRealFloatingType()) {
          const llvm::fltSemantics &FS = Ty->getFltSemantics();
          llvm::APFloat InitVal = llvm::APFloat::getZero(FS);
          llvm::Value *Init =
              llvm::ConstantFP::get(CGM.getLLVMContext(), InitVal);
          InitTempAlloca(Private, Init);
        } else if (QTy->isPointerType()) {
          InitTempAlloca(Private, llvm::ConstantPointerNull::get(
                                      cast<llvm::PointerType>(Ty)));
        } else if (QTy->isAnyComplexType()) {
          const ComplexType *CmplxTy = QTy->castAs<ComplexType>();
          QualType ElTy = CmplxTy->getElementType();
          Ty = ConvertTypeForMem(ElTy);
          llvm::Value *Init;
          if (ElTy->isIntegralOrEnumerationType()) {
            llvm::APInt InitVal = llvm::APInt::getNullValue(
                CGM.getDataLayout().getTypeStoreSizeInBits(Ty));
            Init = llvm::ConstantInt::get(CGM.getLLVMContext(), InitVal);
          } else {
            const llvm::fltSemantics &FS = Ty->getFltSemantics();
            llvm::APFloat InitVal = llvm::APFloat::getZero(FS);
            Init = llvm::ConstantFP::get(CGM.getLLVMContext(), InitVal);
          }
          ComplexPairTy Value(Init, Init);
          LValue Dst = MakeNaturalAlignAddrLValue(Private, QTy);
          EmitStoreOfComplex(Value, Dst, true);
        }
        break;
      }
      case OMPC_REDUCTION_and:
      case OMPC_REDUCTION_mult: {
        if (QTy->isIntegralOrEnumerationType()) {
          llvm::APInt InitVal(CGM.getDataLayout().getTypeStoreSizeInBits(Ty),
                              1);
          llvm::Value *Init =
              llvm::ConstantInt::get(CGM.getLLVMContext(), InitVal);
          InitTempAlloca(Private, Init);
        } else if (QTy->isRealFloatingType()) {
          const llvm::fltSemantics &FS = Ty->getFltSemantics();
          llvm::APFloat InitVal(FS, 1);
          llvm::Value *Init =
              llvm::ConstantFP::get(CGM.getLLVMContext(), InitVal);
          InitTempAlloca(Private, Init);
        } else if (QTy->isPointerType()) {
          llvm::APInt InitVal(CGM.getDataLayout().getTypeStoreSizeInBits(Ty),
                              1);
          llvm::Constant *Init =
              llvm::ConstantInt::get(CGM.getLLVMContext(), InitVal);
          Init = llvm::ConstantExpr::getCast(llvm::Instruction::IntToPtr, Init,
                                             Ty);
          InitTempAlloca(Private, Init);
        } else if (QTy->isAnyComplexType()) {
          const ComplexType *CmplxTy = QTy->castAs<ComplexType>();
          QualType ElTy = CmplxTy->getElementType();
          Ty = ConvertTypeForMem(ElTy);
          llvm::Value *Init;
          if (ElTy->isIntegralOrEnumerationType()) {
            llvm::APInt InitVal(CGM.getDataLayout().getTypeStoreSizeInBits(Ty),
                                1);
            Init = llvm::ConstantInt::get(CGM.getLLVMContext(), InitVal);
          } else {
            const llvm::fltSemantics &FS = Ty->getFltSemantics();
            llvm::APFloat InitVal(FS, 1);
            Init = llvm::ConstantFP::get(CGM.getLLVMContext(), InitVal);
          }
          ComplexPairTy Value(Init, Init);
          LValue Dst = MakeNaturalAlignAddrLValue(Private, QTy);
          EmitStoreOfComplex(Value, Dst, true);
        }
        break;
      }
      case OMPC_REDUCTION_bitand: {
        if (QTy->isIntegralOrEnumerationType()) {
          llvm::APInt InitVal = llvm::APInt::getAllOnesValue(
              CGM.getDataLayout().getTypeStoreSizeInBits(Ty));
          llvm::Value *Init =
              llvm::ConstantInt::get(CGM.getLLVMContext(), InitVal);
          InitTempAlloca(Private, Init);
        } else if (QTy->isRealFloatingType()) {
          llvm::APFloat InitVal = llvm::APFloat::getAllOnesValue(
              CGM.getDataLayout().getTypeStoreSizeInBits(Ty));
          llvm::Value *Init =
              llvm::ConstantFP::get(CGM.getLLVMContext(), InitVal);
          InitTempAlloca(Private, Init);
        } else if (QTy->isPointerType()) {
          llvm::Value *Init = llvm::Constant::getAllOnesValue(Ty);
          InitTempAlloca(Private, Init);
        } else if (QTy->isAnyComplexType()) {
          const ComplexType *CmplxTy = QTy->castAs<ComplexType>();
          QualType ElTy = CmplxTy->getElementType();
          Ty = ConvertTypeForMem(ElTy);
          llvm::Value *Init;
          if (ElTy->isIntegralOrEnumerationType()) {
            llvm::APInt InitVal = llvm::APInt::getAllOnesValue(
                CGM.getDataLayout().getTypeStoreSizeInBits(Ty));
            Init = llvm::ConstantInt::get(CGM.getLLVMContext(), InitVal);
          } else {
            llvm::APFloat InitVal = llvm::APFloat::getAllOnesValue(
                CGM.getDataLayout().getTypeStoreSizeInBits(Ty));
            Init = llvm::ConstantFP::get(CGM.getLLVMContext(), InitVal);
          }
          ComplexPairTy Value(Init, Init);
          LValue Dst = MakeNaturalAlignAddrLValue(Private, QTy);
          EmitStoreOfComplex(Value, Dst, true);
        }
        break;
      }
      case OMPC_REDUCTION_min: {
        if (QTy->isSignedIntegerOrEnumerationType()) {
          llvm::APInt InitVal = llvm::APInt::getSignedMaxValue(
              CGM.getDataLayout().getTypeStoreSizeInBits(Ty));
          llvm::Value *Init =
              llvm::ConstantInt::get(CGM.getLLVMContext(), InitVal);
          InitTempAlloca(Private, Init);
        } else if (QTy->isUnsignedIntegerOrEnumerationType()) {
          llvm::APInt InitVal = llvm::APInt::getMaxValue(
              CGM.getDataLayout().getTypeStoreSizeInBits(Ty));
          llvm::Value *Init =
              llvm::ConstantInt::get(CGM.getLLVMContext(), InitVal);
          InitTempAlloca(Private, Init);
        } else if (QTy->isRealFloatingType()) {
          const llvm::fltSemantics &FS = Ty->getFltSemantics();
          llvm::APFloat InitVal = llvm::APFloat::getLargest(FS);
          llvm::Value *Init =
              llvm::ConstantFP::get(CGM.getLLVMContext(), InitVal);
          InitTempAlloca(Private, Init);
        } else if (QTy->isPointerType()) {
          llvm::APInt InitVal = llvm::APInt::getMaxValue(
              CGM.getDataLayout().getTypeStoreSizeInBits(Ty));
          llvm::Constant *Init =
              llvm::ConstantInt::get(CGM.getLLVMContext(), InitVal);
          Init = llvm::ConstantExpr::getCast(llvm::Instruction::IntToPtr, Init,
                                             Ty);
          InitTempAlloca(Private, Init);
        }
        break;
      }
      case OMPC_REDUCTION_max: {
        if (QTy->isSignedIntegerOrEnumerationType()) {
          llvm::APInt InitVal = llvm::APInt::getSignedMinValue(
              CGM.getDataLayout().getTypeStoreSizeInBits(Ty));
          llvm::Value *Init =
              llvm::ConstantInt::get(CGM.getLLVMContext(), InitVal);
          InitTempAlloca(Private, Init);
        } else if (QTy->isUnsignedIntegerOrEnumerationType()) {
          llvm::APInt InitVal = llvm::APInt::getMinValue(
              CGM.getDataLayout().getTypeStoreSizeInBits(Ty));
          llvm::Value *Init =
              llvm::ConstantInt::get(CGM.getLLVMContext(), InitVal);
          InitTempAlloca(Private, Init);
        } else if (QTy->isRealFloatingType()) {
          const llvm::fltSemantics &FS = Ty->getFltSemantics();
          llvm::APFloat InitVal = llvm::APFloat::getLargest(FS, true);
          llvm::Value *Init =
              llvm::ConstantFP::get(CGM.getLLVMContext(), InitVal);
          InitTempAlloca(Private, Init);
        } else if (QTy->isPointerType()) {
          llvm::APInt InitVal = llvm::APInt::getMinValue(
              CGM.getDataLayout().getTypeStoreSizeInBits(Ty));
          llvm::Constant *Init =
              llvm::ConstantInt::get(CGM.getLLVMContext(), InitVal);
          Init = llvm::ConstantExpr::getCast(llvm::Instruction::IntToPtr, Init,
                                             Ty);
          InitTempAlloca(Private, Init);
        }
        break;
      }
      case OMPC_REDUCTION_custom:
        llvm_unreachable("Custom initialization cannot be NULLed.");
      case OMPC_REDUCTION_unknown:
      case NUM_OPENMP_REDUCTION_OPERATORS:
        llvm_unreachable("Unkonwn operator kind.");
      }
    }
    llvm::Value *Addr = Builder.CreateConstGEP2_32(
        ReductionRecVar, 0, CGM.OpenMPSupport.getReductionVarIdx(VD),
        CGM.getMangledName(VD) + ".addr");
    Builder.CreateStore(Private, Addr);
    // llvm::Value *Var = Builder.CreateLoad(Addr, CGM.getMangledName(VD));
    CGM.OpenMPSupport.addOpenMPPrivateVar(VD, Private);
  }
}

void
CodeGenFunction::EmitPostOMPReductionClause(const OMPReductionClause &C,
                                            const OMPExecutableDirective &S) {
  assert(!isa<OMPSimdDirective>(S)); // Not yet supported
  CodeGenFunction &CGF = CGM.OpenMPSupport.getCGFForReductionFunction();
  llvm::Function *ReduceFunc = CGF.CurFn;
  llvm::SwitchInst *Switch = dyn_cast_or_null<llvm::SwitchInst>(
      CGM.OpenMPSupport.getReductionSwitch());
  llvm::BasicBlock *RedBB1;
  llvm::BasicBlock *RedBB2;
  llvm::Instruction *IP1;
  llvm::Instruction *IP2;
  if (!Switch) {
    // __kmpc_reduce[_nowait](ident_t *loc, int32_t global_tid, int32_t
    // num_vars,
    //                      size_t reduce_size, void *reduce_data,
    //                     kmp_reduce_func reduce_func, kmp_critical_name *lck);
    // ident_t loc = {...};
    llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(C.getLocStart(), *this,
                                                   KMP_IDENT_ATOMIC_REDUCE);
    // global_tid = __kmpc_global_thread_num(...);
    llvm::Value *GTid = CGM.CreateOpenMPGlobalThreadNum(C.getLocStart(), *this);
    // int num_vars = c;
    unsigned NumVars = CGM.OpenMPSupport.getNumberOfReductionVars();
    llvm::Value *NumVarsVal = llvm::ConstantInt::get(Int32Ty, NumVars);
    // size_t reduce_size = sizeof(rec);
    uint64_t ReduceSize = CGM.getDataLayout().getTypeAllocSize(
        CGM.OpenMPSupport.getReductionRec());
    llvm::Value *ReduceSizeVal = llvm::ConstantInt::get(SizeTy, ReduceSize);
    // void *reduce_data = (void *)rec;
    llvm::Value *ReduceData =
        Builder.CreatePointerCast(CGM.OpenMPSupport.getReductionRecVar(*this),
                                  VoidPtrTy, "(void*)reductionrec");
    // kmpc_reduce_func reduce_func = reduce_func;
    // kmp_critical_name lck;
    llvm::Type *LckTy =
        llvm::TypeBuilder<kmp_critical_name, false>::get(CGM.getLLVMContext());

    llvm::GlobalVariable *Lck = CreateRuntimeVariable(CGM, ".lck.", LckTy);
    CGM.OpenMPSupport.setReductionLockVar(Lck);
    llvm::Value *RealArgs[] = { Loc,           GTid,       NumVarsVal,
                                ReduceSizeVal, ReduceData, ReduceFunc,
                                Lck };
    llvm::CallInst *Res = EmitRuntimeCall(CGM.OpenMPSupport.getNoWait()
                                              ? OPENMPRTL_FUNC(reduce_nowait)
                                              : OPENMPRTL_FUNC(reduce),
                                          RealArgs);
    RedBB1 = createBasicBlock("reduction.case1", CurFn);
    RedBB2 = createBasicBlock("reduction.case2", CurFn);
    llvm::BasicBlock *DefaultBlock =
        createBasicBlock("reduction.continue", CurFn);
    Switch = Builder.CreateSwitch(Res, DefaultBlock, 2);
    Switch->addCase(llvm::ConstantInt::get(Int32Ty, 1), RedBB1);
    Switch->addCase(llvm::ConstantInt::get(Int32Ty, 2), RedBB2);
    IP1 = RedBB1->end();
    IP2 = RedBB2->end();
    Builder.SetInsertPoint(DefaultBlock);
    CGM.OpenMPSupport.setReductionSwitch(Switch);
  } else {
    CGM.OpenMPSupport.getReductionIPs(RedBB1, IP1, RedBB2, IP2);
  }
  llvm::Value *ReductionRecVar = CGM.OpenMPSupport.getReductionRecVar(*this);
  ArrayRef<const Expr *>::iterator Par1I = C.getHelperParameters1st().begin();
  ArrayRef<const Expr *>::iterator Par2I = C.getHelperParameters2nd().begin();
  ArrayRef<const Expr *>::iterator OpI = C.getOpExprs().begin();
  for (OMPReductionClause::varlist_const_iterator I = C.varlist_begin(),
                                                  E = C.varlist_end();
       I != E; ++I, ++Par1I, ++Par2I, ++OpI) {
    // Get element type.
    const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(*I)->getDecl());
    QualType QTy = (*I)->getType();
    llvm::Value *Private = CGM.OpenMPSupport.getTopOpenMPPrivateVar(VD);
    if (!Private)
      continue;
    CGM.OpenMPSupport.delOpenMPPrivateVar(VD);

    CGBuilderTy::InsertPoint SavedIP = Builder.saveIP();
    Builder.SetInsertPoint(RedBB1, IP1);
    const VarDecl *Par1 = cast<VarDecl>(cast<DeclRefExpr>(*Par1I)->getDecl());
    const VarDecl *Par2 = cast<VarDecl>(cast<DeclRefExpr>(*Par2I)->getDecl());
    QualType PtrQTy = getContext().getPointerType(QTy);
    llvm::AllocaInst *AI =
        CreateMemTemp(PtrQTy, CGM.getMangledName(VD) + ".addr.lhs.");
    LValue LVal = MakeNaturalAlignAddrLValue(AI, PtrQTy);
    UnaryOperator UOp(const_cast<Expr *>(*I), UO_AddrOf, PtrQTy, VK_LValue,
                      OK_Ordinary, SourceLocation());
    // EmitExprAsInit(&UOp, VD, LVal, false);
    EmitAnyExprToMem(&UOp, AI, UOp.getType().getQualifiers(), false);
    llvm::Value *Addr2 = Builder.CreateConstGEP2_32(
        ReductionRecVar, 0, CGM.OpenMPSupport.getReductionVarIdx(VD),
        CGM.getMangledName(VD) + ".addr.rhs");
    CGM.OpenMPSupport.addOpenMPPrivateVar(Par1, AI);
    CGM.OpenMPSupport.addOpenMPPrivateVar(Par2, Addr2);
    EmitIgnoredExpr(*OpI);
    CGM.OpenMPSupport.delOpenMPPrivateVar(Par1);
    CGM.OpenMPSupport.delOpenMPPrivateVar(Par2);
    IP1 = Builder.GetInsertPoint();
    RedBB1 = Builder.GetInsertBlock();
    Builder.SetInsertPoint(RedBB2, IP2);
    llvm::Value *AtomicFunc = OPENMPRTL_ATOMIC_FUNC(QTy, C.getOperator());
    if (isa<BinaryOperator>((*OpI)->IgnoreImpCasts()) && AtomicFunc) {
      // __kmpc_atomic_...(&loc, global_tid, &glob, &reduction);
      // ident_t loc = {...};
      llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(C.getLocStart(), *this);
      // global_tid = __kmpc_global_thread_num(...);
      llvm::Value *GTid =
          CGM.CreateOpenMPGlobalThreadNum(C.getLocStart(), *this);
      Addr2 = Builder.CreateConstGEP2_32(
          ReductionRecVar, 0, CGM.OpenMPSupport.getReductionVarIdx(VD),
          CGM.getMangledName(VD) + ".addr.rhs");
      llvm::Type *ArgTy = ConvertTypeForMem(getAtomicType(*this, QTy));
      llvm::Type *PtrArgTy = ArgTy->getPointerTo();
      llvm::Value *RealArgs[] = {
        Loc, GTid, Builder.CreatePointerCast(EmitScalarExpr(&UOp), PtrArgTy),
        Builder.CreateLoad(Builder.CreatePointerCast(
            Builder.CreateLoad(Addr2, CGM.getMangledName(VD) + ".rhs"),
            PtrArgTy))
      };
      EmitRuntimeCall(AtomicFunc, RealArgs);
    } else {
      // __kmpc_atomic_start();
      EmitRuntimeCall(OPENMPRTL_FUNC(atomic_start));
      AI = CreateMemTemp(PtrQTy, CGM.getMangledName(VD) + ".addr.lhs.");
      LVal = MakeNaturalAlignAddrLValue(AI, PtrQTy);
      EmitAnyExprToMem(&UOp, AI, UOp.getType().getQualifiers(), false);
      // EmitExprAsInit(&UOp, VD, LVal, false);
      Addr2 = Builder.CreateConstGEP2_32(
          ReductionRecVar, 0, CGM.OpenMPSupport.getReductionVarIdx(VD),
          CGM.getMangledName(VD) + "addr.rhs");
      CGM.OpenMPSupport.addOpenMPPrivateVar(Par1, AI);
      CGM.OpenMPSupport.addOpenMPPrivateVar(Par2, Addr2);
      EmitIgnoredExpr(*OpI);
      CGM.OpenMPSupport.delOpenMPPrivateVar(Par1);
      CGM.OpenMPSupport.delOpenMPPrivateVar(Par2);
      // __kmpc_atomic_end();
      EmitRuntimeCall(OPENMPRTL_FUNC(atomic_end));
    }
    IP2 = Builder.GetInsertPoint();
    RedBB2 = Builder.GetInsertBlock();
    Builder.restoreIP(SavedIP);
    // CodeGen for classes with the destructor.
    const Type *Ty = QTy.getTypePtr();
    if (CXXRecordDecl *RD = Ty->getAsCXXRecordDecl()) {
      // Find destructor.
      CXXDestructorDecl *Dtor = RD->getDestructor();
      if (Dtor && !Dtor->isTrivial()) {
        // Destroy single object.
        EmitCXXDestructorCall(Dtor, Dtor_Complete, false, false, Private);
      }
    }
  }
  CGM.OpenMPSupport.setReductionIPs(RedBB1, IP1, RedBB2, IP2);
}

llvm::CallInst *CodeGenFunction::EmitOMPCallWithLocAndTidHelper(
    llvm::Value *F, SourceLocation L, unsigned Flags) {
  llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(L, *this, Flags);
  llvm::Value *GTid = CGM.CreateOpenMPGlobalThreadNum(L, *this);
  llvm::Value *RealArgs[] = { Loc, GTid };
  return EmitRuntimeCall(F, RealArgs);
}

void
CodeGenFunction::EmitOMPCapturedBodyHelper(const OMPExecutableDirective &S) {
  // TODO: We may inline instead of calling it...
  RunCleanupsScope MyScope(*this);
  EmitStmt(cast<CapturedStmt>(S.getAssociatedStmt())->getCapturedStmt());
  EnsureInsertPoint();
}

void CodeGenFunction::EmitOMPConditionalIfHelper(
    const OMPExecutableDirective &S, llvm::Value *Func, SourceLocation Loc,
    llvm::Value *EndFunc, SourceLocation EndLoc, bool HasClauses,
    llvm::AllocaInst *DidIt, const std::string &NameStr) {

  // This is for master and single directives:
  // if (__kmpc_Call()) {
  //   <captured_body>
  //   __kmpc_EndCall();
  // }
  //
  if (HasClauses) {
    // Pre-process private and firstprivate clauses
    for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                         E = S.clauses().end();
         I != E; ++I) {
      if (*I)
        EmitPreOMPClause(*(*I), S);
    }
  }

  if (DidIt) {
    // Store 0 into .did_it. flag
    llvm::Value *Zero =
        llvm::Constant::getNullValue(ConvertTypeForMem(getContext().IntTy));
    EmitStoreOfScalar(Zero, DidIt, false,
                      CGM.getDataLayout().getPrefTypeAlignment(
                          ConvertTypeForMem(getContext().IntTy)),
                      getContext().IntTy);
  }

  // Start with emission of __kmpc_Call()
  llvm::CallInst *Call = EmitOMPCallWithLocAndTidHelper(Func, Loc);
  // Convert Call's result to bool, to use in IF-stmt
  llvm::Value *CallBool =
      EmitScalarConversion(Call, getContext().IntTy, getContext().BoolTy);
  // Generate the basic blocks
  llvm::BasicBlock *ThenBlock = createBasicBlock((NameStr + ".then").c_str());
  llvm::BasicBlock *ContBlock = createBasicBlock((NameStr + ".end").c_str());
  // Generate the branch (If-stmt)
  Builder.CreateCondBr(CallBool, ThenBlock, ContBlock);
  EmitBlock(ThenBlock);
  // Here we are on Then-branch -- emit captured body and __kmpc_EndCall()
  EmitOMPCapturedBodyHelper(S);
  if (DidIt) {
    // Store 1 into .did_it. flag
    llvm::Value *One = llvm::ConstantInt::get(
        CGM.getLLVMContext(),
        llvm::APInt::getLowBitsSet(CGM.getDataLayout().getTypeStoreSizeInBits(
                                       ConvertTypeForMem(getContext().IntTy)),
                                   1));
    EmitStoreOfScalar(One, DidIt, false,
                      CGM.getDataLayout().getPrefTypeAlignment(
                          DidIt->getType()->getSequentialElementType()),
                      getContext().IntTy);
  }
  EmitOMPCallWithLocAndTidHelper(EndFunc, EndLoc);
  // Emit the rest of bblocks/branches
  EmitBranch(ContBlock);
  EmitBlock(ContBlock, true);

  if (HasClauses) {
    // Post-process private and firstprivate clauses
    for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                         E = S.clauses().end();
         I != E; ++I) {
      if (*I)
        EmitPostOMPClause(*(*I), S);
    }
  }
}

/// "One-call" OMP Directives (barrier, taskyield, taskwait, flush).
/// '#pragma omp barrier' directive.
void CodeGenFunction::EmitOMPBarrierDirective(const OMPBarrierDirective &S) {
  // EmitUntiedPartIdInc(*this);
  EmitOMPCallWithLocAndTidHelper(OPENMPRTL_FUNC(cancel_barrier), S.getLocStart(),
                                 KMP_IDENT_BARRIER_EXPL);
  // EmitUntiedBranchEnd(*this);
  // EmitUntiedTaskSwitch(*this, false);
}

/// '#pragma omp taskyield' directive.
void
CodeGenFunction::EmitOMPTaskyieldDirective(const OMPTaskyieldDirective &S) {
  // EmitUntiedPartIdInc(*this);
  llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
  llvm::Value *GTid = CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), *this);
  llvm::Value *RealArgs[] = { Loc, GTid, Builder.getInt32(0) };
  EmitRuntimeCall(OPENMPRTL_FUNC(omp_taskyield), RealArgs);
  // EmitUntiedBranchEnd(*this);
  // EmitUntiedTaskSwitch(*this, false);
}

/// '#pragma omp taskwait' directive.
void CodeGenFunction::EmitOMPTaskwaitDirective(const OMPTaskwaitDirective &S) {
  // If the task is untied, we may want to generate IF-stmt here:
  // if (__kmpc_omp_taskwait(loc_task_wait, gtid) == CURRENT_TASK_QUEUED) {
  //      T-return; // Exit t1 if it was suspended or queued
  // }
  // But currently RTL always returns TASK_CURRENT_NOT_QUEUED,
  // so probably that make no sence.
  //
  EmitUntiedPartIdInc(*this);
  llvm::Value *Res = EmitOMPCallWithLocAndTidHelper(
      OPENMPRTL_FUNC(omp_taskwait), S.getLocStart());
  if (CGM.OpenMPSupport.getUntied()) {
    llvm::BasicBlock *ThenBB = createBasicBlock("taskwait.then");
    llvm::BasicBlock *EndBB = createBasicBlock("taskwait.end");
    llvm::Value *Cond =
        Builder.CreateICmpEQ(Res, Builder.getInt32(OMP_TASK_CURRENT_QUEUED));
    Builder.CreateCondBr(Cond, ThenBB, EndBB);
    EmitBlock(ThenBB);
    EmitUntiedBranchEnd(*this);
    EmitBlock(EndBB);
    EmitUntiedTaskSwitch(*this, true);
  }
}

/// '#pragma omp flush' directive.
void CodeGenFunction::EmitOMPFlushDirective(const OMPFlushDirective &S) {
  SmallVector<llvm::Value *, 4> Args;
  Args.push_back(CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this));
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E; ++I) {
    const OMPFlushClause *C = cast<OMPFlushClause>(*I);
    for (ArrayRef<const Expr *>::iterator J = C->varlist_begin(),
                                          F = C->varlist_end();
         J != F; ++J) {
      QualType QTy = (*J)->getType();
      QualType PtrQTy = getContext().getPointerType(QTy);
      UnaryOperator UOp(const_cast<Expr *>(*J), UO_AddrOf, PtrQTy, VK_LValue,
                        OK_Ordinary, S.getLocStart());
      llvm::Value *Val = EmitScalarExpr(&UOp);
      Args.push_back(Val);
    }
  }
  EmitRuntimeCall(OPENMPRTL_FUNC(flush), Args);
}

/// '#pragma omp cancel' directive.
void CodeGenFunction::EmitOMPCancelDirective(const OMPCancelDirective &S) {
  llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
  llvm::Value *GTid = CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), *this);
  int Kind = KMP_CANCEL_NOREQ;
  switch (S.getConstructType()) {
  case OMPD_parallel:
    Kind = KMP_CANCEL_PARALLEL;
    break;
  case OMPD_for:
    Kind = KMP_CANCEL_LOOP;
    break;
  case OMPD_sections:
    Kind = KMP_CANCEL_SECTIONS;
    break;
  case OMPD_taskgroup:
    Kind = KMP_CANCEL_TASKGROUP;
    break;
  default:
    llvm_unreachable("Unknown construct type in cancellation point");
    break;
  }
  llvm::Value *RealArgs[] = { Loc, GTid, Builder.getInt32(Kind) };
  EmitRuntimeCall(OPENMPRTL_FUNC(cancel), RealArgs);
  llvm_unreachable("Not supportd yet");
}

/// '#pragma omp cancellation point' directive.
void CodeGenFunction::EmitOMPCancellationPointDirective(
    const OMPCancellationPointDirective &S) {
  llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
  llvm::Value *GTid = CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), *this);
  int Kind = KMP_CANCEL_NOREQ;
  switch (S.getConstructType()) {
  case OMPD_parallel:
    Kind = KMP_CANCEL_PARALLEL;
    break;
  case OMPD_for:
    Kind = KMP_CANCEL_LOOP;
    break;
  case OMPD_sections:
    Kind = KMP_CANCEL_SECTIONS;
    break;
  case OMPD_taskgroup:
    Kind = KMP_CANCEL_TASKGROUP;
    break;
  default:
    llvm_unreachable("Unknown construct type in cancellation point");
    break;
  }
  llvm::Value *RealArgs[] = { Loc, GTid, Builder.getInt32(Kind) };
  EmitRuntimeCall(OPENMPRTL_FUNC(cancellationpoint), RealArgs);
  llvm_unreachable("Not supportd yet");
}

/// Atomic OMP Directive -- pattern match and emit one RTL call.
/// In the future, we may want to generate some atomic llvm instruction
/// instead of RTL call here for some atomic directives.
void CodeGenFunction::EmitOMPAtomicDirective(const OMPAtomicDirective &S) {
  CGM.OpenMPSupport.startOpenMPRegion(false);
  bool IsSeqCst = false;
  bool AtLeastOneLoopTaken = false;
  OpenMPClauseKind Kind = OMPC_update;
  for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                       E = S.clauses().end();
       I != E || !AtLeastOneLoopTaken; ++I) {
    if (I != S.clauses().end()) {
      if ((*I)->getClauseKind() == OMPC_seq_cst) {
        IsSeqCst = true;
        continue;
      }
      Kind = (*I)->getClauseKind();
    }
    LValue X = EmitLValue(S.getX()->IgnoreParenLValueCasts());
    switch (Kind) {
    case OMPC_read: {
      QualType QTy = S.getX()->getType();
      QualType AQTy = getAtomicType(*this, QTy);
      llvm::Value *AtomicFunc =
          AQTy.isNull() ? 0 : OPENMPRTL_ATOMIC_FUNC_GENERAL(
                                  AQTy, AQTy, OMP_Atomic_rd, false, false);
      if (X.isSimple() && AtomicFunc) {
        llvm::Type *ATy = ConvertTypeForMem(AQTy);
        llvm::SmallVector<llvm::Value *, 5> Args;
        // __kmpc_atomic_..._rd(&loc, global_tid, &x);
        // ident_t loc = {...};
        llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
        // global_tid = __kmpc_global_thread_num(...);
        llvm::Value *GTid =
            CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), *this);
        Args.push_back(Loc);
        Args.push_back(GTid);
        Args.push_back(
            Builder.CreatePointerCast(X.getAddress(), ATy->getPointerTo()));
        llvm::Value *Res = EmitRuntimeCall(AtomicFunc, Args);
        // v = x;
        Res = EmitScalarConversion(Res, AQTy, S.getV()->getType());
        EmitStoreOfScalar(Res, EmitLValue(S.getV()));
      } else {
        EmitRuntimeCall(OPENMPRTL_FUNC(atomic_start));
        RValue Val = EmitLoadOfLValue(X, S.getLocStart());
        EmitRuntimeCall(OPENMPRTL_FUNC(atomic_end));
        EmitStoreThroughLValue(Val, EmitLValue(S.getV()));
      }
      break;
    }
    case OMPC_write: {
      QualType QTy = S.getX()->getType();
      QualType AQTy = getAtomicType(*this, QTy);
      QualType QTyIn = S.getExpr()->getType();
      llvm::Value *AtomicFunc =
          AQTy.isNull() ? 0 : OPENMPRTL_ATOMIC_FUNC_GENERAL(
                                  AQTy, AQTy, OMP_Atomic_wr, false, false);
      if (X.isSimple() && AtomicFunc && QTyIn->isScalarType() &&
          !QTyIn->isAnyComplexType()) {
        llvm::Type *ATy = ConvertTypeForMem(AQTy);
        llvm::SmallVector<llvm::Value *, 5> Args;
        // __kmpc_atomic_..._wr(&loc, global_tid, &x, expr);
        // ident_t loc = {...};
        llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
        // global_tid = __kmpc_global_thread_num(...);
        llvm::Value *GTid =
            CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), *this);
        Args.push_back(Loc);
        Args.push_back(GTid);
        Args.push_back(
            Builder.CreatePointerCast(X.getAddress(), ATy->getPointerTo()));
        Args.push_back(
            EmitScalarConversion(EmitAnyExpr(S.getExpr()).getScalarVal(),
                                 S.getExpr()->getType(), AQTy));
        EmitRuntimeCall(AtomicFunc, Args);
      } else {
        RValue Val = EmitAnyExpr(S.getExpr());
        EmitRuntimeCall(OPENMPRTL_FUNC(atomic_start));
        EmitStoreThroughLValue(Val, X);
        EmitRuntimeCall(OPENMPRTL_FUNC(atomic_end));
      }
      break;
    }
    case OMPC_update: {
      QualType QTyRes = S.getX()->getType();
      QualType AQTyRes = getAtomicType(*this, QTyRes);
      QualType QTyIn = S.getExpr()->getType();
      QualType AQTyIn = getAtomicType(*this, QTyIn);
      EAtomicOperation Aop;
      switch (S.getOperator()) {
      case BO_Add:
        Aop = OMP_Atomic_add;
        break;
      case BO_Sub:
        Aop = OMP_Atomic_sub;
        break;
      case BO_Mul:
        Aop = OMP_Atomic_mul;
        break;
      case BO_Div:
        Aop = OMP_Atomic_div;
        break;
      case BO_And:
        Aop = OMP_Atomic_andb;
        break;
      case BO_Or:
        Aop = OMP_Atomic_orb;
        break;
      case BO_Xor:
        Aop = OMP_Atomic_xor;
        break;
      case BO_Shl:
        Aop = OMP_Atomic_shl;
        break;
      case BO_Shr:
        Aop = OMP_Atomic_shr;
        break;
      default:
        Aop = OMP_Atomic_invalid;
        break;
      }
      llvm::Value *AtomicFunc =
          (AQTyRes.isNull() || AQTyIn.isNull())
              ? 0
              : OPENMPRTL_ATOMIC_FUNC_GENERAL(AQTyRes, AQTyIn, Aop, false,
                                              S.isReversed());
      if (X.isSimple() && AtomicFunc && QTyIn->isScalarType() &&
          !QTyIn->isAnyComplexType()) {
        llvm::Type *ATyRes = ConvertTypeForMem(AQTyRes);
        llvm::SmallVector<llvm::Value *, 5> Args;
        // __kmpc_atomic_..._op(&loc, global_tid, &x, expr);
        // ident_t loc = {...};
        llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
        // global_tid = __kmpc_global_thread_num(...);
        llvm::Value *GTid =
            CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), *this);
        Args.push_back(Loc);
        Args.push_back(GTid);
        Args.push_back(
            Builder.CreatePointerCast(X.getAddress(), ATyRes->getPointerTo()));
        Args.push_back(EmitAnyExpr(S.getExpr()).getScalarVal());
        EmitRuntimeCall(AtomicFunc, Args);
      } else {
        EmitRuntimeCall(OPENMPRTL_FUNC(atomic_start));
        EmitStmt(cast<CapturedStmt>(S.getAssociatedStmt())->getCapturedStmt());
        EmitRuntimeCall(OPENMPRTL_FUNC(atomic_end));
      }
      break;
    }
    case OMPC_capture: {
      QualType QTyRes = S.getX()->getType();
      QualType AQTyRes = getAtomicType(*this, QTyRes);
      QualType QTyIn = S.getExpr()->getType();
      QualType AQTyIn = getAtomicType(*this, QTyIn);
      EAtomicOperation Aop;
      switch (S.getOperator()) {
      case BO_Add:
        Aop = OMP_Atomic_add;
        break;
      case BO_Sub:
        Aop = OMP_Atomic_sub;
        break;
      case BO_Mul:
        Aop = OMP_Atomic_mul;
        break;
      case BO_Div:
        Aop = OMP_Atomic_div;
        break;
      case BO_And:
        Aop = OMP_Atomic_andb;
        break;
      case BO_Or:
        Aop = OMP_Atomic_orb;
        break;
      case BO_Xor:
        Aop = OMP_Atomic_xor;
        break;
      case BO_Shl:
        Aop = OMP_Atomic_shl;
        break;
      case BO_Shr:
        Aop = OMP_Atomic_shr;
        break;
      case BO_Assign:
        Aop = OMP_Atomic_assign;
        break;
      default:
        Aop = OMP_Atomic_invalid;
        break;
      }
      llvm::Value *AtomicFunc =
          (AQTyRes.isNull() || AQTyIn.isNull())
              ? 0
              : OPENMPRTL_ATOMIC_FUNC_GENERAL(AQTyRes, AQTyIn, Aop, true,
                                              S.isReversed());
      if (X.isSimple() && AtomicFunc && QTyIn->isScalarType() &&
          !QTyIn->isAnyComplexType()) {
        llvm::Type *ATy = ConvertTypeForMem(AQTyRes);
        llvm::SmallVector<llvm::Value *, 5> Args;
        // __kmpc_atomic_..._op(&loc, global_tid, &x, expr);
        // ident_t loc = {...};
        llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
        // global_tid = __kmpc_global_thread_num(...);
        llvm::Value *GTid =
            CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), *this);
        Args.push_back(Loc);
        Args.push_back(GTid);
        Args.push_back(
            Builder.CreatePointerCast(X.getAddress(), ATy->getPointerTo()));
        Args.push_back(EmitAnyExpr(S.getExpr()).getScalarVal());
        Args.push_back(Builder.getInt32(S.isCaptureAfter() ? 1 : 0));
        llvm::Value *Res = EmitRuntimeCall(AtomicFunc, Args);
        // v = x;
        Res = EmitScalarConversion(Res, AQTyRes, S.getV()->getType());
        EmitStoreOfScalar(Res, EmitLValue(S.getV()));
      } else {
        EmitRuntimeCall(OPENMPRTL_FUNC(atomic_start));
        EmitStmt(cast<CapturedStmt>(S.getAssociatedStmt())->getCapturedStmt());
        EmitRuntimeCall(OPENMPRTL_FUNC(atomic_end));
      }
      break;
    }
    case OMPC_seq_cst:
      llvm_unreachable("SEQ_CST should be processed already.");
      break;
    default:
      llvm_unreachable("Not allowed operation in atomic directive.");
    }
    if (I == E && !AtLeastOneLoopTaken)
      break;
    AtLeastOneLoopTaken = true;
  }
  if (IsSeqCst) {
    SmallVector<llvm::Value *, 1> Args;
    Args.push_back(CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this));
    EmitRuntimeCall(OPENMPRTL_FUNC(flush), Args);
  }
  CGM.OpenMPSupport.endOpenMPRegion();
}

/// "Two-calls" OMP Directives (master, single, critical, ordered).
/// '#pragma omp master' directive.
void CodeGenFunction::EmitOMPMasterDirective(const OMPMasterDirective &S) {
  // if (__kmpc_master()) {
  //   <captured_body>
  //   __kmpc_end_master();
  // }
  EmitOMPConditionalIfHelper(S, OPENMPRTL_FUNC(master), S.getLocStart(),
                             OPENMPRTL_FUNC(end_master), S.getLocStart(),
                             false, // pragma has no clauses
                             0,     // has no need for "didit"
                             "omp.master");
}

/// '#pragma omp single' directive.
void CodeGenFunction::EmitOMPSingleDirective(const OMPSingleDirective &S) {

  // Init list of private globals in the stack.
  CGM.OpenMPSupport.startOpenMPRegion(false);
  CGM.OpenMPSupport.setNoWait(false);
  bool HasClauses = S.getNumClauses();
  if (HasClauses) {
    // Set NoWait flag if the clause nowait is there
    for (ArrayRef<OMPClause *>::iterator I = S.clauses().begin(),
                                         E = S.clauses().end();
         I != E; ++I) {
      if (*I)
        EmitInitOMPClause(*(*I), S);
    }
  }

  // did_it = 0;
  // if (__kmpc_single()) {
  //   <captured_body>
  //   did_it = 1;
  //   __kmpc_end_single();
  // }
  // ... if there is copyprivate clause, call to __kmpc_copyprivate()
  // ... if there is no nowait, call to __kmpc_barrier()
  //

  // Create a "did_it" temp for passing into copyprivate routine.
  llvm::AllocaInst *DidIt = CreateMemTemp(getContext().IntTy, ".did_it.");
  InitTempAlloca(DidIt, llvm::Constant::getNullValue(
                            ConvertTypeForMem(getContext().IntTy)));

  EmitOMPConditionalIfHelper(
      S, OPENMPRTL_FUNC(single), S.getLocStart(), OPENMPRTL_FUNC(end_single),
      S.getLocStart(), HasClauses, // pragma has clauses (private and
                                   // firstprivate will be processed)
      DidIt,                       // address to store 1 for "the single" thread
      "omp.single");

  // Copyprivate clause.
  // Restrictions to copyprivate (from standard):
  // The items that appear in copyprivate must be either threadprivate or
  // private in the enclosing context.
  // A list item that appears in copyprivate clause may not appear in a private
  // or firstprivate clause on the single construct.
  //
  bool HasCopyPrivate = false;
  for (ArrayRef<OMPClause *>::iterator ICL = S.clauses().begin(),
                                       ECL = S.clauses().end();
       ICL != ECL; ++ICL) {
    if (*ICL) {
      if (const OMPCopyPrivateClause *C =
              dyn_cast<OMPCopyPrivateClause>(*ICL)) {
        // Begin copyprivate clause processing
        HasCopyPrivate = true;
        // Start a copy-function.
        CodeGenFunction CGF(CGM, true);
        CGF.CurFn = 0;
        FunctionArgList Args;
        ImplicitParamDecl Arg1(0, SourceLocation(), 0, getContext().VoidPtrTy);
        ImplicitParamDecl Arg2(0, SourceLocation(), 0, getContext().VoidPtrTy);
        Args.push_back(&Arg1);
        Args.push_back(&Arg2);
        const CGFunctionInfo &FI = CGF.getTypes().arrangeFreeFunctionDeclaration(
            getContext().VoidTy, Args, FunctionType::ExtInfo(), false);
        llvm::FunctionType *FTy = CGF.getTypes().GetFunctionType(FI);
        llvm::Function *Fn = llvm::Function::Create(
            FTy, llvm::GlobalValue::InternalLinkage,
            StringRef(".omp_copy_func."), &CGM.getModule());
        CGM.SetInternalFunctionAttributes(CurFuncDecl, Fn, FI);
        CGF.StartFunction(GlobalDecl(), getContext().VoidTy, Fn, FI, Args,
                          SourceLocation());

        // Generate the record of pointers - cpy.var
        llvm::SmallVector<llvm::Type *, 16> CpyFieldTypes;
        for (OMPCopyPrivateClause::varlist_const_iterator
                 I = C->varlist_begin(),
                 E = C->varlist_end();
             I != E; ++I) {
          // const VarDecl *VD =
          // cast<VarDecl>(cast<DeclRefExpr>(*I)->getDecl());
          QualType QTy = (*I)->getType();
          llvm::Type *PtrType = ConvertType(getContext().getPointerType(QTy));
          CpyFieldTypes.push_back(PtrType);
        }
        llvm::StructType *CpyType =
            llvm::StructType::get(CGM.getLLVMContext(), CpyFieldTypes);
        llvm::AllocaInst *CpyVar = CreateTempAlloca(CpyType, "cpy.var");
        CpyVar->setAlignment(CGM.PointerAlignInBytes);

        // Generate initializaion of our local record with addresses.
        int32_t FieldNum = 0;
        for (OMPCopyPrivateClause::varlist_const_iterator
                 I = C->varlist_begin(),
                 E = C->varlist_end();
             I != E; ++I, ++FieldNum) {
          // Store the address into our record.
          Builder.CreateStore(
              EmitLValue(*I).getAddress(),
              Builder.CreateConstGEP2_32(CpyVar, 0, FieldNum));
        }

        // Generate field copying in the copy-function.
        {
          llvm::Function::arg_iterator ArgIt = CGF.CurFn->arg_begin();
          llvm::Value *DstPtr = ArgIt;
          llvm::Value *SrcPtr = ++ArgIt;
          llvm::Value *DstBase = CGF.Builder.CreatePointerCast(
              DstPtr, CpyType->getPointerTo(), "cpy.dst");
          llvm::Value *SrcBase = CGF.Builder.CreatePointerCast(
              SrcPtr, CpyType->getPointerTo(), "cpy.src");

          ArrayRef<const Expr *>::iterator AssignIter =
              C->getAssignments().begin();
          ArrayRef<const Expr *>::iterator VarIter1 =
              C->getPseudoVars1().begin();
          ArrayRef<const Expr *>::iterator VarIter2 =
              C->getPseudoVars2().begin();
          FieldNum = 0;
          for (OMPCopyPrivateClause::varlist_const_iterator
                   I = C->varlist_begin(),
                   E = C->varlist_end();
               I != E; ++I, ++AssignIter, ++VarIter1, ++VarIter2, ++FieldNum) {
            // const VarDecl *VD =
            // cast<VarDecl>(cast<DeclRefExpr>(*I)->getDecl());
            QualType QTy = (*I)->getType();
            llvm::Value *Dst =
                CGF.Builder.CreateConstGEP2_32(DstBase, 0, FieldNum);
            llvm::Value *Src =
                CGF.Builder.CreateConstGEP2_32(SrcBase, 0, FieldNum);
            llvm::Type *PtrType = ConvertType(getContext().getPointerType(QTy));
            llvm::Value *LoadDst = CGF.EmitLoadOfScalar(
                Dst, false, CGM.getDataLayout().getPrefTypeAlignment(PtrType),
                getContext().getPointerType(QTy), SourceLocation());
            llvm::Value *LoadSrc = CGF.EmitLoadOfScalar(
                Src, false, CGM.getDataLayout().getPrefTypeAlignment(PtrType),
                getContext().getPointerType(QTy), SourceLocation());
            CGF.EmitCopyAssignment(I, AssignIter, VarIter1, VarIter2, LoadDst,
                                   LoadSrc);
          }
        }

        // Generate a call to __kmpc_copyprivate.
        {
          // __kmpc_copyprivate(ident_t *loc, int32_t global_tid,
          //                    size_t cpy_size, void *cpy_data,
          //                    kmp_copy_func cpy_func, int32_t didit);
          llvm::Value *Loc =
              CGM.CreateIntelOpenMPRTLLoc(C->getLocStart(), *this);
          llvm::Value *GTid =
              CGM.CreateOpenMPGlobalThreadNum(C->getLocStart(), *this);
          int32_t CpySizeInt = CGM.getDataLayout().getTypeAllocSize(CpyType);
          llvm::Value *CpySize = llvm::ConstantInt::get(SizeTy, CpySizeInt);
          llvm::Value *LoadDidIt = EmitLoadOfScalar(
              DidIt, false, CGM.getDataLayout().getPrefTypeAlignment(
                                DidIt->getType()->getSequentialElementType()),
              getContext().IntTy, SourceLocation());
          llvm::Value *RealArgs[] = { Loc, GTid, CpySize,
                                      Builder.CreateBitCast(CpyVar, VoidPtrTy,
                                                            "(void*)cpyrec"),
                                      CGF.CurFn, LoadDidIt };
          EmitRuntimeCall(OPENMPRTL_FUNC(copyprivate), RealArgs);
        }

        // Stop the copy-function.
        CGF.FinishFunction();
        // End copyprivate clause processing
      }
    }
  }

  if (!HasCopyPrivate && !CGM.OpenMPSupport.getNoWait()) {
    // Note: __kmpc_copyprivate already has a couple of barriers internally.
    EmitOMPCallWithLocAndTidHelper(OPENMPRTL_FUNC(cancel_barrier), S.getLocEnd(),
                                   KMP_IDENT_BARRIER_IMPL_SINGLE);
  }

  // Remove list of private globals from the stack.
  CGM.OpenMPSupport.endOpenMPRegion();
}

/// '#pragma omp critical' directive.
void CodeGenFunction::EmitOMPCriticalDirective(const OMPCriticalDirective &S) {
  // __kmpc_critical();
  // <captured_body>
  // __kmpc_end_critical();
  //

  // Prepare kmp_critical_name -- the name of our critical section.
  std::string directive_name = S.getDirectiveName().getAsString();
  std::string name = ".gomp_critical_user_" + directive_name + ".var";
  llvm::Type *LckTy =
      llvm::TypeBuilder<kmp_critical_name, false>::get(CGM.getLLVMContext());
  llvm::GlobalVariable *Lck = cast<llvm::GlobalVariable>(
      CGM.CreateRuntimeVariable(LckTy, name.c_str()));
  Lck->setLinkage(llvm::GlobalValue::CommonLinkage);
  Lck->setInitializer(llvm::Constant::getNullValue(LckTy));

  // Prepare other arguments and build a call to __kmpc_critical
  llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(S.getLocStart(), *this);
  llvm::Value *GTid = CGM.CreateOpenMPGlobalThreadNum(S.getLocStart(), *this);
  llvm::Value *RealArgs[] = { Loc, GTid, Lck };
  EmitRuntimeCall(OPENMPRTL_FUNC(critical), RealArgs);
  EmitOMPCapturedBodyHelper(S);
  EmitRuntimeCall(OPENMPRTL_FUNC(end_critical), RealArgs);
}

/// '#pragma omp ordered' directive.
void CodeGenFunction::EmitOMPOrderedDirective(const OMPOrderedDirective &S) {
  // __kmpc_ordered();
  //   <captured_body>
  // __kmpc_enc_ordered();
  //
  EmitOMPCallWithLocAndTidHelper(OPENMPRTL_FUNC(ordered), S.getLocStart());
  EmitOMPCapturedBodyHelper(S);
  EmitOMPCallWithLocAndTidHelper(OPENMPRTL_FUNC(end_ordered), S.getLocStart());
}

/// '#pragma omp taskgroup' directive.
void
CodeGenFunction::EmitOMPTaskgroupDirective(const OMPTaskgroupDirective &S) {
  // __kmpc_taskgroup();
  //   <captured_body>
  // __kmpc_enc_taskgroup();
  //
  EmitOMPCallWithLocAndTidHelper(OPENMPRTL_FUNC(taskgroup), S.getLocStart());
  EmitOMPCapturedBodyHelper(S);
  EmitOMPCallWithLocAndTidHelper(OPENMPRTL_FUNC(end_taskgroup), S.getLocEnd());

  // EmitUntiedPartIdInc(*this);
  // EmitUntiedBranchEnd(*this);
  // EmitUntiedTaskSwitch(*this, false);
}

void
CodeGenFunction::EmitCloseOMPReductionClause(const OMPReductionClause &C,
                                             const OMPExecutableDirective &S) {
  assert(!isa<OMPSimdDirective>(S)); // Not yet supported
  llvm::BasicBlock *RedBB1;
  llvm::BasicBlock *RedBB2;
  llvm::Instruction *IP1;
  llvm::Instruction *IP2;
  CGM.OpenMPSupport.getReductionIPs(RedBB1, IP1, RedBB2, IP2);
  llvm::SwitchInst *Switch = dyn_cast_or_null<llvm::SwitchInst>(
      CGM.OpenMPSupport.getReductionSwitch());
  if (Switch && (IP1 || IP2 || RedBB1 || RedBB2)) {
    CGBuilderTy::InsertPoint SavedIP = Builder.saveIP();
    Builder.SetInsertPoint(RedBB1, IP1);
    // __kmpc_end_reduce[_nowait](ident_t *loc, int32_t global_tid, *lck);
    // ident_t loc = {...};
    llvm::Value *Loc = CGM.CreateIntelOpenMPRTLLoc(C.getLocStart(), *this);
    // global_tid = __kmpc_global_thread_num(...);
    llvm::Value *GTid = CGM.CreateOpenMPGlobalThreadNum(C.getLocStart(), *this);
    // kmp_critical_name lck;
    llvm::Value *RealArgs[] = { Loc, GTid,
                                CGM.OpenMPSupport.getReductionLockVar() };
    EmitRuntimeCall(CGM.OpenMPSupport.getNoWait()
                        ? OPENMPRTL_FUNC(end_reduce_nowait)
                        : OPENMPRTL_FUNC(end_reduce),
                    RealArgs);
    Builder.CreateBr(Switch->getDefaultDest());
    // Switch->addCase(llvm::ConstantInt::get(Int32Ty, 1), RedBB1);
    Builder.SetInsertPoint(RedBB2, IP2);
    Builder.CreateBr(Switch->getDefaultDest());
    // Switch->addCase(llvm::ConstantInt::get(Int32Ty, 2), RedBB2);
    Builder.restoreIP(SavedIP);
    CGM.OpenMPSupport.setReductionIPs(0, 0, 0, 0);
  }

  CodeGenFunction &CGF = CGM.OpenMPSupport.getCGFForReductionFunction();
  llvm::Value *Arg1;
  llvm::Value *Arg2;
  CGM.OpenMPSupport.getReductionFunctionArgs(Arg1, Arg2);
  ArrayRef<const Expr *>::iterator Par1I = C.getHelperParameters1st().begin();
  ArrayRef<const Expr *>::iterator Par2I = C.getHelperParameters2nd().begin();
  ArrayRef<const Expr *>::iterator OpI = C.getOpExprs().begin();
  for (OMPReductionClause::varlist_const_iterator I = C.varlist_begin(),
                                                  E = C.varlist_end();
       I != E; ++I, ++Par1I, ++Par2I, ++OpI) {
    // Get element type.
    const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(*I)->getDecl());
    if (VD->hasLocalStorage() &&
        (!CapturedStmtInfo || !CapturedStmtInfo->lookup(VD)))
      continue;
    const VarDecl *Par1 = cast<VarDecl>(cast<DeclRefExpr>(*Par1I)->getDecl());
    const VarDecl *Par2 = cast<VarDecl>(cast<DeclRefExpr>(*Par2I)->getDecl());
    llvm::Value *Addr1 = CGF.Builder.CreateConstGEP2_32(
        Arg1, 0, CGM.OpenMPSupport.getReductionVarIdx(VD),
        CGM.getMangledName(VD) + ".addr.lhs");
    llvm::Value *Addr2 = CGF.Builder.CreateConstGEP2_32(
        Arg2, 0, CGM.OpenMPSupport.getReductionVarIdx(VD),
        CGM.getMangledName(VD) + ".addr.rhs");
    CGM.OpenMPSupport.addOpenMPPrivateVar(Par1, Addr1);
    CGM.OpenMPSupport.addOpenMPPrivateVar(Par2, Addr2);
    CGF.EmitIgnoredExpr(*OpI);
    CGM.OpenMPSupport.delOpenMPPrivateVar(Par1);
    CGM.OpenMPSupport.delOpenMPPrivateVar(Par2);
  }
}

void
CodeGenFunction::EmitFinalOMPReductionClause(const OMPReductionClause &C,
                                             const OMPExecutableDirective &S) {
  assert(!isa<OMPSimdDirective>(S)); // Not yet supported
  CodeGenFunction &CGF = CGM.OpenMPSupport.getCGFForReductionFunction();
  if (CGF.CurFn) {
    CGF.FinishFunction();
    CGF.CurFn = 0;
  }
}

// Implementation of '#pragma omp simd'.
//

SourceLocation CodeGenFunction::CGPragmaOmpSimd::getForLoc() const {
  const CapturedStmt *Cap = cast<CapturedStmt>(SimdOmp->getAssociatedStmt());
  const ForStmt *For = dyn_cast<ForStmt>(Cap->getCapturedStmt());
  if (For) {
    return For->getSourceRange().getBegin();
  }
  return SimdOmp->getSourceRange().getBegin();
}

SourceRange CodeGenFunction::CGPragmaOmpSimd::getSourceRange() const {
  return SimdOmp->getSourceRange();
}

const Stmt *CodeGenFunction::CGPragmaOmpSimd::getInit() const {
  return getInitFromLoopDirective(SimdOmp);
}

const Expr *CodeGenFunction::CGPragmaOmpSimd::getCond() const {
  const CapturedStmt *Cap = dyn_cast_or_null<CapturedStmt>(getAssociatedStmt());
  if (!Cap)
    return 0;
  const ForStmt *For = dyn_cast_or_null<ForStmt>(Cap->getCapturedStmt());
  if (!For)
    return 0;
  return For->getCond();
}

const CapturedStmt *
CodeGenFunction::CGPragmaOmpSimd::getAssociatedStmt() const {
  return dyn_cast_or_null<CapturedStmt>(SimdOmp->getAssociatedStmt());
}

const Expr *CodeGenFunction::CGPragmaOmpSimd::getLoopCount() const {
  const Expr *Op = getNewIterEndFromLoopDirective(SimdOmp);
  if (const BinaryOperator *Bop = dyn_cast<BinaryOperator>(Op)) {
    // Expected "N-1" here, so why not eat "-1" to get "N".
    if (Bop->getOpcode() == BO_Sub) {
      const Expr *Op = Bop->getRHS();
      if (const ImplicitCastExpr *Cast = dyn_cast<ImplicitCastExpr>(Op)) {
        Op = Cast->getSubExpr();
      }
      if (const IntegerLiteral *One = dyn_cast<IntegerLiteral>(Op)) {
        if (One->getValue() == 1) {
          return Bop->getLHS();
        }
      }
    }
  }
  assert(0 && "Unexpected loop count expression");
  return Op;
}

Stmt *CodeGenFunction::CGPragmaOmpSimd::extractLoopBody(Stmt *S) const {
  // '#pragma omp simd' stores the full loop nest, and now we are
  // going to extract the loop body.
  unsigned CollapseNum = getCollapsedNumberFromLoopDirective(SimdOmp);
  if (CollapseNum == 0) {
    CollapseNum = 1;
  }
  Stmt *Body = S;
  while (CollapseNum > 0) {
    if (ForStmt *For = dyn_cast<ForStmt>(Body)) {
      Body = For->getBody();
      --CollapseNum;
    } else if (AttributedStmt *AS = dyn_cast<AttributedStmt>(Body)) {
      Body = AS->getSubStmt();
    } else if (CompoundStmt *CS = dyn_cast<CompoundStmt>(Body)) {
      if (CS->size() == 1) {
        Body = CS->body_back();
      } else {
        assert(0 && "Unexpected compound stmt in the loop nest");
      }
    } else {
      assert(0 && "Unexpected stmt in the loop nest");
    }
  }
  assert(Body && "Failed to extract the loop body for 'omp simd'");
  return Body;
}

// Simd wrappers implementation for '#pragma omp simd'.
bool CodeGenFunction::CGPragmaOmpSimd::emitSafelen(CodeGenFunction *CGF) const {
  bool SeparateLastIter = false;
  CGF->LoopStack.SetParallel();
  CGF->LoopStack.SetVectorizerEnable(true);
  for (ArrayRef<OMPClause *>::iterator I = SimdOmp->clauses().begin(),
                                       E = SimdOmp->clauses().end();
       I != E; ++I) {
    OMPClause *C = dyn_cast<OMPClause>(*I);
    switch (C->getClauseKind()) {
    case OMPC_safelen: {
      RValue Len = CGF->EmitAnyExpr(cast<OMPSafelenClause>(C)->getSafelen(),
                                    AggValueSlot::ignored(), true);
      llvm::ConstantInt *Val = dyn_cast<llvm::ConstantInt>(Len.getScalarVal());
      assert(Val);
      CGF->LoopStack.SetVectorizerWidth(Val->getZExtValue());
      // In presence of finite 'safelen', it may be unsafe to mark all
      // the memory instructions parallel, because loop-carried
      // dependences of 'safelen' iterations are possible.
      CGF->LoopStack.SetParallel(false);
      break;
    }
    case OMPC_lastprivate: {
      SeparateLastIter = true;
      break;
    }
    default:
      // Not handled yet
      ;
    }
  }
  return SeparateLastIter;
}

llvm::ConstantInt *
CodeGenFunction::CGPragmaOmpSimd::emitClauseTail(CodeGenFunction *CGF,
                                                 Expr *E) const {
  // Emit a constant integer for clause's tail expression.
  // E can be an integer or NULL.
  llvm::ConstantInt *Val = 0;
  if (E != 0) {
    RValue RVal = CGF->EmitAnyExpr(E, AggValueSlot::ignored(), true);
    Val = dyn_cast<llvm::ConstantInt>(RVal.getScalarVal());
  } else {
    Val = cast<llvm::ConstantInt>(
        llvm::ConstantInt::getNullValue(CGF->CGM.IntTy));
  }
  assert(Val);
  return Val;
}

// Walker for '#pragma omp simd'
bool CodeGenFunction::CGPragmaOmpSimd::walkLocalVariablesToEmit(
    CodeGenFunction *CGF, CGSIMDForStmtInfo *Info) const {

  // Init the OpenMP local vars stack.
  CGF->CGM.OpenMPSupport.startOpenMPRegion(true);
  CGF->CGM.OpenMPSupport.setMergeable(false);
  CGF->CGM.OpenMPSupport.setOrdered(false);

  // Make sure we have local vars for all the loop counters.
  ArrayRef<Expr *> Counters = getCountersFromLoopDirective(SimdOmp);
  for (unsigned I = 0; I < getCollapsedNumberFromLoopDirective(SimdOmp); ++I) {
    const VarDecl *VD =
        cast<VarDecl>(cast<DeclRefExpr>(Counters[I])->getDecl());
    if (CGF->CGM.OpenMPSupport.getTopOpenMPPrivateVar(VD))
      continue;
    QualType QTy = Counters[I]->getType();
    llvm::AllocaInst *Private =
        CGF->CreateMemTemp(QTy, CGF->CGM.getMangledName(VD) + ".counter.");
    CGF->CGM.OpenMPSupport.addOpenMPPrivateVar(VD, Private);
  }

  // Here we push index parameter into openmp map.
  // It is useful for loop counters calculation.
  const CapturedDecl *CD =
      cast<CapturedStmt>(getAssociatedStmt())->getCapturedDecl();
  llvm::Value *LoopIndex = CGF->LocalDeclMap.lookup(CD->getParam(1));
  const VarDecl *IndexVD = cast<VarDecl>(
      cast<DeclRefExpr>(getNewIterVarFromLoopDirective(SimdOmp))->getDecl());
  CGF->CGM.OpenMPSupport.addOpenMPPrivateVar(IndexVD, LoopIndex);

  for (ArrayRef<OMPClause *>::iterator I = SimdOmp->clauses().begin(),
                                       E = SimdOmp->clauses().end();
       I != E; ++I) {
    OMPClause *C = dyn_cast<OMPClause>(*I);
    switch (C->getClauseKind()) {
    case OMPC_private: {
      CGF->EmitPreOMPClause(*(*I), *SimdOmp);
      break;
    }
    case OMPC_lastprivate: {
      CGF->EmitPreOMPClause(*(*I), *SimdOmp);
      break;
    }
    case OMPC_linear: {
      // Linear vars are calculated from index, similar to loop indices.
      OMPLinearClause *L = cast<OMPLinearClause>(C);
      for (OMPLinearClause::varlist_const_iterator J = L->varlist_begin(),
                                                   F = L->varlist_end();
           J != F; ++J) {
        const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(*J)->getDecl());
        if (CGF->CGM.OpenMPSupport.getTopOpenMPPrivateVar(VD)) {
          continue;
        }
        QualType QTy = (*J)->getType();
        llvm::Value *Private =
            CGF->CreateMemTemp(QTy, CGF->CGM.getMangledName(VD) + ".linear.");

        // Generate "Private = Index * Step + Start"
        llvm::Value *Start = CGF->EmitAnyExprToTemp(*J).getScalarVal();
        llvm::Value *Index = CGF->Builder.CreateLoad(LoopIndex);
        llvm::Value *Result = 0;
        if (const Expr *StepExpr = L->getStep()) {
          Result = CGF->EmitAnyExpr(StepExpr).getScalarVal();
          QualType IndexTy = CD->getParam(1)->getType();
          Result = CGF->Builder.CreateIntCast(
              Result, Index->getType(),
              IndexTy->hasSignedIntegerRepresentation());
        } else
          Result = llvm::ConstantInt::get(Index->getType(), 1);
        Result = CGF->Builder.CreateMul(Index, Result);
        if (Start->getType()->isPointerTy()) {
          Result = CGF->Builder.CreateGEP(Start, Result);
        } else {
          Result = CGF->Builder.CreateIntCast(Result, Start->getType(), false);
          Result =
              CGF->Builder.CreateAdd(Start, Result, "add", false,
                                     QTy->isSignedIntegerOrEnumerationType());
        }
        CGF->Builder.CreateStore(Result, Private);

        CGF->CGM.OpenMPSupport.addOpenMPPrivateVar(VD, Private);
      }
      break;
    }
    default:
      break;
    }
  }

  // Mark 'aligned' variables -- do this after all private variables are
  // made 'omp-private' in CGM.OpenMPSupport.
  for (ArrayRef<OMPClause *>::iterator I = SimdOmp->clauses().begin(),
                                       E = SimdOmp->clauses().end();
       I != E; ++I) {
    OMPClause *C = dyn_cast<OMPClause>(*I);
    switch (C->getClauseKind()) {
    case OMPC_aligned: {
      OMPAlignedClause *A = cast<OMPAlignedClause>(C);
      // Prepare alignment expression for using it below.
      llvm::ConstantInt *AVal = emitClauseTail(CGF, A->getAlignment());
      // Walk the list and push each var's alignment into metadata.
      for (OMPAlignedClause::varlist_iterator J = A->varlist_begin(),
                                              F = A->varlist_end();
           J != F; ++J) {
        LValue LVal = CGF->EmitLValue(*J);
        CGF->LoopStack.AddAligned(LVal.getAddress(),
                                  (int)(AVal->getZExtValue()));
      }
      break;
    }
    default:
      break;
    }
  }

  // Emit initializations of loop indices.
  CGF->EmitStmt(getInitFromLoopDirective(SimdOmp));
  return false;
}

void CodeGenFunction::CGPragmaOmpSimd::emitInit(CodeGenFunction &CGF,
                                                llvm::Value *&LoopIndex,
                                                llvm::Value *&LoopCount) {
  // Emit loop index
  const Expr *IterVar = getNewIterVarFromLoopDirective(SimdOmp);
  LoopIndex = CGF.CreateMemTemp(IterVar->getType(), ".idx.");
  const VarDecl *VD = cast<VarDecl>(cast<DeclRefExpr>(IterVar)->getDecl());
  CGF.CGM.OpenMPSupport.addOpenMPPrivateVar(VD, LoopIndex);

  // Emit loop count.
  LoopCount = CGF.EmitAnyExpr(getLoopCount()).getScalarVal();
}

// Emit the final values of the loop counters and linear vars.
void
CodeGenFunction::CGPragmaOmpSimd::emitLinearFinal(CodeGenFunction &CGF) const {

  // Check if we need to update the loop counters.
  bool NeedUpdateLC = true;
  ArrayRef<Expr *> Counters = getCountersFromLoopDirective(SimdOmp);
  for (unsigned I = 0; I < getCollapsedNumberFromLoopDirective(SimdOmp); ++I) {
    const DeclRefExpr *DRE = cast<DeclRefExpr>(Counters[I]);
    if (!CGF.LocalDeclMap.lookup(DRE->getDecl())) {
      NeedUpdateLC = false;
    }
  }

  // Emit final values of the loop-counters.
  if (NeedUpdateLC)
    CGF.EmitStmt(getFinalFromLoopDirective(SimdOmp));

  // Emit final values of the linear vars.
  for (ArrayRef<OMPClause *>::iterator I = SimdOmp->clauses().begin(),
                                       E = SimdOmp->clauses().end();
       I != E; ++I) {
    OMPClause *C = dyn_cast<OMPClause>(*I);
    switch (C->getClauseKind()) {
    case OMPC_linear: {
      OMPLinearClause *L = cast<OMPLinearClause>(C);
      for (OMPLinearClause::varlist_const_iterator J = L->varlist_begin(),
                                                   F = L->varlist_end();
           J != F; ++J) {

        // Generate "L = LoopCount * Step + L"
        const Expr *CountExpr = getLoopCount();
        llvm::Value *Index = CGF.EmitAnyExpr(CountExpr).getScalarVal();
        llvm::Value *Result = 0;
        if (const Expr *StepExpr = L->getStep()) {
          Result = CGF.EmitAnyExpr(StepExpr).getScalarVal();
          QualType IndexTy = CountExpr->getType();
          Result = CGF.Builder.CreateIntCast(
              Result, Index->getType(),
              IndexTy->hasSignedIntegerRepresentation());
        } else
          Result = llvm::ConstantInt::get(Index->getType(), 1);
        Result = CGF.Builder.CreateMul(Index, Result);

        // Prepare destination lvalue to store result into.
        LValue LV = CGF.EmitLValue(*J);
        llvm::Value *Start =
            CGF.EmitLoadOfLValue(LV, (*J)->getExprLoc()).getScalarVal();

        if (Start->getType()->isPointerTy()) {
          Result = CGF.Builder.CreateGEP(Start, Result);
        } else {
          Result = CGF.Builder.CreateIntCast(Result, Start->getType(), false);
          Result =
              CGF.Builder.CreateAdd(Start, Result, "add", false,
                  (*J)->getType()->isSignedIntegerOrEnumerationType());
        }
        CGF.EmitStoreOfScalar(Result, LV, false);
      }
      break;
    }
    default:
      break;
    }
  }
}

// Generate the instructions for '#pragma omp simd' directive.
void CodeGenFunction::EmitOMPSimdDirective(const OMPSimdDirective &S) {
  CGPragmaOmpSimd Wrapper(&S);
  EmitPragmaSimd(Wrapper);
}

// Generate the instructions for '#pragma omp for simd' directive.
void CodeGenFunction::EmitOMPForSimdDirective(const OMPForSimdDirective &S) {
  EmitOMPDirectiveWithLoop(OMPD_for_simd, OMPD_for_simd, S);
}
