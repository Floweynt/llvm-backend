//===-- Dioptase.h - Top-level interface for Dioptase representation --------*-
// C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// Dioptase back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIOPTASE_DIOPTASE_H
#define LLVM_LIB_TARGET_DIOPTASE_DIOPTASE_H

#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class AsmPrinter;
class FunctionPass;
class MCInst;
class MachineInstr;
class PassRegistry;
class DioptaseTargetMachine;

FunctionPass *createDioptaseISelDag(DioptaseTargetMachine &TM, CodeGenOptLevel OptLevel);
void initializeDioptaseAsmPrinterPass(PassRegistry &);
void initializeDioptaseDAGToDAGISelLegacyPass(PassRegistry &);

void LowerDioptaseMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                       AsmPrinter &AP);
} // namespace llvm

#endif
