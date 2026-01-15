//===-- DioptaseRegisterInfo.cpp - Dioptase Register Information
//----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Dioptase implementation of the TargetRegisterInfo
// class.
//
//===----------------------------------------------------------------------===//

#include "DioptaseRegisterInfo.h"
#include "Dioptase.h"
#include "DioptaseSubtarget.h"
#include "MCTargetDesc/DioptaseMCTargetDesc.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include <cassert>

using namespace llvm;

#define DEBUG_TYPE "ve-register-info"

#define GET_REGINFO_TARGET_DESC
#include "DioptaseGenRegisterInfo.inc"

DioptaseRegisterInfo::DioptaseRegisterInfo()
    : DioptaseGenRegisterInfo(Dioptase::R29) {}

const MCPhysReg *
DioptaseRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  switch (MF->getFunction().getCallingConv()) {
  default:
    return CSR_SaveList;
  case CallingConv::PreserveAll:
    assert(false && "unreachable");
  }
}

const uint32_t *
DioptaseRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                           CallingConv::ID CC) const {
  switch (CC) {
  case CallingConv::Fast:
    // Being explicit (same as standard CC).
  default:
    return CSR_RegMask;
  case CallingConv::PreserveAll:
    assert(false && "unreachable");
  }
}

BitVector
DioptaseRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  const Register ReservedRegs[] = {
      Dioptase::R28,
      Dioptase::R29,
      Dioptase::R30,
      Dioptase::R31,
  };

  for (auto R : ReservedRegs)
    for (MCRegAliasIterator ItAlias(R, this, true); ItAlias.isValid();
         ++ItAlias)
      Reserved.set(*ItAlias);

  // Reserve constant registers.
  Reserved.set(Dioptase::R0);

  return Reserved;
}

const TargetRegisterClass *
DioptaseRegisterInfo::getPointerRegClass(unsigned Kind) const {
  return &Dioptase::IntRegsRegClass;
}

/*
namespace {
class EliminateFrameIndex {
  const TargetInstrInfo &TII;
  const TargetRegisterInfo &TRI;
  const DebugLoc &DL;
  MachineBasicBlock &MBB;
  MachineBasicBlock::iterator II;
  Register clobber;

  // Some helper functions for the ease of instruction building.
  MachineFunction &getFunc() const { return *MBB.getParent(); }
  inline MCRegister getSubReg(MCRegister Reg, unsigned Idx) const {
    return TRI.getSubReg(Reg, Idx);
  }
  inline const MCInstrDesc &get(unsigned Opcode) const {
    return TII.get(Opcode);
  }
  inline MachineInstrBuilder build(const MCInstrDesc &MCID, Register DestReg) {
    return BuildMI(MBB, II, DL, MCID, DestReg);
  }
  inline MachineInstrBuilder build(unsigned InstOpc, Register DestReg) {
    return build(get(InstOpc), DestReg);
  }
  inline MachineInstrBuilder build(const MCInstrDesc &MCID) {
    return BuildMI(MBB, II, DL, MCID);
  }
  inline MachineInstrBuilder build(unsigned InstOpc) {
    return build(get(InstOpc));
  }

  // Calculate an address of frame index from a frame register and a given
  // offset if the offset doesn't fit in the immediate field.  Use a clobber
  // register to hold calculated address.
  void prepareReplaceFI(MachineInstr &MI, Register &FrameReg, int64_t &Offset,
                        int64_t Bytes = 0);
  // Replace the frame index in \p MI with a frame register and a given offset
  // if it fits in the immediate field.  Otherwise, use pre-calculated address
  // in a clobber regsiter.
  void replaceFI(MachineInstr &MI, Register FrameReg, int64_t Offset,
                 int FIOperandNum);

  // Expand and eliminate Frame Index of pseudo STQrii and LDQrii.
  void processSTQ(MachineInstr &MI, Register FrameReg, int64_t Offset,
                  int FIOperandNum);
  void processLDQ(MachineInstr &MI, Register FrameReg, int64_t Offset,
                  int FIOperandNum);
  // Expand and eliminate Frame Index of pseudo STVMrii and LDVMrii.
  void processSTVM(MachineInstr &MI, Register FrameReg, int64_t Offset,
                   int FIOperandNum);
  void processLDVM(MachineInstr &MI, Register FrameReg, int64_t Offset,
                   int FIOperandNum);
  // Expand and eliminate Frame Index of pseudo STVM512rii and LDVM512rii.
  void processSTVM512(MachineInstr &MI, Register FrameReg, int64_t Offset,
                      int FIOperandNum);
  void processLDVM512(MachineInstr &MI, Register FrameReg, int64_t Offset,
                      int FIOperandNum);

public:
  EliminateFrameIndex(const TargetInstrInfo &TII, const TargetRegisterInfo &TRI,
                      const DebugLoc &DL, MachineBasicBlock &MBB,
                      MachineBasicBlock::iterator II)
      : TII(TII), TRI(TRI), DL(DL), MBB(MBB), II(II), clobber(Dioptase::SX13) {}

  // Expand and eliminate Frame Index from MI
  void processMI(MachineInstr &MI, Register FrameReg, int64_t Offset,
                 int FIOperandNum);
};
} // namespace

// Prepare the frame index if it doesn't fit in the immediate field.  Use
// clobber register to hold calculated address.
void EliminateFrameIndex::prepareReplaceFI(MachineInstr &MI, Register &FrameReg,
                                           int64_t &Offset, int64_t Bytes) {
  if (isInt<32>(Offset) && isInt<32>(Offset + Bytes)) {
    // If the offset is small enough to fit in the immediate field, directly
    // encode it.  So, nothing to prepare here.
    return;
  }

  // If the offset doesn't fit, emit following codes.  This clobbers SX13
  // which we always know is available here.
  //   lea     %clobber, Offset@lo
  //   and     %clobber, %clobber, (32)0
  //   lea.sl  %clobber, Offset@hi(FrameReg, %clobber)
  build(Dioptase::LEAzii, clobber).addImm(0).addImm(0).addImm(Lo_32(Offset));
  build(Dioptase::ANDrm, clobber).addReg(clobber).addImm(M0(32));
  build(Dioptase::LEASLrri, clobber)
      .addReg(clobber)
      .addReg(FrameReg)
      .addImm(Hi_32(Offset));

  // Use clobber register as a frame register and 0 offset
  FrameReg = clobber;
  Offset = 0;
}

// Replace the frame index in \p MI with a proper byte and framereg offset.
void EliminateFrameIndex::replaceFI(MachineInstr &MI, Register FrameReg,
                                    int64_t Offset, int FIOperandNum) {
  assert(isInt<32>(Offset));

  // The offset must be small enough to fit in the immediate field after
  // call of prepareReplaceFI.  Therefore, we directly encode it.
  MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, false);
  MI.getOperand(FIOperandNum + offsetToDisp(MI)).ChangeToImmediate(Offset);
}

void EliminateFrameIndex::processSTQ(MachineInstr &MI, Register FrameReg,
                                     int64_t Offset, int FIOperandNum) {
  assert(MI.getOpcode() == Dioptase::STQrii);
  LLVM_DEBUG(dbgs() << "processSTQ: "; MI.dump());

  prepareReplaceFI(MI, FrameReg, Offset, 8);

  Register SrcReg = MI.getOperand(3).getReg();
  Register SrcHiReg = getSubReg(SrcReg, Dioptase::sub_even);
  Register SrcLoReg = getSubReg(SrcReg, Dioptase::sub_odd);
  // Dioptase stores HiReg to 8(addr) and LoReg to 0(addr)
  MachineInstr *StMI = build(Dioptase::STrii)
                           .addReg(FrameReg)
                           .addImm(0)
                           .addImm(0)
                           .addReg(SrcLoReg);
  replaceFI(*StMI, FrameReg, Offset, 0);
  // Mutate to 'hi' store.
  MI.setDesc(get(Dioptase::STrii));
  MI.getOperand(3).setReg(SrcHiReg);
  Offset += 8;
  replaceFI(MI, FrameReg, Offset, FIOperandNum);
}

void EliminateFrameIndex::processLDQ(MachineInstr &MI, Register FrameReg,
                                     int64_t Offset, int FIOperandNum) {
  assert(MI.getOpcode() == Dioptase::LDQrii);
  LLVM_DEBUG(dbgs() << "processLDQ: "; MI.dump());

  prepareReplaceFI(MI, FrameReg, Offset, 8);

  Register DestReg = MI.getOperand(0).getReg();
  Register DestHiReg = getSubReg(DestReg, Dioptase::sub_even);
  Register DestLoReg = getSubReg(DestReg, Dioptase::sub_odd);
  // Dioptase loads HiReg from 8(addr) and LoReg from 0(addr)
  MachineInstr *StMI =
      build(Dioptase::LDrii, DestLoReg).addReg(FrameReg).addImm(0).addImm(0);
  replaceFI(*StMI, FrameReg, Offset, 1);
  MI.setDesc(get(Dioptase::LDrii));
  MI.getOperand(0).setReg(DestHiReg);
  Offset += 8;
  replaceFI(MI, FrameReg, Offset, FIOperandNum);
}

void EliminateFrameIndex::processSTVM(MachineInstr &MI, Register FrameReg,
                                      int64_t Offset, int FIOperandNum) {
  assert(MI.getOpcode() == Dioptase::STVMrii);
  LLVM_DEBUG(dbgs() << "processSTVM: "; MI.dump());

  // Original MI is:
  //   STVMrii frame-index, 0, offset, reg (, memory operand)
  // Convert it to:
  //   SVMi   tmp-reg, reg, 0
  //   STrii  frame-reg, 0, offset, tmp-reg
  //   SVMi   tmp-reg, reg, 1
  //   STrii  frame-reg, 0, offset+8, tmp-reg
  //   SVMi   tmp-reg, reg, 2
  //   STrii  frame-reg, 0, offset+16, tmp-reg
  //   SVMi   tmp-reg, reg, 3
  //   STrii  frame-reg, 0, offset+24, tmp-reg

  prepareReplaceFI(MI, FrameReg, Offset, 24);

  Register SrcReg = MI.getOperand(3).getReg();
  bool isKill = MI.getOperand(3).isKill();
  // FIXME: it would be better to scavenge a register here instead of
  // reserving SX16 all of the time.
  Register TmpReg = Dioptase::SX16;
  for (int i = 0; i < 3; ++i) {
    build(Dioptase::SVMmr, TmpReg).addReg(SrcReg).addImm(i);
    MachineInstr *StMI = build(Dioptase::STrii)
                             .addReg(FrameReg)
                             .addImm(0)
                             .addImm(0)
                             .addReg(TmpReg, getKillRegState(true));
    replaceFI(*StMI, FrameReg, Offset, 0);
    Offset += 8;
  }
  build(Dioptase::SVMmr, TmpReg)
      .addReg(SrcReg, getKillRegState(isKill))
      .addImm(3);
  MI.setDesc(get(Dioptase::STrii));
  MI.getOperand(3).ChangeToRegister(TmpReg, false, false, true);
  replaceFI(MI, FrameReg, Offset, FIOperandNum);
}

void EliminateFrameIndex::processLDVM(MachineInstr &MI, Register FrameReg,
                                      int64_t Offset, int FIOperandNum) {
  assert(MI.getOpcode() == Dioptase::LDVMrii);
  LLVM_DEBUG(dbgs() << "processLDVM: "; MI.dump());

  // Original MI is:
  //   LDVMri reg, frame-index, 0, offset (, memory operand)
  // Convert it to:
  //   LDrii  tmp-reg, frame-reg, 0, offset
  //   LVMir vm, 0, tmp-reg
  //   LDrii  tmp-reg, frame-reg, 0, offset+8
  //   LVMir_m vm, 1, tmp-reg, vm
  //   LDrii  tmp-reg, frame-reg, 0, offset+16
  //   LVMir_m vm, 2, tmp-reg, vm
  //   LDrii  tmp-reg, frame-reg, 0, offset+24
  //   LVMir_m vm, 3, tmp-reg, vm

  prepareReplaceFI(MI, FrameReg, Offset, 24);

  Register DestReg = MI.getOperand(0).getReg();
  // FIXME: it would be better to scavenge a register here instead of
  // reserving SX16 all of the time.
  unsigned TmpReg = Dioptase::SX16;
  for (int i = 0; i < 4; ++i) {
    if (i != 3) {
      MachineInstr *StMI =
          build(Dioptase::LDrii, TmpReg).addReg(FrameReg).addImm(0).addImm(0);
      replaceFI(*StMI, FrameReg, Offset, 1);
      Offset += 8;
    } else {
      // Last LDrii replace the target instruction.
      MI.setDesc(get(Dioptase::LDrii));
      MI.getOperand(0).ChangeToRegister(TmpReg, true);
    }
    // First LVM is LVMir.  Others are LVMir_m.  Last LVM places at the
    // next of the target instruction.
    if (i == 0)
      build(Dioptase::LVMir, DestReg)
          .addImm(i)
          .addReg(TmpReg, getKillRegState(true));
    else if (i != 3)
      build(Dioptase::LVMir_m, DestReg)
          .addImm(i)
          .addReg(TmpReg, getKillRegState(true))
          .addReg(DestReg);
    else
      BuildMI(*MI.getParent(), std::next(II), DL, get(Dioptase::LVMir_m),
              DestReg)
          .addImm(3)
          .addReg(TmpReg, getKillRegState(true))
          .addReg(DestReg);
  }
  replaceFI(MI, FrameReg, Offset, FIOperandNum);
}

void EliminateFrameIndex::processSTVM512(MachineInstr &MI, Register FrameReg,
                                         int64_t Offset, int FIOperandNum) {
  assert(MI.getOpcode() == Dioptase::STVM512rii);
  LLVM_DEBUG(dbgs() << "processSTVM512: "; MI.dump());

  prepareReplaceFI(MI, FrameReg, Offset, 56);

  Register SrcReg = MI.getOperand(3).getReg();
  Register SrcLoReg = getSubReg(SrcReg, Dioptase::sub_vm_odd);
  Register SrcHiReg = getSubReg(SrcReg, Dioptase::sub_vm_even);
  bool isKill = MI.getOperand(3).isKill();
  // FIXME: it would be better to scavenge a register here instead of
  // reserving SX16 all of the time.
  Register TmpReg = Dioptase::SX16;
  // store low part of VMP
  MachineInstr *LastMI = nullptr;
  for (int i = 0; i < 4; ++i) {
    LastMI = build(Dioptase::SVMmr, TmpReg).addReg(SrcLoReg).addImm(i);
    MachineInstr *StMI = build(Dioptase::STrii)
                             .addReg(FrameReg)
                             .addImm(0)
                             .addImm(0)
                             .addReg(TmpReg, getKillRegState(true));
    replaceFI(*StMI, FrameReg, Offset, 0);
    Offset += 8;
  }
  if (isKill)
    LastMI->addRegisterKilled(SrcLoReg, &TRI, true);
  // store high part of VMP
  for (int i = 0; i < 3; ++i) {
    build(Dioptase::SVMmr, TmpReg).addReg(SrcHiReg).addImm(i);
    MachineInstr *StMI = build(Dioptase::STrii)
                             .addReg(FrameReg)
                             .addImm(0)
                             .addImm(0)
                             .addReg(TmpReg, getKillRegState(true));
    replaceFI(*StMI, FrameReg, Offset, 0);
    Offset += 8;
  }
  LastMI = build(Dioptase::SVMmr, TmpReg).addReg(SrcHiReg).addImm(3);
  if (isKill) {
    LastMI->addRegisterKilled(SrcHiReg, &TRI, true);
    // Add implicit super-register kills to the particular MI.
    LastMI->addRegisterKilled(SrcReg, &TRI, true);
  }
  MI.setDesc(get(Dioptase::STrii));
  MI.getOperand(3).ChangeToRegister(TmpReg, false, false, true);
  replaceFI(MI, FrameReg, Offset, FIOperandNum);
}

void EliminateFrameIndex::processLDVM512(MachineInstr &MI, Register FrameReg,
                                         int64_t Offset, int FIOperandNum) {
  assert(MI.getOpcode() == Dioptase::LDVM512rii);
  LLVM_DEBUG(dbgs() << "processLDVM512: "; MI.dump());

  prepareReplaceFI(MI, FrameReg, Offset, 56);

  Register DestReg = MI.getOperand(0).getReg();
  Register DestLoReg = getSubReg(DestReg, Dioptase::sub_vm_odd);
  Register DestHiReg = getSubReg(DestReg, Dioptase::sub_vm_even);
  // FIXME: it would be better to scavenge a register here instead of
  // reserving SX16 all of the time.
  Register TmpReg = Dioptase::SX16;
  build(Dioptase::IMPLICIT_DEF, DestReg);
  for (int i = 0; i < 4; ++i) {
    MachineInstr *LdMI =
        build(Dioptase::LDrii, TmpReg).addReg(FrameReg).addImm(0).addImm(0);
    replaceFI(*LdMI, FrameReg, Offset, 1);
    build(Dioptase::LVMir_m, DestLoReg)
        .addImm(i)
        .addReg(TmpReg, getKillRegState(true))
        .addReg(DestLoReg);
    Offset += 8;
  }
  for (int i = 0; i < 3; ++i) {
    MachineInstr *LdMI =
        build(Dioptase::LDrii, TmpReg).addReg(FrameReg).addImm(0).addImm(0);
    replaceFI(*LdMI, FrameReg, Offset, 1);
    build(Dioptase::LVMir_m, DestHiReg)
        .addImm(i)
        .addReg(TmpReg, getKillRegState(true))
        .addReg(DestHiReg);
    Offset += 8;
  }
  MI.setDesc(get(Dioptase::LDrii));
  MI.getOperand(0).ChangeToRegister(TmpReg, true);
  BuildMI(*MI.getParent(), std::next(II), DL, get(Dioptase::LVMir_m), DestHiReg)
      .addImm(3)
      .addReg(TmpReg, getKillRegState(true))
      .addReg(DestHiReg);
  replaceFI(MI, FrameReg, Offset, FIOperandNum);
}

void EliminateFrameIndex::processMI(MachineInstr &MI, Register FrameReg,
                                    int64_t Offset, int FIOperandNum) {
  switch (MI.getOpcode()) {
  case Dioptase::STQrii:
    processSTQ(MI, FrameReg, Offset, FIOperandNum);
    return;
  case Dioptase::LDQrii:
    processLDQ(MI, FrameReg, Offset, FIOperandNum);
    return;
  case Dioptase::STVMrii:
    processSTVM(MI, FrameReg, Offset, FIOperandNum);
    return;
  case Dioptase::LDVMrii:
    processLDVM(MI, FrameReg, Offset, FIOperandNum);
    return;
  case Dioptase::STVM512rii:
    processSTVM512(MI, FrameReg, Offset, FIOperandNum);
    return;
  case Dioptase::LDVM512rii:
    processLDVM512(MI, FrameReg, Offset, FIOperandNum);
    return;
  }
  prepareReplaceFI(MI, FrameReg, Offset);
  replaceFI(MI, FrameReg, Offset, FIOperandNum);
}
*/
bool DioptaseRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                               int SPAdj, unsigned FIOperandNum,
                                               RegScavenger *RS) const {
  assert(false && "unimplemented");

  /*
  assert(SPAdj == 0 && "Unexpected");

MachineInstr &MI = *II;
int FrameIndex = MI.getOperand(FIOperandNum).getIndex();

MachineFunction &MF = *MI.getParent()->getParent();
const DioptaseSubtarget &Subtarget = MF.getSubtarget<DioptaseSubtarget>();
const DioptaseFrameLowering &TFI = *getFrameLowering(MF);
const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
const DioptaseRegisterInfo &TRI = *Subtarget.getRegisterInfo();
DebugLoc DL = MI.getDebugLoc();
EliminateFrameIndex EFI(TII, TRI, DL, *MI.getParent(), II);

// Retrieve FrameReg and byte offset for stack slot.
Register FrameReg;
int64_t Offset =
    TFI.getFrameIndexReference(MF, FrameIndex, FrameReg).getFixed();
Offset += MI.getOperand(FIOperandNum + offsetToDisp(MI)).getImm();

EFI.processMI(MI, FrameReg, Offset, FIOperandNum);
return false;*/
}

Register
DioptaseRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return Dioptase::R30;
}
