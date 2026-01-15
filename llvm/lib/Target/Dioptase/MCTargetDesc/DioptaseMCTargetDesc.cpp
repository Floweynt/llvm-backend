//===-- DioptaseMCTargetDesc.cpp - MSP430 Target Descriptions
//---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides Dioptase specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "DioptaseMCTargetDesc.h"
#include "DioptaseInstPrinter.h"
#include "DioptaseMCAsmInfo.h"
#include "TargetInfo/DioptaseTargetInfo.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "DioptaseGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "DioptaseGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "DioptaseGenRegisterInfo.inc"

static MCInstrInfo *createDioptaseMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitDioptaseMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createDioptaseMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitDioptaseMCRegisterInfo(X, Dioptase::R29);
  return X;
}

static MCAsmInfo *createDioptaseMCAsmInfo(const MCRegisterInfo &MRI,
                                          const Triple &TT,
                                          const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new DioptaseMCAsmInfo(TT, Options);

  // Initialize initial frame state.
  int stackGrowth = -2;

  // Initial state of the frame pointer is sp+ptr_size.
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(
      nullptr, MRI.getDwarfRegNum(Dioptase::R31, true), -stackGrowth);
  MAI->addInitialFrameState(Inst);

  // Add return address to move list
  MCCFIInstruction Inst2 = MCCFIInstruction::createOffset(
      nullptr, MRI.getDwarfRegNum(Dioptase::R31, true), stackGrowth);
  MAI->addInitialFrameState(Inst2);

  return MAI;
}

static MCSubtargetInfo *
createDioptaseMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createDioptaseMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCInstPrinter *createDioptaseMCInstPrinter(const Triple &T,
                                                  unsigned SyntaxVariant,
                                                  const MCAsmInfo &MAI,
                                                  const MCInstrInfo &MII,
                                                  const MCRegisterInfo &MRI) {
  return new DioptaseInstPrinter(MAI, MII, MRI);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeDioptaseTargetMC() {
  Target &T = getTheDioptaseTarget();

  TargetRegistry::RegisterMCAsmInfo(T, createDioptaseMCAsmInfo);
  TargetRegistry::RegisterMCInstrInfo(T, createDioptaseMCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(T, createDioptaseMCRegisterInfo);
  TargetRegistry::RegisterMCSubtargetInfo(T, createDioptaseMCSubtargetInfo);
  TargetRegistry::RegisterMCInstPrinter(T, createDioptaseMCInstPrinter);
  // TargetRegistry::RegisterMCCodeEmitter(T, createDioptaseMCCodeEmitter);
  // TargetRegistry::RegisterMCAsmBackend(T, createDioptaseMCAsmBackend);
  // TargetRegistry::RegisterObjectTargetStreamer(T,
  // createDioptaseObjectTargetStreamer);
}
