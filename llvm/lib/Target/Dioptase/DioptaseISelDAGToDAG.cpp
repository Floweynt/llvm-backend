//===-- DioptaseISelDAGToDAG.cpp - Minimal DAG->DAG ISel
//--------------------===//
//
// Minimal instruction selector for the Dioptase target.
// Relies entirely on TableGen-generated patterns from InstrInfo.td.
//
//===----------------------------------------------------------------------===//

#include "Dioptase.h"
#include "DioptaseTargetMachine.h"

#include "MCTargetDesc/DioptaseMCTargetDesc.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "dioptase-isel"

using namespace llvm;

namespace {

class DioptaseDAGToDAGISel : public SelectionDAGISel {
public:
  DioptaseDAGToDAGISel(DioptaseTargetMachine &TM, CodeGenOptLevel OptLevel)
      : SelectionDAGISel(TM, OptLevel) {}

  void Select(SDNode *N) override;

  // TableGen-generated instruction selector
#include "DioptaseGenDAGISel.inc"
};

class DioptaseDAGToDAGISelLegacy : public SelectionDAGISelLegacy {
public:
  static char ID;
  DioptaseDAGToDAGISelLegacy(DioptaseTargetMachine &TM,
                             CodeGenOptLevel OptLevel)
      : SelectionDAGISelLegacy(
            ID, std::make_unique<DioptaseDAGToDAGISel>(TM, OptLevel)) {}
};

} // end anonymous namespace

char DioptaseDAGToDAGISelLegacy::ID = 0;

INITIALIZE_PASS(DioptaseDAGToDAGISelLegacy, DEBUG_TYPE,
                "Dioptase DAG->DAG Instruction Selection", false, false)

void DioptaseDAGToDAGISel::Select(SDNode *N) {
  // If already selected, nothing to do.
  if (N->isMachineOpcode()) {
    N->setNodeId(-1);
    return;
  }

  // Defer entirely to TableGen patterns.
  SelectCode(N);
}

FunctionPass *llvm::createDioptaseISelDag(DioptaseTargetMachine &TM,
                                          CodeGenOptLevel OptLevel) {
  return new DioptaseDAGToDAGISelLegacy(TM, OptLevel);
}
