#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace mlir {
   class ModuleOp;
}

#include "clang/AST/ASTConsumer.h"
#include "clang/Basic/DiagnosticFrontend.h"
#include "clang/CIR/CIRGenerator.h"
#include "clang/CIR/CIRToCIRPasses.h"
#include "clang/CIR/FrontendAction/CIRGenAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "common.h"
#include "llvm/Passes/PassPlugin.h"
#include "microcl/plugin/Attributes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"

#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h> // dladdr
#include <limits.h>
#include <unistd.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <stringapiset.h>
#include <windows.h>
#endif

//===----------------------------------------------------------------------===//
// Global state for this plugin DSO
//===----------------------------------------------------------------------===//

static bool g_DisableLLVMPasses = false;

//===----------------------------------------------------------------------===//
// Helper function to get the path of this plugin DSO
//===----------------------------------------------------------------------===//

static std::string &_cachedDSOPath() {
   // Must be in same TU as PluginASTAction so the symbol lives in this DSO.
   static std::string path;
   return path;
}

static void *thisAddressAnchor() {
   // Any non-inline function defined in this DSO works as an anchor symbol.
   return (void *)&_cachedDSOPath;
}

static std::string getPluginPathFromThisDSO() {
   static std::once_flag once;
   std::call_once(once, [] {
#if defined(__linux__) || defined(__APPLE__)
      Dl_info info{};
      if (dladdr(thisAddressAnchor(), &info) && info.dli_fname) {
         _cachedDSOPath() = info.dli_fname; // absolute or realpath-ish on most libdl impls
      } else {
         _cachedDSOPath().clear();
      }
#elif defined(_WIN32)
      HMODULE hMod = nullptr;
      if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                   GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                             reinterpret_cast<LPCSTR>(thisAddressAnchor()), &hMod)) {
         // Get wide path
         std::wstring wbuf(256, L'\0');
         for (;;) {
            DWORD n = GetModuleFileNameW(hMod, wbuf.data(), static_cast<DWORD>(wbuf.size()));
            if (n == 0) {
               wbuf.clear();
               break;
            }
            if (n < wbuf.size() - 1) {
               wbuf.resize(n);
               break;
            }
            wbuf.resize(wbuf.size() * 2);
         }
         // Wide → UTF-8
         if (!wbuf.empty()) {
            int need = WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), static_cast<int>(wbuf.size()),
                                           nullptr, 0, nullptr, nullptr);
            std::string utf8(need, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), static_cast<int>(wbuf.size()), utf8.data(),
                                need, nullptr, nullptr);
            _cachedDSOPath() = std::move(utf8);
         } else {
            _cachedDSOPath().clear();
         }
      } else {
         _cachedDSOPath().clear();
      }
#else
      _cachedDSOPath().clear();
#endif
   });

   return _cachedDSOPath();
}

//===----------------------------------------------------------------------===//
// Clang FE plugin declaration and implementation
//===----------------------------------------------------------------------===//

using namespace clang;

namespace microcl {

   class MainAction;
   class MainConsumer;

} // namespace microcl

class microcl::MainConsumer : public ASTConsumer {
public:
   explicit MainConsumer(CompilerInstance &CI, std::unique_ptr<raw_pwrite_stream> OS)
         : CI(CI),
           OutputStream(std::move(OS)),
           FS(&CI.getVirtualFileSystem()),
           Gen(std::make_unique<cir::CIRGenerator>(CI.getDiagnostics(), std::move(FS),
                                                   CI.getCodeGenOpts())),
           FEOptions(CI.getFrontendOpts()),
           CGO(CI.getCodeGenOpts()) {}

   void Initialize(ASTContext &Ctx) override {
      assert(!Context && "initialized multiple times");
      Context = &Ctx;
      Gen->Initialize(Ctx);
   }

   bool HandleTopLevelDecl(DeclGroupRef D) override {
      Gen->HandleTopLevelDecl(D);
      return true;
   }

   void HandleCXXStaticMemberVarInstantiation(clang::VarDecl *VD) override {
      Gen->HandleCXXStaticMemberVarInstantiation(VD);
   }

   void HandleInlineFunctionDefinition(FunctionDecl *D) override {
      Gen->HandleInlineFunctionDefinition(D);
   }

   void HandleTranslationUnit(ASTContext &C) override {
      using namespace cir;

      // Generate CIR for the entire translation unit.
      Gen->HandleTranslationUnit(C);

      // Verify the generated CIR module before running any CIR passes.
      if (!Gen->verifyModule()) {
         llvm::report_fatal_error(
               "CIR codegen: module verification error before running CIR passes");
         return;
      }

      // Setup and run CIR pipeline.
      mlir::ModuleOp MlirModule = Gen->getModule();

      // Output the final CIR module.
      if (OutputStream && MlirModule) {
         mlir::OpPrintingFlags Flags;
         Flags.enableDebugInfo(/*enable=*/true, /*prettyForm=*/false);
         MlirModule->print(*OutputStream, Flags);
      }
   }

   void HandleTagDeclDefinition(TagDecl *D) override {
      PrettyStackTraceDecl CrashInfo(D, SourceLocation(), Context->getSourceManager(),
                                     "CIR generation of declaration");
      Gen->HandleTagDeclDefinition(D);
   }

   void HandleTagDeclRequiredDefinition(const TagDecl *D) override {
      Gen->HandleTagDeclRequiredDefinition(D);
   }

   void CompleteTentativeDefinition(VarDecl *D) override { Gen->CompleteTentativeDefinition(D); }

   void HandleVTable(CXXRecordDecl *RD) override { Gen->HandleVTable(RD); }

private:
   CompilerInstance &CI;
   std::unique_ptr<raw_pwrite_stream> OutputStream;
   ASTContext *Context{nullptr};
   IntrusiveRefCntPtr<llvm::vfs::FileSystem> FS;
   std::unique_ptr<cir::CIRGenerator> Gen;
   const FrontendOptions &FEOptions;
   CodeGenOptions &CGO;
};

class microcl::MainAction : public PluginASTAction {
protected:
   std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef InFile) override {
      std::filesystem::path OutPath{InFile.str()};
      OutPath.replace_extension("mlir");
      // llvm::errs() << "Writing CIR output to '" << OutPath.filename() << "\n";
      std::error_code EC;
      // auto OS = std::make_unique<llvm::raw_fd_ostream>(OutPath.filename().string(), EC, llvm::sys::fs::OF_None);
      auto OS = CI.createOutputFile(OutPath.filename().string(), false, false, false);
      return std::make_unique<MainConsumer>(CI, std::move(OS));
   }

   bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &Args) override {
      // Tell the backend to load this plugin as a pass plugin
      auto &CO = const_cast<CompilerInstance &>(CI).getCodeGenOpts();
      CO.PassPlugins.push_back(getPluginPathFromThisDSO());

      // Now parse the command line args
      for (const auto &arg : Args) {
         if (arg == "help") {
            llvm::errs() << "Not implemented\n";
            return false;
         } else if (arg == "disable-llvm-passes") {
            // Disable all our LLVM passes (do not inject them into the pipeline)
            g_DisableLLVMPasses = true;
         } else {
            llvm::errs() << "warning: ignoring unknown option '" << arg << "' for microCL plugin\n";
            // continue processing other args
         }
      }

      return true;
   }

   ActionType getActionType() override { return ActionType::AddBeforeMainAction; }
};

//===----------------------------------------------------------------------===//
// Clang FE plugin registration
//===----------------------------------------------------------------------===//

static FrontendPluginRegistry::Add<microcl::MainAction> ACT0{
      "microcl",
      "compile a file with annotated microCL code",
};

static ParsedAttrInfoRegistry::Add<microcl::plugin::GpuDeviceAttrInfo> ATTR0{
      microcl::plugin::GetAttrName<microcl::plugin::GpuDeviceAttrInfo>(),
      "",
};

static ParsedAttrInfoRegistry::Add<microcl::plugin::DriverDeviceAttrInfo> ATTR1{
      microcl::plugin::GetAttrName<microcl::plugin::DriverDeviceAttrInfo>(),
      "",
};

//===----------------------------------------------------------------------===//
// LLVM pass plugin registration
//===----------------------------------------------------------------------===//

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
   return microcl::plugin::GetPassPluginInfo({.InjectOurPasses = !g_DisableLLVMPasses});
}
