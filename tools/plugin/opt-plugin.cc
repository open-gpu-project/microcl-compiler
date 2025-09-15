#include "common.h"
#include "llvm/Passes/PassPlugin.h"

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
   return microcl::plugin::GetPassPluginInfo();
}
