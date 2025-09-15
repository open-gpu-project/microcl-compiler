add_library(
   microcl-lib

   STATIC
   ${CMAKE_CURRENT_LIST_DIR}/llvm/Transforms/RewriteRuntimeFunctions.cc
   ${CMAKE_CURRENT_LIST_DIR}/llvm/Transforms/PrintAnnotatedFunctions.cc
   ${CMAKE_CURRENT_LIST_DIR}/plugin/DeviceAttributes.cc
)

target_link_libraries(microcl-lib PRIVATE
   ${LLVM_LIBS}
)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
