//===-- Dioptase.td - Describe the Dioptase Target Machine -------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

include "DioptaseRegisterInfo.td"
include "DioptaseCallingConv.td"
include "DioptaseInstrInfo.td"

defm : RemapAllTargetPseudoPointerOperands<IntRegs>;

def DioptaseInstrInfo : InstrInfo {}

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"dioptase", []>;

def DioptaseAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  int PassSubtarget = 1;
  int Variant = 0;
}

def Dioptase : Target {
  // Pull in Instruction Info:
  let InstructionSet = DioptaseInstrInfo;
  // let AssemblyParsers = [DioptaseAsmParser];
  // let AssemblyWriters = [DioptaseAsmWriter];
  let AllowRegisterRenaming = 1;
}

