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

class DioptaseInstrBr<bits<5> cc, string asmstr, list<dag> pattern> 
    : DioptaseInstr<asmstr # " $out, $target", (outs IntRegs:$out), (ins IntRegs:$target), pattern> {
  bits<5> out;
  bits<5> target;
  let Opc = 0b01101;
  let Inst{9-5} = out;
  let Inst{4-0} = target;
  let Inst{26-22} = cc;
}

class DioptaseMemWord<bits<1> mode> : DioptaseInstr<"swa $ra, [$rb], $imm12", (outs IntRegs:$ra), (ins IntRegs:$rb, simm13Op:$imm12), []> {
  bits<12> imm12;
  bits<5> rb;
  bits<5> ra;

  let Opc = 0b00011;
  let Inst{16} = mode;
  let Inst{11-0} = imm12;
  let Inst{26-22} = ra;
  let Inst{21-17} = rb;
}

class Pseudo<string asmstr, dag outs, dag ins, list<dag> pattern>
     : DioptaseInstr<asmstr, outs, ins, pattern> {
   let isPseudo = 1; 
   let isCodeGenOnly = 1;
   let Size = 4;
 }
