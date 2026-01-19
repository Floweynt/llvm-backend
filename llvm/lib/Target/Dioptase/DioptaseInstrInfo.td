include "DioptaseInstrFormats.td"

def ADD : DioptaseInstrALU<0b00000, "add $ra, $rb, $rc",
    (outs IntRegs:$rc), 
    (ins IntRegs:$ra, IntRegs:$rb), 
    [(set i32:$rc, (add i32:$ra, i32:$rb))]
>;

def RET_GLUE : SDNode<"DioptaseISD::RET_GLUE", SDTNone,
                 [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

let isReturn = 1, isTerminator = 1, isBarrier = 1 in {
  def RET : DioptaseInstrBr<0b00000, "ret", []>;
}
