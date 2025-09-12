#include "microcl/plugin/Attributes.h"

using namespace clang;
using namespace microcl::plugin;
using AttrHandling = ParsedAttrInfo::AttrHandling;
using ParsedAttr = clang::ParsedAttr;

// This attribute appertains to function declarations only.
bool DeviceAttrBase::diagAppertainsToDecl(Sema &S, const ParsedAttr &Attr, const Decl *D) const {
   if (!isa<FunctionDecl>(D)) {
      S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
            << Attr << Attr.isRegularKeywordAttribute() << ExpectedFunction;
      return false;
   }
   return true;
}

// This attribute does not appertain to any statements.
bool DeviceAttrBase::diagAppertainsToStmt(Sema &S, const ParsedAttr &Attr, const Stmt *St) const {
   S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
         << Attr << Attr.isRegularKeywordAttribute() << ExpectedFunction;
   return false;
}

// Handle applying to declaration
AttrHandling DeviceAttrBase::handleDeclAttribute(Sema &S, Decl *D, const ParsedAttr &Attr) const {
   // Check if the decl is at file scope.
   if (!D->getDeclContext()->isFileContext()) {
      unsigned ID = S.getDiagnostics().getCustomDiagID(
            DiagnosticsEngine::Error, "'cl-*' attributes only allowed at file scope");
      S.Diag(Attr.getLoc(), ID);
      return AttributeNotApplied;
   }
   // Attach the attribute
   D->addAttr(clang::AnnotateAttr::Create(S.Context, Name(), nullptr, 0, Attr.getRange()));
   return AttributeApplied;
}

GpuDeviceAttrInfo::GpuDeviceAttrInfo() {
   static constexpr Spelling S[] = {{ParsedAttr::AS_GNU, "microcl_gpu"},
                                    {ParsedAttr::AS_C23, "microcl_gpu"},
                                    {ParsedAttr::AS_CXX11, "microcl::gpu"}};
   Spellings = S;
}

DriverDeviceAttrInfo::DriverDeviceAttrInfo() {
   static constexpr Spelling S[] = {{ParsedAttr::AS_GNU, "microcl_driver"},
                                    {ParsedAttr::AS_C23, "microcl_driver"},
                                    {ParsedAttr::AS_CXX11, "microcl::driver"}};
   Spellings = S;
}
