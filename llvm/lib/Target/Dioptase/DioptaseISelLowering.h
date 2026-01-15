//===-- DioptaseISelLowering.h - Dioptase DAG Lowering Interface ------------*-
// C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Dioptase uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIOPTASE_DIOPTASEISELLOWERING_H
#define LLVM_LIB_TARGET_DIOPTASE_DIOPTASEISELLOWERING_H

#include "Dioptase.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
class DioptaseSubtarget;
class DioptaseCustomDAG;

class DioptaseTargetLowering : public TargetLowering {
  const DioptaseSubtarget *Subtarget;

  void initRegisterClasses();
  void initSPUActions();
  void initVPUActions();

public:
  DioptaseTargetLowering(const TargetMachine &TM, const DioptaseSubtarget &STI);

  MVT getScalarShiftAmountTy(const DataLayout &, EVT) const override {
    return MVT::i32;
  }

  Register getRegisterByName(const char *RegName, LLT VT,
                             const MachineFunction &MF) const override;

  EVT getSetCCResultType(const DataLayout &DL, LLVMContext &Context,
                         EVT VT) const override;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                      bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &ArgsFlags,
                      LLVMContext &Context, const Type *RetTy) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &dl,
                      SelectionDAG &DAG) const override;
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_Dioptase_DioptaseISELLOWERING_H
