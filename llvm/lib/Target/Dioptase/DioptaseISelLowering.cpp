//===-- DioptaseISelLowering.cpp - Dioptase DAG Lowering Implementation
//---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the interfaces that Dioptase uses to lower LLVM code
// into a selection DAG.
//
//===----------------------------------------------------------------------===//

#include "DioptaseISelLowering.h"
#include "DioptaseMachineFunctionInfo.h"
#include "DioptaseRegisterInfo.h"
#include "DioptaseSelectionDAGInfo.h"
#include "DioptaseTargetMachine.h"
#include "MCTargetDesc/DioptaseMCTargetDesc.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>
using namespace llvm;

//===----------------------------------------------------------------------===//
// Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "DioptaseGenCallingConv.inc"

static CCAssignFn *getReturnCC(CallingConv::ID CallConv) {
  switch (CallConv) {
  default:
    return RetCC_Dioptase;
  }
}

static CCAssignFn *getParamCC(CallingConv::ID CallConv, bool IsVarArg) {
  switch (CallConv) {
  default:
    return CC_Dioptase;
  }
}

DioptaseTargetLowering::DioptaseTargetLowering(const TargetMachine &TM,
                                               const DioptaseSubtarget &STI)
    : TargetLowering(TM, STI), Subtarget(&STI) {
  // Set up the register classes.DioptaseTargetLowering
  addRegisterClass(MVT::i32, &Dioptase::IntRegsRegClass);
  computeRegisterProperties(Subtarget->getRegisterInfo());
}

Register
DioptaseTargetLowering::getRegisterByName(const char *RegName, LLT VT,
                                          const MachineFunction &MF) const {
  assert(false && "not implemented");
}

bool DioptaseTargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &MF, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context,
    const Type *RetTy) const {
  CCAssignFn *RetCC = getReturnCC(CallConv);
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, Context);
  return CCInfo.CheckReturn(Outs, RetCC);
}

EVT DioptaseTargetLowering::getSetCCResultType(const DataLayout &DL,
                                               LLVMContext &Context,
                                               EVT VT) const {
  assert(false && "not implemented");
}

SDValue DioptaseTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  EVT PtrVT = getPointerTy(DAG.getDataLayout());

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_Dioptase);

  const unsigned StackOffset = 92;
  bool IsLittleEndian = DAG.getDataLayout().isLittleEndian();

  unsigned InIdx = 0;

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i, ++InIdx) {
    CCValAssign &VA = ArgLocs[i];
    EVT LocVT = VA.getLocVT();

    if (Ins[InIdx].Flags.isSRet()) {
      assert(false && "todo");
    }

    SDValue Arg;
    if (VA.isRegLoc()) {
      if (VA.needsCustom()) {
        assert(false && "todo");
      }

      Register VReg = RegInfo.createVirtualRegister(&Dioptase::IntRegsRegClass);
      MF.getRegInfo().addLiveIn(VA.getLocReg(), VReg);
      Arg = DAG.getCopyFromReg(Chain, dl, VReg, MVT::i32);
      if (VA.getLocInfo() != CCValAssign::Indirect) {
        if (VA.getLocVT() == MVT::f32)
          Arg = DAG.getNode(ISD::BITCAST, dl, MVT::f32, Arg);
        else if (VA.getLocVT() != MVT::i32) {
          Arg = DAG.getNode(ISD::AssertSext, dl, MVT::i32, Arg,
                            DAG.getValueType(VA.getLocVT()));
          Arg = DAG.getNode(ISD::TRUNCATE, dl, VA.getLocVT(), Arg);
        }
        InVals.push_back(Arg);
        continue;
      }
    } else {
      assert(VA.isMemLoc());

      unsigned Offset = VA.getLocMemOffset() + StackOffset;

      if (VA.needsCustom()) {
        assert(VA.getValVT() == MVT::f64 || VA.getValVT() == MVT::v2i32);
        // If it is double-word aligned, just load.
        if (Offset % 8 == 0) {
          int FI = MF.getFrameInfo().CreateFixedObject(8, Offset, true);
          SDValue FIPtr = DAG.getFrameIndex(FI, PtrVT);
          SDValue Load = DAG.getLoad(VA.getValVT(), dl, Chain, FIPtr,
                                     MachinePointerInfo());
          InVals.push_back(Load);
          continue;
        }

        int FI = MF.getFrameInfo().CreateFixedObject(4, Offset, true);
        SDValue FIPtr = DAG.getFrameIndex(FI, PtrVT);
        SDValue HiVal =
            DAG.getLoad(MVT::i32, dl, Chain, FIPtr, MachinePointerInfo());
        int FI2 = MF.getFrameInfo().CreateFixedObject(4, Offset + 4, true);
        SDValue FIPtr2 = DAG.getFrameIndex(FI2, PtrVT);

        SDValue LoVal =
            DAG.getLoad(MVT::i32, dl, Chain, FIPtr2, MachinePointerInfo());

        if (IsLittleEndian)
          std::swap(LoVal, HiVal);

        SDValue WholeValue =
            DAG.getNode(ISD::BUILD_PAIR, dl, MVT::i64, LoVal, HiVal);
        WholeValue = DAG.getNode(ISD::BITCAST, dl, VA.getValVT(), WholeValue);
        InVals.push_back(WholeValue);
        continue;
      }

      int FI = MF.getFrameInfo().CreateFixedObject(LocVT.getSizeInBits() / 8,
                                                   Offset, true);
      SDValue FIPtr = DAG.getFrameIndex(FI, PtrVT);
      SDValue Load = DAG.getLoad(LocVT, dl, Chain, FIPtr,
                                 MachinePointerInfo::getFixedStack(MF, FI));
      if (VA.getLocInfo() != CCValAssign::Indirect) {
        InVals.push_back(Load);
        continue;
      }
      Arg = Load;
    }

    assert(VA.getLocInfo() == CCValAssign::Indirect);

    SDValue ArgValue =
        DAG.getLoad(VA.getValVT(), dl, Chain, Arg, MachinePointerInfo());
    InVals.push_back(ArgValue);

    unsigned ArgIndex = Ins[InIdx].OrigArgIndex;
    assert(Ins[InIdx].PartOffset == 0);
    while (i + 1 != e && Ins[InIdx + 1].OrigArgIndex == ArgIndex) {
      CCValAssign &PartVA = ArgLocs[i + 1];
      unsigned PartOffset = Ins[InIdx + 1].PartOffset;
      SDValue Address = DAG.getMemBasePlusOffset(
          ArgValue, TypeSize::getFixed(PartOffset), dl);
      InVals.push_back(DAG.getLoad(PartVA.getValVT(), dl, Chain, Address,
                                   MachinePointerInfo()));
      ++i;
      ++InIdx;
    }
  }

  if (MF.getFunction().hasStructRetAttr()) {
    assert(false && "todo");
  }

  // Store remaining ArgRegs to the stack if this is a varargs function.
  if (isVarArg) {
    assert(false && "todo");
  }

  return Chain;
}

SDValue
DioptaseTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                  SmallVectorImpl<SDValue> &InVals) const {
  assert(false && "not implemented");
}

SDValue
DioptaseTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                    bool IsVarArg,
                                    const SmallVectorImpl<ISD::OutputArg> &Outs,
                                    const SmallVectorImpl<SDValue> &OutVals,
                                    const SDLoc &DL, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC_Dioptase);
  SDValue Glue;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  for (unsigned I = 0, RealRvLocIdx = 0; I != RVLocs.size();
       ++I, ++RealRvLocIdx) {
    CCValAssign &VA = RVLocs[I];
    assert(VA.isRegLoc() && "Can only return in registers!");

    SDValue Arg = OutVals[RealRvLocIdx];

    if (VA.needsCustom()) {
      assert(false && "todo");
    } else {
      Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), Arg, Glue);
    }

    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  if (MF.getFunction().hasStructRetAttr()) {

      assert(false && "todo");
  }

  RetOps[0] = Chain;

  if (Glue.getNode())
    RetOps.push_back(Glue);

  return DAG.getNode(DioptaseISD::RET_GLUE, DL, MVT::Other, RetOps);
}
