#pragma once

#include <string_view>
#include <type_traits>

#include "clang/Sema/Sema.h"

namespace microcl::plugin {

   /*
      Why so complicated? Because of the constraints:
      - GetAttrName() should be consteval
      - Implementaion of (.*)AttrBase should be in the cpp file for clarity, so no templates
      - Single source of truth for the attribute name
      - Name should be attached to the class for easy refactor and renaming
   */

   class NamedAttrBase;

   /**
    * @brief Gets the proper annotation name for the given attribute class
    *
    * @tparam T The attribute class, must derive from NamedAttrBase
    */
   template <typename T>
      requires(std::is_base_of_v<NamedAttrBase, T>)
   consteval std::string_view GetAttrName() {
      static_assert(false, "you must specify a template specialization for this function");
      return "";
   }

   /**
    * @brief
    * Base class for all attribute base classes to inherit from
    */
   class NamedAttrBase : public clang::ParsedAttrInfo {
   protected:
      virtual std::string_view Name() const = 0;
   };

   /**
    * @brief
    * Base class for device-specifier attributes
    */
   class DeviceAttrBase : public NamedAttrBase {
   protected:
      DeviceAttrBase() { OptArgs = 0; }

   public:
      bool diagAppertainsToDecl(clang::Sema &S, const clang::ParsedAttr &Attr,
                                const clang::Decl *D) const override;
      bool diagAppertainsToStmt(clang::Sema &S, const clang::ParsedAttr &Attr,
                                const clang::Stmt *St) const override;
      bool diagMutualExclusion(clang::Sema &S, const clang::ParsedAttr &,
                               const clang::Decl *D) const override {
         // This attribute is mutually exclusive with all other CommonDeviceAttrInfo types
         return true;
      }

      AttrHandling handleDeclAttribute(clang::Sema &S, clang::Decl *D,
                                       const clang::ParsedAttr &Attr) const override;
      AttrHandling handleStmtAttribute(clang::Sema &S, clang::Stmt *St,
                                       const clang::ParsedAttr &Attr,
                                       clang::Attr *&) const override {
         // This attribute cannot be applied to statements
         return AttributeNotApplied;
      }
   };

   /**
    * @brief
    * CRTP mixin to create a NamedAttrBase with the correct Name() implementation
    *
    * @tparam Base The base class, must derive from NamedAttrBase
    * @tparam Derived The derived class that is being implemented
    */
   template <typename Base, typename Derived>
      requires(std::is_base_of_v<NamedAttrBase, Base>)
   class NamedAttrMixin : public Base {
   public:
      std::string_view Name() const override { return GetAttrName<Derived>(); }
   };

   class GpuDeviceAttrInfo : public NamedAttrMixin<DeviceAttrBase, GpuDeviceAttrInfo> {
   public:
      GpuDeviceAttrInfo();
   };

   class DriverDeviceAttrInfo : public NamedAttrMixin<DeviceAttrBase, DriverDeviceAttrInfo> {
   public:
      DriverDeviceAttrInfo();
   };

} // namespace microcl::plugin

template <>
consteval std::string_view microcl::plugin::GetAttrName<microcl::plugin::GpuDeviceAttrInfo>() {
   return "microcl-device";
}

template <>
consteval std::string_view microcl::plugin::GetAttrName<microcl::plugin::DriverDeviceAttrInfo>() {
   return "microcl-runtime";
}
