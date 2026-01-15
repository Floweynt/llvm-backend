//===-- DioptaseInstrInfo.cpp - Dioptase Instruction Information ------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Dioptase implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "DioptaseInstrInfo.h"
#include "DioptaseRegisterInfo.h"
#include "DioptaseSubtarget.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "DioptaseGenInstrInfo.inc"

DioptaseInstrInfo::DioptaseInstrInfo(const DioptaseSubtarget &STI)
    : DioptaseGenInstrInfo(STI, RI), RI() {}
