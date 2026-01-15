def RetCC_Dioptase : CallingConv<[
    CCIfType<[i32], CCAssignToReg<[R1]>>
]>;

def CC_Dioptase : CallingConv<[
    CCIfType<[i32], CCAssignToReg<[R1, R2, R3, R4, R5, R6, R7, R8]>>
]>;


def CSR : CalleeSavedRegs<(add R20, R21, R22, R23, R24, R25, R26, R27)>;
