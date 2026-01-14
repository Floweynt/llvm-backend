add_llvm_component_library(LLVMDioptaseInfo
  DioptaseTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  Dioptase
  )

