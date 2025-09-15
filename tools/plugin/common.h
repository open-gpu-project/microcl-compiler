#pragma once

#include "llvm/Passes/PassPlugin.h"

namespace microcl::plugin {

   /**
    * @brief
    * Struct to hold global options for the LLVM passes in this plugin.
    */
   struct GlobalLLVMOpts {
      bool InjectOurPasses = false; /// Whether to inject our passes into the pipeline automatically.
   };

   llvm::PassPluginLibraryInfo GetPassPluginInfo(GlobalLLVMOpts opts = {});
}
