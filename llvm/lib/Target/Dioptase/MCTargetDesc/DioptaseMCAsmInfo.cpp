//===-- DioptaseMCAsmInfo.cpp - Dioptase asm properties
//-----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the DioptaseMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "DioptaseMCAsmInfo.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCValue.h"
#include "llvm/TargetParser/Triple.h"
#include <cassert>

using namespace llvm;

DioptaseMCAsmInfo::DioptaseMCAsmInfo(const Triple &TT,
                                     const MCTargetOptions &Options) {
  CodePointerSize = 2;
  CalleeSaveStackSlotSize = 2;
  CommentString = ";";
  SeparatorString = "$";
  UsesELFSectionDirectiveForBSS = true;
  SupportsDebugInformation = true;
}

void DioptaseMCAsmInfo::printSpecifierExpr(raw_ostream &OS,
                                           const MCSpecifierExpr &Expr) const {
  assert(false && "todo");
}

bool DioptaseMCAsmInfo::evaluateAsRelocatableImpl(
    const MCSpecifierExpr &Expr, MCValue &Res, const MCAssembler *Asm) const {
  assert(false && "todo");
}
