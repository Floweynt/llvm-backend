//===-- DioptaseFrameLowering.cpp - Dioptase Frame Information -*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Dioptase implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "DioptaseFrameLowering.h"
#include "DioptaseMachineFunctionInfo.h"
#include "DioptaseSubtarget.h"
#include "MCTargetDesc/DioptaseMCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/MC/MCDwarf.h"
#include <cassert>

using namespace llvm;

#define DEBUG_TYPE "dioptase-frame-lowering"

DioptaseFrameLowering::DioptaseFrameLowering(const DioptaseSubtarget &ST)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(1), -2),
      STI(ST) {}

// Return true if the specified function should have a dedicated frame
// pointer register.  This is true if frame pointer elimination is
// disabled, if it needs dynamic stack realignment, if the function has
// variable sized allocas, or if the frame address is taken.
bool DioptaseFrameLowering::hasFPImpl(const MachineFunction &MF) const {
  const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();

  const MachineFrameInfo &MFI = MF.getFrameInfo();
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
         RegInfo->hasStackRealignment(MF) || MFI.hasVarSizedObjects() ||
         MFI.isFrameAddressTaken();
}

bool DioptaseFrameLowering::hasBP(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterInfo *TRI = STI.getRegisterInfo();

  return MFI.hasVarSizedObjects() && TRI->hasStackRealignment(MF);
}

static uint64_t estimateFunctionSizeInBytes(const DioptaseInstrInfo *TII,
                                            const MachineFunction &MF) {
  uint64_t FuncSize = 0;
  for (auto &MBB : MF)
    for (auto &MI : MBB)
      FuncSize += TII->getInstSizeInBytes(MI);
  return FuncSize;
}

void DioptaseFrameLowering::emitPrologue(MachineFunction &MF,
                                         MachineBasicBlock &MBB) const {
  assert(false && "not implemented");
  /*
MachineFrameInfo &MFI = MF.getFrameInfo();
auto *DioptaseFI = MF.getInfo<DioptaseMachineFunctionInfo>();
const DioptaseRegisterInfo *RI = STI.getRegisterInfo();
const DioptaseInstrInfo *TII = STI.getInstrInfo();
MachineBasicBlock::iterator MBBI = MBB.begin();

Register SPReg = Dioptase::R3;
Register FPReg = Dioptase::R22;

// Debug location must be unknown since the first debug location is used
// to determine the end of the prologue.
DebugLoc DL;
// All calls are tail calls in GHC calling conv, and functions have no
// prologue/epilogue.
if (MF.getFunction().getCallingConv() == CallingConv::GHC)
  return;
// Determine the correct frame layout
determineFrameLayout(MF);

// First, compute final stack size.
uint64_t StackSize = MFI.getStackSize();
uint64_t RealStackSize = StackSize;

// Early exit if there is no need to allocate space in the stack.
if (StackSize == 0 && !MFI.adjustsStack())
  return;

uint64_t FirstSPAdjustAmount = getFirstSPAdjustAmount(MF);
// Split the SP adjustment to reduce the offsets of callee saved spill.
if (FirstSPAdjustAmount)
  StackSize = FirstSPAdjustAmount;

// Adjust stack.
adjustReg(MBB, MBBI, DL, SPReg, SPReg, -StackSize, MachineInstr::FrameSetup);
// Emit ".cfi_def_cfa_offset StackSize".
unsigned CFIIndex =
    MF.addFrameInst(MCCFIInstruction::cfiDefCfaOffset(nullptr, StackSize));
BuildMI(MBB, MBBI, DL, TII->get(TargetOpcode::CFI_INSTRUCTION))
    .addCFIIndex(CFIIndex)
    .setMIFlag(MachineInstr::FrameSetup);

const auto &CSI = MFI.getCalleeSavedInfo();

// The frame pointer is callee-saved, and code has been generated for us to
// save it to the stack. We need to skip over the storing of callee-saved
// registers as the frame pointer must be modified after it has been saved
// to the stack, not before.
std::advance(MBBI, CSI.size());

// Iterate over list of callee-saved registers and emit .cfi_offset
// directives.
for (const auto &Entry : CSI) {
  int64_t Offset = MFI.getObjectOffset(Entry.getFrameIdx());
  unsigned CFIIndex = MF.addFrameInst(MCCFIInstruction::createOffset(
      nullptr, RI->getDwarfRegNum(Entry.getReg(), true), Offset));
  BuildMI(MBB, MBBI, DL, TII->get(TargetOpcode::CFI_INSTRUCTION))
      .addCFIIndex(CFIIndex)
      .setMIFlag(MachineInstr::FrameSetup);
}

// Generate new FP.
if (hasFP(MF)) {
  adjustReg(MBB, MBBI, DL, FPReg, SPReg,
            StackSize - DioptaseFI->getVarArgsSaveSize(),
            MachineInstr::FrameSetup);

  // Emit ".cfi_def_cfa $fp, DioptaseFI->getVarArgsSaveSize()"
  unsigned CFIIndex = MF.addFrameInst(
      MCCFIInstruction::cfiDefCfa(nullptr, RI->getDwarfRegNum(FPReg, true),
                                  DioptaseFI->getVarArgsSaveSize()));
  BuildMI(MBB, MBBI, DL, TII->get(TargetOpcode::CFI_INSTRUCTION))
      .addCFIIndex(CFIIndex)
      .setMIFlag(MachineInstr::FrameSetup);
}

// Emit the second SP adjustment after saving callee saved registers.
if (FirstSPAdjustAmount) {
  uint64_t SecondSPAdjustAmount = RealStackSize - FirstSPAdjustAmount;
  assert(SecondSPAdjustAmount > 0 &&
         "SecondSPAdjustAmount should be greater than zero");
  adjustReg(MBB, MBBI, DL, SPReg, SPReg, -SecondSPAdjustAmount,
            MachineInstr::FrameSetup);

  if (!hasFP(MF)) {
    // If we are using a frame-pointer, and thus emitted ".cfi_def_cfa fp, 0",
    // don't emit an sp-based .cfi_def_cfa_offset
    // Emit ".cfi_def_cfa_offset RealStackSize"
    unsigned CFIIndex = MF.addFrameInst(
        MCCFIInstruction::cfiDefCfaOffset(nullptr, RealStackSize));
    BuildMI(MBB, MBBI, DL, TII->get(TargetOpcode::CFI_INSTRUCTION))
        .addCFIIndex(CFIIndex)
        .setMIFlag(MachineInstr::FrameSetup);
  }
}

if (hasFP(MF)) {
  // Realign stack.
  if (RI->hasStackRealignment(MF)) {
    unsigned Align = Log2(MFI.getMaxAlign());
    assert(Align > 0 && "The stack realignment size is invalid!");
    BuildMI(MBB, MBBI, DL,
            TII->get(IsLA64 ? Dioptase::BSTRINS_D : Dioptase::BSTRINS_W),
            SPReg)
        .addReg(SPReg)
        .addReg(Dioptase::R0)
        .addImm(Align - 1)
        .addImm(0)
        .setMIFlag(MachineInstr::FrameSetup);
    // FP will be used to restore the frame in the epilogue, so we need
    // another base register BP to record SP after re-alignment. SP will
    // track the current stack after allocating variable sized objects.
    if (hasBP(MF)) {
      // move BP, $sp
      BuildMI(MBB, MBBI, DL, TII->get(Dioptase::OR),
              DioptaseABI::getBPReg())
          .addReg(SPReg)
          .addReg(Dioptase::R0)
          .setMIFlag(MachineInstr::FrameSetup);
    }
  }
}*/
}

void DioptaseFrameLowering::emitEpilogue(MachineFunction &MF,
                                         MachineBasicBlock &MBB) const {

  assert(false && "not implemented");
  /*
    const DioptaseRegisterInfo *RI = STI.getRegisterInfo();
    MachineFrameInfo &MFI = MF.getFrameInfo();
    auto *DioptaseFI = MF.getInfo<DioptaseMachineFunctionInfo>();
    Register SPReg = Dioptase::R3;
    // All calls are tail calls in GHC calling conv, and functions have no
    // prologue/epilogue.
    if (MF.getFunction().getCallingConv() == CallingConv::GHC)
      return;
    MachineBasicBlock::iterator MBBI = MBB.getFirstTerminator();
    DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

    const auto &CSI = MFI.getCalleeSavedInfo();
    // Skip to before the restores of callee-saved registers.
    auto LastFrameDestroy = MBBI;
    if (!CSI.empty())
      LastFrameDestroy = std::prev(MBBI, CSI.size());

    // Get the number of bytes from FrameInfo.
    uint64_t StackSize = MFI.getStackSize();

    // Restore the stack pointer.
    if (RI->hasStackRealignment(MF) || MFI.hasVarSizedObjects()) {
      assert(hasFP(MF) && "frame pointer should not have been eliminated");
      adjustReg(MBB, LastFrameDestroy, DL, SPReg, Dioptase::R22,
                -StackSize + DioptaseFI->getVarArgsSaveSize(),
                MachineInstr::FrameDestroy);
    }

    uint64_t FirstSPAdjustAmount = getFirstSPAdjustAmount(MF);
    if (FirstSPAdjustAmount) {
      uint64_t SecondSPAdjustAmount = StackSize - FirstSPAdjustAmount;
      assert(SecondSPAdjustAmount > 0 &&
             "SecondSPAdjustAmount should be greater than zero");

      adjustReg(MBB, LastFrameDestroy, DL, SPReg, SPReg, SecondSPAdjustAmount,
                MachineInstr::FrameDestroy);
      StackSize = FirstSPAdjustAmount;
    }

    // Deallocate stack
    adjustReg(MBB, MBBI, DL, SPReg, SPReg, StackSize,
    MachineInstr::FrameDestroy);*/
}

void DioptaseFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                                 BitVector &SavedRegs,
                                                 RegScavenger *RS) const {

  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);

  /* TODO: ????
  if (hasFP(MF)) {
    SavedRegs.set(Dioptase::R1);
    SavedRegs.set(Dioptase::R22);
  }
  // Mark BP as used if function has dedicated base pointer.
  if (hasBP(MF))
    SavedRegs.set(DioptaseABI::getBPReg());*/
}

// Do not preserve stack space within prologue for outgoing variables if the
// function contains variable size objects.
// Let eliminateCallFramePseudoInstr preserve stack space for it.
bool DioptaseFrameLowering::hasReservedCallFrame(
    const MachineFunction &MF) const {
  return !MF.getFrameInfo().hasVarSizedObjects();
}

// Eliminate ADJCALLSTACKDOWN, ADJCALLSTACKUP pseudo instructions.
MachineBasicBlock::iterator
DioptaseFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  assert(false && "not implemented");
  /*

    Register SPReg = Dioptase::R3;
    DebugLoc DL = MI->getDebugLoc();

    if (!hasReservedCallFrame(MF)) {
      // If space has not been reserved for a call frame, ADJCALLSTACKDOWN and
      // ADJCALLSTACKUP must be converted to instructions manipulating the stack
      // pointer. This is necessary when there is a variable length stack
      // allocation (e.g. alloca), which means it's not possible to allocate
      // space for outgoing arguments from within the function prologue.
      int64_t Amount = MI->getOperand(0).getImm();

      if (Amount != 0) {
        // Ensure the stack remains aligned after adjustment.
        Amount = alignSPAdjust(Amount);

        if (MI->getOpcode() == Dioptase::ADJCALLSTACKDOWN)
          Amount = -Amount;

        adjustReg(MBB, MI, DL, SPReg, SPReg, Amount, MachineInstr::NoFlags);
      }
    }

    return MBB.erase(MI);*/
}

StackOffset
DioptaseFrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                              Register &FrameReg) const {

  assert(false && "not implemented");
  /*
    const MachineFrameInfo &MFI = MF.getFrameInfo();
    const TargetRegisterInfo *RI = MF.getSubtarget().getRegisterInfo();
    auto *DioptaseFI = MF.getInfo<DioptaseMachineFunctionInfo>();
    uint64_t StackSize = MFI.getStackSize();
    uint64_t FirstSPAdjustAmount = getFirstSPAdjustAmount(MF);

    // Callee-saved registers should be referenced relative to the stack
    // pointer (positive offset), otherwise use the frame pointer (negative
    // offset).
    const auto &CSI = MFI.getCalleeSavedInfo();
    int MinCSFI = 0;
    int MaxCSFI = -1;
    StackOffset Offset =
        StackOffset::getFixed(MFI.getObjectOffset(FI) - getOffsetOfLocalArea() +
                              MFI.getOffsetAdjustment());

    if (CSI.size()) {
      MinCSFI = CSI[0].getFrameIdx();
      MaxCSFI = CSI[CSI.size() - 1].getFrameIdx();
    }

    if (FI >= MinCSFI && FI <= MaxCSFI) {
      FrameReg = Dioptase::R3;
      if (FirstSPAdjustAmount)
        Offset += StackOffset::getFixed(FirstSPAdjustAmount);
      else
        Offset += StackOffset::getFixed(StackSize);
    } else if (RI->hasStackRealignment(MF) && !MFI.isFixedObjectIndex(FI)) {
      // If the stack was realigned, the frame pointer is set in order to allow
      // SP to be restored, so we need another base register to record the stack
      // after realignment.
      FrameReg = hasBP(MF) ? DioptaseABI::getBPReg() : Dioptase::R3;
      Offset += StackOffset::getFixed(StackSize);
    } else {
      FrameReg = RI->getFrameRegister(MF);
      if (hasFP(MF))
        Offset += StackOffset::getFixed(DioptaseFI->getVarArgsSaveSize());
      else
        Offset += StackOffset::getFixed(StackSize);
    }

    return Offset;*/
}
