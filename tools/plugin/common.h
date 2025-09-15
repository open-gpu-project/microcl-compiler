#pragma once

#include "llvm/Passes/PassPlugin.h"

namespace microcl::plugin {

   /**
    * @brief
    * Struct to hold global options for the LLVM passes in this plugin.
    */
   struct GlobalLLVMOpts {
      bool InjectOurPasses = true;
   };

   llvm::PassPluginLibraryInfo GetPassPluginInfo(GlobalLLVMOpts opts = {});
}
