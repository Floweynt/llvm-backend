include "DioptaseInstrFormats.td"

def simm12 : PatLeaf<(imm), [{ return isInt<12>(N->getSExtValue()); }]>;

def simm13Op : Operand<iPTR> {
  let OperandType = "OPERAND_IMMEDIATE";
  let DecoderMethod = "DecodeSIMM12";
  let EncoderMethod = "getSImm12OpValue";
}

def ADD : DioptaseInstrALU<0b00000, "add $ra, $rb, $rc",
    (outs IntRegs:$rc), 
    (ins IntRegs:$ra, IntRegs:$rb), 
    [(set i32:$rc, (add i32:$ra, i32:$rb))]
>;

def RET_GLUE : SDNode<"DioptaseISD::RET_GLUE", SDTNone,
                 [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def SWA : DioptaseMemWord<0>;
def LWA : DioptaseMemWord<1>;

def BR : DioptaseInstrBr<0b00000, "br", []>;

def PSEUDO_RET : Pseudo<"", (outs), (ins), [(RET_GLUE)]>, PseudoInstExpansion<(BR R0, R29)>;
