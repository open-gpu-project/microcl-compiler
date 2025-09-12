#pragma once

#include <string_view>
#include "llvm/IR/PassManager.h"

namespace microcl::llvm {

   using namespace ::llvm;

   /**
    * @brief
    * This pass should go through all annotated runtime and device functions
    * and rewrite their bodies to call the host-side stubs. These stubs will be
    * linked against the runtime library when the device code is compiled.
    */
   class RewriteRuntimeFunctionsPass : public PassInfoMixin<RewriteRuntimeFunctionsPass> {
   public:
      PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

   private:
      void handleFunctionAnnotation(Function &F, std::string_view Annotation);
   };

} // namespace microcl::llvm
