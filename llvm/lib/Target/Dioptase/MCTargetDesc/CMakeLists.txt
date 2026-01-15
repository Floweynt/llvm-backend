add_llvm_component_library(LLVMDioptaseDesc
  DioptaseMCTargetDesc.cpp
  DioptaseMCAsmInfo.cpp  
  DioptaseInstPrinter.cpp 
  
  LINK_COMPONENTS
  CodeGenTypes
  MC
  Support
  TargetParser
  DioptaseInfo

  ADD_TO_COMPONENT
  Dioptase
  )

