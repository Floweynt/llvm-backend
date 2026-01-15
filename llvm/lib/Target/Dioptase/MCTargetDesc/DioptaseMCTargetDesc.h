//===-- DioptaseMCTargetDesc.h - Dioptase Target Descriptions -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides Dioptase specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIOPTASE_MCTARGETDESC_DIOPTASEMCTARGETDESC_H
#define LLVM_LIB_TARGET_DIOPTASE_MCTARGETDESC_DIOPTASEMCTARGETDESC_H

#include "llvm/ADT/SmallVector.h"
#include <cstdint>
#include <memory>
#include <string>

namespace llvm {
class formatted_raw_ostream;
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInst;
class MCInstPrinter;
class MCInstrInfo;
class MCObjectStreamer;
class MCObjectTargetWriter;
class MCObjectWriter;
class MCRegister;
class MCRegisterInfo;
class MCStreamer;
class MCSubtargetInfo;
class MCTargetOptions;
class MCTargetStreamer;
class Target;
class Triple;
class StringRef;

/// Flavour of dwarf regnumbers
///
namespace DWARFFlavour {
  enum {
    Dioptase_64 = 0, Dioptase_32_DarwinEH = 1, Dioptase_32_Generic = 2
  };
}

///  Native Dioptase register numbers
///
namespace N86 {
  enum {
    EAX = 0, ECX = 1, EDX = 2, EBX = 3, ESP = 4, EBP = 5, ESI = 6, EDI = 7
  };
}

namespace Dioptase_MC {
std::string ParseDioptaseTriple(const Triple &TT);

unsigned getDwarfRegFlavour(const Triple &TT, bool isEH);

void initLLVMToSEHAndCVRegMapping(MCRegisterInfo *MRI);


/// Returns true if this instruction has a LOCK prefix.
bool hasLockPrefix(const MCInst &MI);

/// \param Op operand # of the memory operand.
///
/// \returns true if the specified instruction has a 16-bit memory operand.
bool is16BitMemOperand(const MCInst &MI, unsigned Op,
                       const MCSubtargetInfo &STI);

/// \param Op operand # of the memory operand.
///
/// \returns true if the specified instruction has a 32-bit memory operand.
bool is32BitMemOperand(const MCInst &MI, unsigned Op);

/// \param Op operand # of the memory operand.
///
/// \returns true if the specified instruction has a 64-bit memory operand.
#ifndef NDEBUG
bool is64BitMemOperand(const MCInst &MI, unsigned Op);
#endif

/// Returns true if this instruction needs an Address-Size override prefix.
bool needsAddressSizeOverride(const MCInst &MI, const MCSubtargetInfo &STI,
                              int MemoryOperand, uint64_t TSFlags);

/// Create a Dioptase MCSubtargetInfo instance. This is exposed so Asm parser, etc.
/// do not need to go through TargetRegistry.
MCSubtargetInfo *createDioptaseMCSubtargetInfo(const Triple &TT, StringRef CPU,
                                          StringRef FS);

void emitInstruction(MCObjectStreamer &, const MCInst &Inst,
                     const MCSubtargetInfo &STI);

void emitPrefix(MCCodeEmitter &MCE, const MCInst &MI, SmallVectorImpl<char> &CB,
                const MCSubtargetInfo &STI);
}

MCCodeEmitter *createDioptaseMCCodeEmitter(const MCInstrInfo &MCII,
                                      MCContext &Ctx);

MCAsmBackend *createDioptase_32AsmBackend(const Target &T,
                                     const MCSubtargetInfo &STI,
                                     const MCRegisterInfo &MRI,
                                     const MCTargetOptions &Options);
MCAsmBackend *createDioptase_64AsmBackend(const Target &T,
                                     const MCSubtargetInfo &STI,
                                     const MCRegisterInfo &MRI,
                                     const MCTargetOptions &Options);

/// Implements Dioptase-only directives for assembly emission.
MCTargetStreamer *createDioptaseAsmTargetStreamer(MCStreamer &S,
                                             formatted_raw_ostream &OS,
                                             MCInstPrinter *InstPrinter);

/// Implements Dioptase-only directives for object files.
MCTargetStreamer *createDioptaseObjectTargetStreamer(MCStreamer &S,
                                                const MCSubtargetInfo &STI);

/// Construct an Dioptase Windows COFF machine code streamer which will generate
/// PE/COFF format object files.
///
/// Takes ownership of \p AB and \p CE.
MCStreamer *createDioptaseWinCOFFStreamer(MCContext &C,
                                     std::unique_ptr<MCAsmBackend> &&AB,
                                     std::unique_ptr<MCObjectWriter> &&OW,
                                     std::unique_ptr<MCCodeEmitter> &&CE);

MCStreamer *createDioptaseELFStreamer(const Triple &T, MCContext &Context,
                                 std::unique_ptr<MCAsmBackend> &&MAB,
                                 std::unique_ptr<MCObjectWriter> &&MOW,
                                 std::unique_ptr<MCCodeEmitter> &&MCE);

/// Construct an Dioptase Mach-O object writer.
std::unique_ptr<MCObjectTargetWriter>
createDioptaseMachObjectWriter(bool Is64Bit, uint32_t CPUType, uint32_t CPUSubtype);

/// Construct an Dioptase ELF object writer.
std::unique_ptr<MCObjectTargetWriter>
createDioptaseELFObjectWriter(bool IsELF64, uint8_t OSABI, uint16_t EMachine);
/// Construct an Dioptase Win COFF object writer.
std::unique_ptr<MCObjectTargetWriter>
createDioptaseWinCOFFObjectWriter(bool Is64Bit);

/// \param Reg speicifed register.
/// \param Size the bit size of returned register.
/// \param High requires the high register.
///
/// \returns the sub or super register of a specific Dioptase register.
MCRegister getDioptaseSubSuperRegister(MCRegister Reg, unsigned Size,
                                  bool High = false);
} // End llvm namespace


// Defines symbolic names for Dioptase registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "DioptaseGenRegisterInfo.inc"

// Defines symbolic names for the Dioptase instructions.
//
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "DioptaseGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "DioptaseGenSubtargetInfo.inc"

#endif

