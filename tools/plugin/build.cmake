add_llvm_library(
    microcl-plugin
   
    MODULE
    ${CMAKE_CURRENT_LIST_DIR}/microcl-plugin.cc

    PLUGIN_TOOL
    clang
)
target_link_libraries(microcl-plugin PRIVATE microcl-lib)
