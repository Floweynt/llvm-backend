//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIOPTASE_DIOPTASESELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_DIOPTASE_DIOPTASESELECTIONDAGINFO_H

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"

#define GET_SDNODE_ENUM
#include "DioptaseGenSDNodeInfo.inc"

namespace llvm {
namespace DioptaseISD {

enum NodeType : unsigned {
  GLOBAL_BASE_REG = GENERATED_OPCODE_END, // Global base reg for PIC.
};

} // namespace DioptaseISD

class DioptaseSelectionDAGInfo : public SelectionDAGGenTargetInfo {
public:
  DioptaseSelectionDAGInfo();

  ~DioptaseSelectionDAGInfo() override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_DIOPTASE_DIOPTASESELECTIONDAGINFO_H
