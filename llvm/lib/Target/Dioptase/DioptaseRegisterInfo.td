class DioptaseReg<bits<5> enc, string n> : Register<n> {
  let Namespace = "Dioptase";
  let HWEncoding{4-0} = enc;
}

foreach i = 1...31 in
  def R#i : DioptaseReg<i, "r"#i>;


let isConstant = true in {
  def R0 : DioptaseReg<0, "r"#i>;
}

def IntRegs : RegisterClass<"Dioptase", [i32], 32, (add (sequence "R%u", 1, 31))>;
