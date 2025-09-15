set(SRCDIR ${LLVM_MAIN_SRC_DIR}/clang/lib/CIR)
get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
# set(dialect_libs "")

# Add CIR header compilation targets
add_subdirectory(
   ${LLVM_MAIN_SRC_DIR}/clang/include/clang/CIR/Dialect
   ${CMAKE_BINARY_DIR}/external/include/clang/CIR/Dialect
)
add_subdirectory(
   ${LLVM_MAIN_SRC_DIR}/clang/include/clang/CIR/Interfaces
   ${CMAKE_BINARY_DIR}/external/include/clang/CIR/Interfaces
)

# Ensure that the CIR generated headers are available
include_directories(${CMAKE_BINARY_DIR}/external/include)

# Add dummy target to make CIR compile
add_custom_target(MLIRSymbolInterfacesIncGen)
add_custom_target(MLIRBuiltinLocationAttributesIncGen)

# Add CIR targets, the main one is the CodeGen target "clangCIR"
# Then, we also add (minimally) the dependencies of it
add_subdirectory(
   ${LLVM_MAIN_SRC_DIR}/clang/lib/CIR/CodeGen
   ${CMAKE_BINARY_DIR}/external/clang/lib/CIR/CodeGen
)
add_subdirectory(
   ${LLVM_MAIN_SRC_DIR}/clang/lib/CIR/Dialect
   ${CMAKE_BINARY_DIR}/external/clang/lib/CIR/Dialect
)
add_subdirectory(
   ${LLVM_MAIN_SRC_DIR}/clang/lib/CIR/Interfaces
   ${CMAKE_BINARY_DIR}/external/clang/lib/CIR/Interfaces
)

target_include_directories(
   TargetLowering PRIVATE
   ${SRCDIR}/Dialect/Transforms/TargetLowering
)

# Now, we remove unnecessary dependencies from the main target clangCIR as these
# are dynamically loaded by the clang/LLVM plugin system
remove_target_dependencies_recursive(clangCIR "^LLVM.*$")
remove_target_dependencies_recursive(clangCIR "^clang.*$")
remove_target_dependencies_recursive(clangCIR "LLVMSupport")
# list_target_linked_libraries(clangCIR)
