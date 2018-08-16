// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -ast-print %s | FileCheck --check-prefix=PRINT --match-full-lines %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -disable-llvm-passes -o - %s | FileCheck --check-prefix=IR %s
// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.0.24215 -std=c99 -emit-llvm -O3 -mllvm -polly -mllvm -polly-process-unprofitable -mllvm -polly-use-llvm-names -o - %s | FileCheck --check-prefix=TRANS %s

void pragma_pack(int n, int m, double C[m][n], double A[m][n]) {
  for (int i = 0; i < m; i += 1)
    #pragma clang loop pack array(A)
    for (int j = 0; j < n; j += 1)
      C[i][j] = A[j][i] + i + j;
}

// PRINT-LABEL: void pragma_pack(int n, int m, double C[m][n], double A[m][n]) {
// PRINT-NEXT:      for (int i = 0; i < m; i += 1)
// PRINT-NEXT:  #pragma clang loop pack array(A)
// PRINT-NEXT:          for (int j = 0; j < n; j += 1)
// PRINT-NEXT:              C[i][j] = A[j][i] + i + j;
// PRINT-NEXT:  }


// IR-LABEL: define dso_local void @pragma_pack(i32 %n, i32 %m, double* %C, double* %A) #0 !looptransform !2 {
//
// IR:         %A.addr = alloca double*, align 8
//
// IR:       !2 = !{!3}
// IR:       !3 = !{!"llvm.data.pack", !4, double** %A.addr}
// IR:       !4 = distinct !{!4}


// TRANS: define dso_local void @pragma_id_interchange(i32 %n, i32 %m, double* nocapture %C, double* nocapture readonly %A) local_unnamed_addr #0 !looptransform !2 {
// TRANS: entry:
// TRANS:   %0 = icmp sge i32 %n, %m
// TRANS:   %1 = sext i32 %n to i64
// TRANS:   %2 = icmp sgt i32 %n, -1
// TRANS:   %3 = and i1 %2, %0
// TRANS:   %4 = add nsw i64 %1, -1
// TRANS:   %5 = zext i32 %n to i64
// TRANS:   %polly.access.mul.A = tail call { i64, i1 } @llvm.smul.with.overflow.i64(i64 %4, i64 %5)
// TRANS:   %polly.access.mul.A.obit = extractvalue { i64, i1 } %polly.access.mul.A, 1
// TRANS:   %polly.access.mul.A.res = extractvalue { i64, i1 } %polly.access.mul.A, 0
// TRANS:   %6 = sext i32 %m to i64
// TRANS:   %polly.access.add.A = tail call { i64, i1 } @llvm.sadd.with.overflow.i64(i64 %polly.access.mul.A.res, i64 %6)
// TRANS:   %polly.access.add.A.obit = extractvalue { i64, i1 } %polly.access.add.A, 1
// TRANS:   %polly.access.add.A.res = extractvalue { i64, i1 } %polly.access.add.A, 0
// TRANS:   %polly.access.A = getelementptr double, double* %A, i64 %polly.access.add.A.res
// TRANS:   %7 = icmp ule double* %polly.access.A, %C
// TRANS:   %8 = add nsw i64 %6, -1
// TRANS:   %polly.overflow.state46 = or i1 %polly.access.mul.A.obit, %polly.access.add.A.obit
// TRANS:   %polly.access.mul.C48 = tail call { i64, i1 } @llvm.smul.with.overflow.i64(i64 %8, i64 %5)
// TRANS:   %polly.access.mul.C.obit49 = extractvalue { i64, i1 } %polly.access.mul.C48, 1
// TRANS:   %polly.overflow.state50 = or i1 %polly.access.mul.C.obit49, %polly.overflow.state46
// TRANS:   %polly.access.mul.C.res51 = extractvalue { i64, i1 } %polly.access.mul.C48, 0
// TRANS:   %polly.access.add.C52 = tail call { i64, i1 } @llvm.sadd.with.overflow.i64(i64 %polly.access.mul.C.res51, i64 %1)
// TRANS:   %polly.access.add.C.obit53 = extractvalue { i64, i1 } %polly.access.add.C52, 1
// TRANS:   %polly.overflow.state54 = or i1 %polly.access.add.C.obit53, %polly.overflow.state50
// TRANS:   %polly.access.add.C.res55 = extractvalue { i64, i1 } %polly.access.add.C52, 0
// TRANS:   %polly.access.C56 = getelementptr double, double* %C, i64 %polly.access.add.C.res55
// TRANS:   %9 = icmp ule double* %polly.access.C56, %A
// TRANS:   %10 = or i1 %7, %9
// TRANS:   %11 = and i1 %3, %10
// TRANS:   %polly.rtc.overflown = xor i1 %polly.overflow.state54, true
// TRANS:   %polly.rtc.result = and i1 %11, %polly.rtc.overflown
// TRANS:   br i1 %polly.rtc.result, label %polly.start, label %entry.split
// 
// TRANS: entry.split:                                     
// TRANS:   %cmp33 = icmp sgt i32 %m, 0
// TRANS:   %cmp231 = icmp sgt i32 %n, 0
// TRANS:   %or.cond = and i1 %cmp231, %cmp33
// TRANS:   br i1 %or.cond, label %for.cond1.preheader.us.preheader, label %for.cond.cleanup
// 
// TRANS: for.cond1.preheader.us.preheader:                
// TRANS:   %wide.trip.count38 = zext i32 %m to i64
// TRANS:   br label %for.cond1.preheader.us
// 
// TRANS: for.cond1.preheader.us:                          
// TRANS:   %indvars.iv36 = phi i64 [ 0, %for.cond1.preheader.us.preheader ], [ %indvars.iv.next37, %for.cond1.for.cond.cleanup3_crit_edge.us ]
// TRANS:   %12 = trunc i64 %indvars.iv36 to i32
// TRANS:   %conv.us = sitofp i32 %12 to double
// TRANS:   %13 = mul nuw nsw i64 %indvars.iv36, %5
// TRANS:   %arrayidx10.us = getelementptr inbounds double, double* %C, i64 %13
// TRANS:   %arrayidx.us = getelementptr inbounds double, double* %A, i64 %indvars.iv36
// TRANS:   br label %for.body4.us
//
// TRANS: for.body4.us:                                    
// TRANS:   %indvars.iv = phi i64 [ 0, %for.cond1.preheader.us ], [ %indvars.iv.next, %for.body4.us ]
// TRANS:   %14 = mul nuw nsw i64 %indvars.iv, %5
// TRANS:   %arrayidx6.us = getelementptr inbounds double, double* %arrayidx.us, i64 %14
// TRANS:   %15 = load double, double* %arrayidx6.us, align 8, !tbaa !6
// TRANS:   %add.us = fadd double %15, %conv.us
// TRANS:   %16 = trunc i64 %indvars.iv to i32
// TRANS:   %conv7.us = sitofp i32 %16 to double
// TRANS:   %add8.us = fadd double %add.us, %conv7.us
// TRANS:   %arrayidx12.us = getelementptr inbounds double, double* %arrayidx10.us, i64 %indvars.iv
// TRANS:   store double %add8.us, double* %arrayidx12.us, align 8, !tbaa !6
// TRANS:   %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
// TRANS:   %exitcond = icmp eq i64 %indvars.iv.next, %5
// TRANS:   br i1 %exitcond, label %for.cond1.for.cond.cleanup3_crit_edge.us, label %for.body4.us, !llvm.loop !10
//
// TRANS: for.cond1.for.cond.cleanup3_crit_edge.us:        
// TRANS:   %indvars.iv.next37 = add nuw nsw i64 %indvars.iv36, 1
// TRANS:   %exitcond39 = icmp eq i64 %indvars.iv.next37, %wide.trip.count38
// TRANS:   br i1 %exitcond39, label %for.cond.cleanup, label %for.cond1.preheader.us, !llvm.loop !12
// 
// TRANS: for.cond.cleanup:                
// TRANS:   ret void
// 
// TRANS: polly.start:                                      
// TRANS:   %polly.loop_guard = icmp sgt i32 %n, 0
// TRANS:   %polly.loop_guard70 = icmp sgt i32 %m, 0
// TRANS:   %or.cond84 = and i1 %polly.loop_guard, %polly.loop_guard70
// TRANS:   br i1 %or.cond84, label %polly.loop_header.us, label %for.cond.cleanup
// 
// TRANS: polly.loop_header.us:                             ; preds = %polly.start, %polly.loop_exit69.us
// TRANS:   %polly.indvar.us = phi i64 [ %polly.indvar_next.us, %polly.loop_exit69.us ], [ 0, %polly.start ]
// TRANS:   %17 = mul nuw nsw i64 %polly.indvar.us, %5
// TRANS:   %scevgep.us = getelementptr double, double* %A, i64 %17
// TRANS:   %18 = trunc i64 %polly.indvar.us to i32
// TRANS:   %scevgep75.us = getelementptr double, double* %C, i64 %polly.indvar.us
// TRANS:   %p_conv7.us.us = sitofp i32 %18 to double
// TRANS:   br label %polly.loop_header67.us
// 
// TRANS: polly.loop_header67.us:                           
// TRANS:   %polly.indvar71.us = phi i64 [ 0, %polly.loop_header.us ], [ %polly.indvar_next72.us, %polly.loop_header67.us ]
// TRANS:   %19 = trunc i64 %polly.indvar71.us to i32
// TRANS:   %p_conv.us.us = sitofp i32 %19 to double
// TRANS:   %scevgep74.us = getelementptr double, double* %scevgep.us, i64 %polly.indvar71.us
// TRANS:   %_p_scalar_.us = load double, double* %scevgep74.us, align 8, !alias.scope !14, !noalias !16
// TRANS:   %p_add.us.us = fadd double %_p_scalar_.us, %p_conv.us.us
// TRANS:   %p_add8.us.us = fadd double %p_add.us.us, %p_conv7.us.us
// TRANS:   %20 = mul nuw nsw i64 %polly.indvar71.us, %5
// TRANS:   %scevgep76.us = getelementptr double, double* %scevgep75.us, i64 %20
// TRANS:   store double %p_add8.us.us, double* %scevgep76.us, align 8, !alias.scope !17, !noalias !18
// TRANS:   %polly.indvar_next72.us = add nuw nsw i64 %polly.indvar71.us, 1
// TRANS:   %exitcond81 = icmp eq i64 %polly.indvar_next72.us, %6
// TRANS:   br i1 %exitcond81, label %polly.loop_exit69.us, label %polly.loop_header67.us
// 
// TRANS: polly.loop_exit69.us:                            
// TRANS:   %polly.indvar_next.us = add nuw nsw i64 %polly.indvar.us, 1
// TRANS:   %exitcond82 = icmp eq i64 %polly.indvar_next.us, %1
// TRANS:   br i1 %exitcond82, label %for.cond.cleanup, label %polly.loop_header.us
// TRANS: }
