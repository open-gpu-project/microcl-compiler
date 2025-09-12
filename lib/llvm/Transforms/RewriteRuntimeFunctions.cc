#include "microcl/llvm/Transforms/RewriteRuntimeFunctions.h"

#include "Utils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "microcl/plugin/Attributes.h"

using namespace llvm;
using microcl::llvm::RewriteRuntimeFunctionsPass;

PreservedAnalyses RewriteRuntimeFunctionsPass::run(Module &M, ModuleAnalysisManager &AM) {
   if (auto *GA = M.getNamedGlobal("llvm.global.annotations")) {
      if (auto *CA = dyn_cast<ConstantArray>(GA->getOperand(0))) {
         for (unsigned i = 0; i < CA->getNumOperands(); ++i) {
            auto *CS = cast<ConstantStruct>(CA->getOperand(i));
            auto *Fn = cast<Function>(CS->getOperand(0)->stripPointerCasts());
            auto *AnnoGV = cast<GlobalVariable>(CS->getOperand(1)->stripPointerCasts());
            auto *Anno = cast<ConstantDataArray>(AnnoGV->getInitializer());
            handleFunctionAnnotation(*Fn, Anno->getAsCString().data());
         }
      }
   }

   return PreservedAnalyses::none();
}

void RewriteRuntimeFunctionsPass::handleFunctionAnnotation(Function &F, std::string_view Annot) {
   if (isAnnot<plugin::GpuDeviceAttrInfo>(Annot)) {
      // TODO
   } else if (isAnnot<plugin::DriverDeviceAttrInfo>(Annot)) {
      // TODO
   }
}
