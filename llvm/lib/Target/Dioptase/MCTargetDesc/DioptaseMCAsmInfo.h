//===-- DioptaseMCAsmInfo.h - Dioptase asm properties ---------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the DioptaseMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_Dioptase_ASM_INFO_H
#define LLVM_Dioptase_ASM_INFO_H

#include "llvm/MC/MCAsmInfoELF.h"
#include "llvm/MC/MCExpr.h"

namespace llvm {

class Triple;

/// Specifies the format of Dioptase assembly files.
class DioptaseMCAsmInfo : public MCAsmInfoELF {
public:
  explicit DioptaseMCAsmInfo(const Triple &TT, const MCTargetOptions &Options);
  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
  bool evaluateAsRelocatableImpl(const MCSpecifierExpr &Expr, MCValue &Res,
                                 const MCAssembler *Asm) const override;
};

} // namespace llvm
#endif // LLVM_Dioptase_ASM_INFO_H
