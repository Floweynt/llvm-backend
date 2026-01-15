//===-- DioptaseAsmPrinter.cpp - Dioptase LLVM assembly writer
//----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GAS-format Dioptase assembly language.
//
//===----------------------------------------------------------------------===//

#include "Dioptase.h"
#include "DioptaseSubtarget.h"
#include "DioptaseTargetMachine.h"
#include "MCTargetDesc/DioptaseInstPrinter.h"
#include "MCTargetDesc/DioptaseMCAsmInfo.h"
#include "TargetInfo/DioptaseTargetInfo.h"

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include <cassert>

#define DEBUG_TYPE "avr-asm-printer"

using namespace llvm;

namespace {

/// An Dioptase assembly code printer.
class DioptaseAsmPrinter : public AsmPrinter {
public:
  DioptaseAsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer), ID), MRI(*TM.getMCRegisterInfo()) {}

  StringRef getPassName() const override { return "Dioptase Assembly Printer"; }

  void printOperand(const MachineInstr *MI, unsigned OpNo, raw_ostream &O);

  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNum,
                       const char *ExtraCode, raw_ostream &O) override;

  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNum,
                             const char *ExtraCode, raw_ostream &O) override;

  void emitInstruction(const MachineInstr *MI) override;

  const MCExpr *lowerConstant(const Constant *CV, const Constant *BaseCV,
                              uint64_t Offset) override;

  void emitXXStructor(const DataLayout &DL, const Constant *CV) override;

  bool doFinalization(Module &M) override;

  void emitStartOfAsmFile(Module &M) override;

  static char ID;

private:
  const MCRegisterInfo &MRI;
  bool EmittedStructorSymbolAttrs = false;
};

} // namespace

void DioptaseAsmPrinter::printOperand(const MachineInstr *MI, unsigned OpNo,
                                      raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(OpNo);

  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    O << DioptaseInstPrinter::getRegisterName(MO.getReg());
    break;
  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    break;
  case MachineOperand::MO_GlobalAddress:
    O << getSymbol(MO.getGlobal());
    break;
  case MachineOperand::MO_ExternalSymbol:
    O << *GetExternalSymbolSymbol(MO.getSymbolName());
    break;
  case MachineOperand::MO_MachineBasicBlock:
    O << *MO.getMBB()->getSymbol();
    break;
  default:
    llvm_unreachable("Not implemented yet!");
  }
}

bool DioptaseAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNum,
                                         const char *ExtraCode,
                                         raw_ostream &O) {
  // Default asm printer can only deal with some extra codes,
  // so try it first.
  if (!AsmPrinter::PrintAsmOperand(MI, OpNum, ExtraCode, O))
    return false;

  const MachineOperand &MO = MI->getOperand(OpNum);

  if (MO.getType() == MachineOperand::MO_GlobalAddress)
    PrintSymbolOperand(MO, O); // Print global symbols.
  else
    printOperand(MI, OpNum, O); // Fallback to ordinary cases.

  return false;
}

bool DioptaseAsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                               unsigned OpNum,
                                               const char *ExtraCode,
                                               raw_ostream &O) {
  if (ExtraCode && ExtraCode[0])
    return true; // Unknown modifier

  const MachineOperand &MO = MI->getOperand(OpNum);
  (void)MO;
  assert(MO.isReg() && "Unexpected inline asm memory operand");

  // If NumOpRegs == 2, then we assume it is product of a FrameIndex expansion
  // and the second operand is an Imm.
  const InlineAsm::Flag OpFlags(MI->getOperand(OpNum - 1).getImm());
  const unsigned NumOpRegs = OpFlags.getNumOperandRegisters();

  return false;
}

void DioptaseAsmPrinter::emitInstruction(const MachineInstr *MI) {
  assert(false && "todo");
}

const MCExpr *DioptaseAsmPrinter::lowerConstant(const Constant *CV,
                                                const Constant *BaseCV,
                                                uint64_t Offset) {

  assert(false && "todo");
}

void DioptaseAsmPrinter::emitXXStructor(const DataLayout &DL,
                                        const Constant *CV) {
  assert(false && "todo");
}

bool DioptaseAsmPrinter::doFinalization(Module &M) { assert(false && "todo"); }

void DioptaseAsmPrinter::emitStartOfAsmFile(Module &M) {
}

char DioptaseAsmPrinter::ID = 0;

INITIALIZE_PASS(DioptaseAsmPrinter, "dioptase-asm-printer",
                "Dioptase Assembly Printer", false, false)

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeDioptaseAsmPrinter() {
  llvm::RegisterAsmPrinter<DioptaseAsmPrinter> X(getTheDioptaseTarget());
}
