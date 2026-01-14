add_llvm_component_group(Dioptase)

set(LLVM_TARGET_DEFINITIONS Dioptase.td)

tablegen(LLVM DioptaseGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM DioptaseGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM DioptaseGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM DioptaseGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM DioptaseGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM DioptaseGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM DioptaseGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM DioptaseGenSDNodeInfo.inc -gen-sd-node-info)
tablegen(LLVM DioptaseGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM DioptaseGenCallingConv.inc -gen-callingconv)
add_public_tablegen_target(DioptaseCommonTableGen)

add_llvm_target(DioptaseCodeGen
  DioptaseSubtarget.cpp
  DioptaseTargetMachine.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  MC
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils
  DioptaseDesc
  DioptaseInfo

  ADD_TO_COMPONENT
  Dioptase
  )

add_subdirectory(TargetInfo)
