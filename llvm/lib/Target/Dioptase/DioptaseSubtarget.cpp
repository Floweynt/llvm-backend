//===-- DioptaseSubtarget.cpp - Dioptase Subtarget Information ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Dioptase specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "DioptaseSubtarget.h"
#include "DioptaseSelectionDAGInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "ve-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "DioptaseGenSubtargetInfo.inc"

void DioptaseSubtarget::anchor() {}

DioptaseSubtarget &DioptaseSubtarget::initializeSubtargetDependencies(StringRef CPU,
                                                          StringRef FS) {
  // Default feature settings
  EnableVPU = false;

  // Determine default and user specified characteristics
  std::string CPUName = std::string(CPU);
  if (CPUName.empty())
    CPUName = "generic";

  // Parse features string.
  ParseSubtargetFeatures(CPUName, /*TuneCPU=*/CPU, FS);

  return *this;
}

DioptaseSubtarget::DioptaseSubtarget(const Triple &TT, const std::string &CPU,
                         const std::string &FS, const TargetMachine &TM)
    : DioptaseGenSubtargetInfo(TT, CPU, /*TuneCPU=*/CPU, FS), TargetTriple(TT),
      InstrInfo(initializeSubtargetDependencies(CPU, FS)), TLInfo(TM, *this),
      FrameLowering(*this) {
  TSInfo = std::make_unique<DioptaseSelectionDAGInfo>();
}

DioptaseSubtarget::~DioptaseSubtarget() = default;

const SelectionDAGTargetInfo *DioptaseSubtarget::getSelectionDAGInfo() const {
  return TSInfo.get();
}

uint64_t DioptaseSubtarget::getAdjustedFrameSize(uint64_t FrameSize) const {
  // Calculate adjusted frame size by adding the size of RSA frame,
  // return address, and frame poitner as described in DioptaseFrameLowering.cpp.
  const DioptaseFrameLowering *TFL = getFrameLowering();

  FrameSize += getRsaSize();
  FrameSize = alignTo(FrameSize, TFL->getStackAlign());

  return FrameSize;
}

bool DioptaseSubtarget::enableMachineScheduler() const { return true; }

