#include "common.h"

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "microcl/llvm/Transforms/PrintAnnotatedFunctions.h"
#include "microcl/llvm/Transforms/RewriteRuntimeFunctions.h"

using namespace llvm;

static microcl::plugin::GlobalLLVMOpts gOpts;

static bool ModulePipelineParsingCallback(StringRef Name, llvm::ModulePassManager &PM,
                                          ArrayRef<llvm::PassBuilder::PipelineElement>) {
   using namespace llvm;
   using namespace microcl::llvm;
   if (Name == "microcl-rewrite-runtime-functions") {
      PM.addPass(RewriteRuntimeFunctionsPass{});
   } else if (Name == "microcl-print-annotated-functions") {
      PM.addPass(PrintAnnotatedFunctionsPass{});
   } else {
      return false;
   }
   return true;
}

static void PipelineStartEPCallback(llvm::ModulePassManager &MPM, llvm::OptimizationLevel Level) {
   using namespace microcl::llvm;
   if (gOpts.InjectOurPasses) {
      // Inject passes here before the standard optimization pipelines
      MPM.addPass(RewriteRuntimeFunctionsPass{});
   }
}

llvm::PassPluginLibraryInfo microcl::plugin::GetPassPluginInfo(GlobalLLVMOpts Opts) {
   gOpts = Opts;
   return {
         LLVM_PLUGIN_API_VERSION,
         "microcl",
         "0.1",
         [](llvm::PassBuilder &PB) {
            // 1. Textual pipeline name -> pass mapping
            PB.registerPipelineParsingCallback(ModulePipelineParsingCallback);
            // 2. Auto-inject near the start of the default pipeline
            PB.registerPipelineStartEPCallback(PipelineStartEPCallback);
         },
   };
}
