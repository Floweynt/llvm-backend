//===-- DioptaseTargetInfo.cpp - LoongArch Target Implementation ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/DioptaseTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
using namespace llvm;

Target &llvm::getTheDioptaseTarget() {
  static Target TheDioptase32Target;
  return TheDioptase32Target;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeDioptaseTargetInfo() {
  RegisterTarget<Triple::dioptase, /*HasJIT=*/false> X(
      getTheDioptaseTarget(), "dioptase", "Dioptase",
      "Dioptase");
}
