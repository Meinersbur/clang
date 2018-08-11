; ModuleID = 'C:\Users\meinersbur\src\llvm\tools\clang\test\TransformPragma\pragma-id-reverse-illegal.cpp'
source_filename = "C:\5CUsers\5Cmeinersbur\5Csrc\5Cllvm\5Ctools\5Cclang\5Ctest\5CTransformPragma\5Cpragma-id-reverse-illegal.cpp"
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.0.24215"

; Function Attrs: norecurse nounwind
define dso_local void @pragma_id_reverse(double* nocapture %A, i32 %N) local_unnamed_addr #0 !looptransform !2 {
entry:
  %0 = sext i32 %N to i64
  %1 = icmp sgt i32 %N, 1
  br i1 %1, label %polly.loop_preheader, label %polly.exiting

polly.exiting:                                    ; preds = %polly.stmt.for.body.cont, %entry
  ret void

polly.loop_header:                                ; preds = %polly.stmt.for.body.cont, %polly.loop_preheader
  %polly.indvar = phi i64 [ 0, %polly.loop_preheader ], [ %2, %polly.stmt.for.body.cont ]
  %2 = add nuw nsw i64 %polly.indvar, 1
  %polly.access.A11 = getelementptr double, double* %A, i64 %2
  %polly.access.A11.reload = load double, double* %polly.access.A11, align 8, !alias.scope !4, !noalias !6
  %p_add = fadd double %polly.access.A11.reload, 1.000000e+00
  %scevgep12 = getelementptr double, double* %polly.access.A, i64 %polly.indvar
  store double %p_add, double* %scevgep12, align 8, !alias.scope !4, !noalias !6
  %3 = add nuw nsw i64 %polly.indvar, 3
  %4 = icmp sgt i64 %3, %0
  br i1 %4, label %polly.stmt.for.body.cont, label %polly.stmt.for.body.Stmt2_Write0.partial

polly.stmt.for.body.Stmt2_Write0.partial:         ; preds = %polly.loop_header
  %5 = add nuw nsw i64 %polly.indvar, 2
  %polly.access.A13 = getelementptr double, double* %A, i64 %5
  store double %p_add, double* %polly.access.A13, align 8, !alias.scope !4, !noalias !6
  br label %polly.stmt.for.body.cont

polly.stmt.for.body.cont:                         ; preds = %polly.loop_header, %polly.stmt.for.body.Stmt2_Write0.partial
  %exitcond = icmp eq i64 %2, %8
  br i1 %exitcond, label %polly.exiting, label %polly.loop_header

polly.loop_preheader:                             ; preds = %entry
  %6 = bitcast double* %A to i64*
  %.pre_p_scalar_14 = load i64, i64* %6, align 8, !alias.scope !4, !noalias !6
  %polly.access.A = getelementptr double, double* %A, i64 1
  %7 = bitcast double* %polly.access.A to i64*
  store i64 %.pre_p_scalar_14, i64* %7, align 8, !alias.scope !4, !noalias !6
  %8 = add nsw i64 %0, -1
  br label %polly.loop_header
}

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "polly-optimized" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{!"clang version 8.0.0 (trunk 334718) (llvm/trunk 338031)"}
!2 = !{!3}
!3 = !{!"llvm.loop.reverse", !"myloop"}
!4 = distinct !{!4, !5, !"polly.alias.scope.MemRef0"}
!5 = distinct !{!5, !"polly.alias.scope.domain"}
!6 = !{}
