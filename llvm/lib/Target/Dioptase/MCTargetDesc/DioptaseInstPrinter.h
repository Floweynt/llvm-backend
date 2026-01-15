//===-- DioptaseInstPrinter.h - Convert Dioptase MCInst to assembly syntax
//------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints an Dioptase MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIOPTASE_INSTPRINTER_DIOPTASEINSTPRINTER_H
#define LLVM_LIB_TARGET_DIOPTASE_INSTPRINTER_DIOPTASEINSTPRINTER_H

#include "MCTargetDesc/DioptaseMCTargetDesc.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>

namespace llvm {

class DioptaseInstPrinter : public MCInstPrinter {
public:
  DioptaseInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                      const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

  void printRegName(raw_ostream &OS, MCRegister Reg) override {

    assert(false && "todo");
  }

  void printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                 const MCSubtargetInfo &STI, raw_ostream &OS) override {
    assert(false && "todo");
  }

  static const char *getRegisterName(MCRegister Reg);

private:
  void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O) {
    assert(false && "todo");
  }

  std::pair<const char *, uint64_t>
  getMnemonic(const MCInst &MI) const override;
  void printInstruction(const MCInst *MI, uint64_t Address, raw_ostream &O);
  bool printAliasInstr(const MCInst *MI, uint64_t Address, raw_ostream &O);
  void printCustomAliasOperand(const MCInst *MI, uint64_t Address,
                               unsigned OpIdx, unsigned);
};
} // namespace llvm

#endif
