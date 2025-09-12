#include "microcl/llvm/Transforms/PrintAnnotatedFunctions.h"

#include "Utils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "microcl/plugin/Attributes.h"

using namespace llvm;
using microcl::llvm::PrintAnnotatedFunctionsPass;

PreservedAnalyses PrintAnnotatedFunctionsPass::run(Module &M, ModuleAnalysisManager &AM) {
   if (auto *GA = M.getNamedGlobal("llvm.global.annotations")) {
      if (auto *CA = dyn_cast<ConstantArray>(GA->getOperand(0))) {
         for (unsigned i = 0; i < CA->getNumOperands(); ++i) {
            auto *CS = cast<ConstantStruct>(CA->getOperand(i));
            auto *F = cast<Function>(CS->getOperand(0)->stripPointerCasts());
            auto *AnnoGV = cast<GlobalVariable>(CS->getOperand(1)->stripPointerCasts());
            auto *Anno = cast<ConstantDataArray>(AnnoGV->getInitializer());
            auto Name = Anno->getAsCString().data();
            if (isAnnot<plugin::GpuDeviceAttrInfo>(Name)) {
               llvm::errs() << "[[GPU]]" << " " << F->getName() << "\n";
            } else if (isAnnot<plugin::DriverDeviceAttrInfo>(Name)) {
               llvm::errs() << "[[Driver]]" << " " << F->getName() << "\n";
            }
         }
      }
   }

   return PreservedAnalyses::all();
}
