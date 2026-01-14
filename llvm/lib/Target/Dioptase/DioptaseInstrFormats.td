class DioptaseInstr<string asmstr, dag outs, dag ins, list<dag> pattern>
   : Instruction {
  field bits<32> Inst;
  bits<5> Opc;
  let Inst{31-27} = Opc;

  let Namespace = "Dioptase";
  let Size = 4;

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString   = asmstr;
  let Pattern = pattern;

  let DecoderNamespace = "Dioptase";
}

class DioptaseInstrALU<bits<5> aluOpc, string asmstr, dag outs, dag ins, list<dag> pattern> 
    : DioptaseInstr<asmstr, outs, ins, pattern> {
  bits<5> ra;
  bits<5> rb;
  bits<5> rc;
  let Opc = 0;
  let Inst{9-5} = aluOpc;
  let Inst{4-0} = rc;
  let Inst{21-17} = rb;
  let Inst{26-22} = ra;
}


