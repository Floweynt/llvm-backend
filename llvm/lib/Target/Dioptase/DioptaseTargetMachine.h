//===-- DioptaseTargetMachine.h - Define TargetMachine for Dioptase ---*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Dioptase specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIOPTASE_DIOPTASETARGETMACHINE_H
#define LLVM_LIB_TARGET_DIOPTASE_DIOPTASETARGETMACHINE_H

#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/Target/TargetMachine.h"
#include "DioptaseSubtarget.h"

namespace llvm {

class Module;

class DioptaseTargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  DioptaseSubtarget Subtarget;
  // Hold Strings that can be free'd all together with DioptaseTargetMachine
  //   e.g.: "GCC_except_tableXX" string.
  std::list<std::string> StrList;

public:
  DioptaseTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                  StringRef FS, const TargetOptions &Options,
                  std::optional<Reloc::Model> RM,
                  std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                  bool JIT);
  ~DioptaseTargetMachine() override;

  const DioptaseSubtarget *getSubtargetImpl() const { return &Subtarget; }
  const DioptaseSubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }
  std::list<std::string> *getStrList() const {
    return const_cast<std::list<std::string> *>(&StrList);
  }

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  bool isMachineVerifierClean() const override { return false; }

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  unsigned getSjLjDataSize() const override { return 64; }
};


} // end namespace llvm

#endif

