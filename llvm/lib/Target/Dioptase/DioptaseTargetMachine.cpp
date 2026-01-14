//===-- DioptaseTargetMachine.cpp - Define TargetMachine for Dioptase -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "DioptaseTargetMachine.h"
#include "TargetInfo/DioptaseTargetInfo.h"
#include "Dioptase.h"
#include "DioptaseMachineFunctionInfo.h"
#include "DioptaseTargetTransformInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include <optional>

using namespace llvm;

#define DEBUG_TYPE "ve"

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeDioptaseTarget() {
  // Register the target.
  RegisterTargetMachine<DioptaseTargetMachine> X(getTheDioptaseTarget());

  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeDioptaseAsmPrinterPass(PR);
  initializeDioptaseDAGToDAGISelLegacyPass(PR);
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

namespace {
class DioptaseELFTargetObjectFile : public TargetLoweringObjectFileELF {
  void Initialize(MCContext &Ctx, const TargetMachine &TM) override {
    TargetLoweringObjectFileELF::Initialize(Ctx, TM);
    InitializeELF(TM.Options.UseInitArray);
  }
};
} // namespace

static std::unique_ptr<TargetLoweringObjectFile> createTLOF() {
  return std::make_unique<DioptaseELFTargetObjectFile>();
}

/// Create an Aurora Dioptase architecture model
DioptaseTargetMachine::DioptaseTargetMachine(const Target &T, const Triple &TT,
                                 StringRef CPU, StringRef FS,
                                 const TargetOptions &Options,
                                 std::optional<Reloc::Model> RM,
                                 std::optional<CodeModel::Model> CM,
                                 CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, TT.computeDataLayout(), TT, CPU, FS, Options,
                               getEffectiveRelocModel(RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(createTLOF()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  initAsmInfo();
}

DioptaseTargetMachine::~DioptaseTargetMachine() = default;

TargetTransformInfo
DioptaseTargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(std::make_unique<DioptaseTTIImpl>(this, F));
}

MachineFunctionInfo *DioptaseTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return DioptaseMachineFunctionInfo::create<DioptaseMachineFunctionInfo>(Allocator, F,
                                                              STI);
}

namespace {
/// Dioptase Code Generator Pass Configuration Options.
class DioptasePassConfig : public TargetPassConfig {
public:
  DioptasePassConfig(DioptaseTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  DioptaseTargetMachine &getDioptaseTargetMachine() const {
    return getTM<DioptaseTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *DioptaseTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new DioptasePassConfig(*this, PM);
}

void DioptasePassConfig::addIRPasses() {
  // Dioptase requires atomic expand pass.
  addPass(createAtomicExpandLegacyPass());
  TargetPassConfig::addIRPasses();
}

bool DioptasePassConfig::addInstSelector() {
  addPass(createDioptaseISelDag(getDioptaseTargetMachine()));
  return false;
}

void DioptasePassConfig::addPreEmitPass() {
  // LVLGen should be called after scheduling and register allocation
  addPass(createLVLGenPass());
}

