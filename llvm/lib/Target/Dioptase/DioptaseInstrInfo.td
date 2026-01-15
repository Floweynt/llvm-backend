include "DioptaseInstrFormats.td"

def ADD : DioptaseInstrALU<0b00000, "add $ra, $rb, $rc",
    (outs IntRegs:$rc), 
    (ins IntRegs:$ra, IntRegs:$rb), 
    [(set i32:$rc, (add i32:$ra, i32:$rb))]
>;

def ret : SDNode<"DioptaseISD::RET", SDTNone,
                 [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
