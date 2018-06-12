; ModuleID = 'C:\Users\Meinersbur\src\llvm\tools\clang\test\TransformPragma\pragma-id-reverse.cpp'
source_filename = "C:\5CUsers\5CMeinersbur\5Csrc\5Cllvm\5Ctools\5Cclang\5Ctest\5CTransformPragma\5Cpragma-id-reverse.cpp"
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.0.24215"

; Function Attrs: noinline nounwind optnone
define dso_local void @pragma_id_reverse(double* %A, i32 %N) #0 !looptransform !2 {
entry:
  %N.addr = alloca i32, align 4
  %A.addr = alloca double*, align 8
  %i = alloca i32, align 4
  store i32 %N, i32* %N.addr, align 4
  store double* %A, double** %A.addr, align 8
  %0 = load i32, i32* %N.addr, align 4
  %sub = sub nsw i32 %0, 1
  store i32 %sub, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32* %i, align 4
  %cmp = icmp sge i32 %1, 0
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load double*, double** %A.addr, align 8
  %3 = load i32, i32* %i, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds double, double* %2, i64 %idxprom
  %4 = load double, double* %arrayidx, align 8
  %add = fadd double %4, 1.000000e+00
  %5 = load double*, double** %A.addr, align 8
  %6 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %6 to i64
  %arrayidx2 = getelementptr inbounds double, double* %5, i64 %idxprom1
  store double %add, double* %arrayidx2, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %7 = load i32, i32* %i, align 4
  %dec = add nsw i32 %7, -1
  store i32 %dec, i32* %i, align 4
  br label %for.cond, !llvm.loop !4

for.end:                                          ; preds = %for.cond
  ret void
}

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{!"clang version 7.0.0 (trunk 331768)"}
!2 = !{!3}
!3 = !{!"llvm.loop.reverse", !"myloop"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.id", !"myloop"}
