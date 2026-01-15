//===-- DioptaseFrameLowering.h - Define frame lowering for Dioptase --*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class implements Dioptase-specific bits of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Dioptase_DioptaseFRAMELOWERING_H
#define LLVM_LIB_TARGET_Dioptase_DioptaseFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Support/TypeSize.h"

namespace llvm {

class DioptaseSubtarget;
class DioptaseFrameLowering : public TargetFrameLowering {
public:
  explicit DioptaseFrameLowering(const DioptaseSubtarget &ST);

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitPrologueInsns(MachineFunction &MF, MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator MBBI, uint64_t NumBytes,
                         bool RequireFPUpdate) const;
  void emitEpilogueInsns(MachineFunction &MF, MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator MBBI, uint64_t NumBytes,
                         bool RequireFPUpdate) const;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;

  bool hasBP(const MachineFunction &MF) const;
  bool hasGOT(const MachineFunction &MF) const;

  bool hasReservedCallFrame(const MachineFunction &MF) const override;

  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS = nullptr) const override;

  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,
                                     Register &FrameReg) const override;

  const SpillSlot *
  getCalleeSavedSpillSlots(unsigned &NumEntries) const override {
    /*static const SpillSlot Offsets[] = {
     * TODO
        {Dioptase::SX17, 40},  {Dioptase::SX18, 48},  {Dioptase::SX19, 56},
        {Dioptase::SX20, 64},  {Dioptase::SX21, 72},  {Dioptase::SX22, 80},
        {Dioptase::SX23, 88},  {Dioptase::SX24, 96},  {Dioptase::SX25, 104},
        {Dioptase::SX26, 112}, {Dioptase::SX27, 120}, {Dioptase::SX28, 128},
        {Dioptase::SX29, 136}, {Dioptase::SX30, 144}, {Dioptase::SX31, 152},
        {Dioptase::SX32, 160}, {Dioptase::SX33, 168}};
    NumEntries = std::size(Offsets);*/
    return {};
  }

protected:
  const DioptaseSubtarget &STI;

  bool hasFPImpl(const MachineFunction &MF) const override;

private:
  // Returns true if MF is a leaf procedure.
  bool isLeafProc(MachineFunction &MF) const;

  // Emits code for adjusting SP in function prologue/epilogue.
  void emitSPAdjustment(MachineFunction &MF, MachineBasicBlock &MBB,
                        MachineBasicBlock::iterator MBBI, int64_t NumBytes,
                        MaybeAlign MayAlign = MaybeAlign()) const;

  // Emits code for extending SP in function prologue/epilogue.
  void emitSPExtend(MachineFunction &MF, MachineBasicBlock &MBB,
                    MachineBasicBlock::iterator MBBI) const;
};

} // namespace llvm

#endif
