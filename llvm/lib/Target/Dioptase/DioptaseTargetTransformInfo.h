//===- DioptaseTargetTransformInfo.h - Dioptase specific TTI ------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file a TargetTransformInfoImplBase conforming object specific to the
/// Dioptase target machine. It uses the target's detailed information to
/// provide more precise answers to certain TTI queries, while letting the
/// target independent and default TTI implementations handle the rest.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIOPTASE_DIOPTASETARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_DIOPTASE_DIOPTASETARGETTRANSFORMINFO_H

#include "Dioptase.h"
#include "DioptaseTargetMachine.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"

namespace llvm {

class DioptaseTTIImpl final : public BasicTTIImplBase<DioptaseTTIImpl> {
  using BaseT = BasicTTIImplBase<DioptaseTTIImpl>;
  friend BaseT;

  const DioptaseSubtarget *ST;
  const DioptaseTargetLowering *TLI;

  const DioptaseSubtarget *getST() const { return ST; }
  const DioptaseTargetLowering *getTLI() const { return TLI; }

  static bool isSupportedReduction(Intrinsic::ID ReductionID) {
#define DioptaseC_VP_CASE(SUFFIX)                                              \
  case Intrinsic::vp_reduce_##SUFFIX:                                          \
  case Intrinsic::vector_reduce_##SUFFIX:

    switch (ReductionID) {
      DioptaseC_VP_CASE(add) DioptaseC_VP_CASE(and) DioptaseC_VP_CASE(or)
          DioptaseC_VP_CASE(xor) DioptaseC_VP_CASE(smax) return true;

    default:
      return false;
    }
#undef DioptaseC_VP_CASE
  }

public:
  explicit DioptaseTTIImpl(const DioptaseTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  unsigned getNumberOfRegisters(unsigned ClassID) const override {
    bool VectorRegs = (ClassID == 1);
    if (VectorRegs) {
      // TODO report vregs once vector isel is stable.
      return 0;
    }

    return 64;
  }
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_Dioptase_DioptaseTARGETTRANSFORMINFO_H
