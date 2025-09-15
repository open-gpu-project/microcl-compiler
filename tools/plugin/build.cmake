add_llvm_library(
    microcl-plugin
   
    MODULE
    ${CMAKE_CURRENT_LIST_DIR}/clang-plugin.cc
    ${CMAKE_CURRENT_LIST_DIR}/common.cc

    PLUGIN_TOOL
    clang
)
target_link_libraries(microcl-plugin PRIVATE microcl-lib clangCIR)

add_llvm_library(
    opt-plugin
   
    MODULE
    ${CMAKE_CURRENT_LIST_DIR}/opt-plugin.cc
    ${CMAKE_CURRENT_LIST_DIR}/common.cc

    PLUGIN_TOOL
    opt
)
target_link_libraries(opt-plugin PRIVATE microcl-lib)
