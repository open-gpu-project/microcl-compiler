cmake_minimum_required(VERSION 3.20.0)
project(microcl-compiler LANGUAGES CXX C)

# Set global CMake options
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ambiguous-reversed-operator -Wno-deprecated-this-capture")

# Check for required properties
if(NOT DEFINED LLVM_MAIN_SRC_DIR)
   message(FATAL_ERROR "You must define LLVM_MAIN_SRC_DIR")
endif()
if(NOT DEFINED LLVM_BUILD_DIR)
   message(FATAL_ERROR "You must define LLVM_BUILD_DIR")
endif()

# Setup so find_package(...) points to our LLVM build
set(LLVM_DIR "${LLVM_BUILD_DIR}/lib/cmake/llvm")
set(Clang_DIR "${LLVM_BUILD_DIR}/lib/cmake/clang")
set(MLIR_DIR "${LLVM_BUILD_DIR}/lib/cmake/mlir")
set(CLANG_TABLEGEN_EXE "${LLVM_BUILD_DIR}/bin/clang-tblgen")

# Find the required LLVM package
find_package(Clang REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)
find_package(MLIR REQUIRED CONFIG)

# Setup so include(...) points to LLVM's CMake modules
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${CLANG_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")

# Include the LLVM CMake modules
include(TableGen)
include(AddLLVM)
include(AddClang)
include(AddMLIR)
include(HandleLLVMOptions)

# Include directories provided by find_package(...)
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

# Include our CMake scripts
include(${CMAKE_CURRENT_LIST_DIR}/scripts/Utils.cmake)

# Include CIR targets cherry-picked from LLVM/Clang/MLIR
include(${CMAKE_CURRENT_LIST_DIR}/scripts/CIRTargets.cmake)

# Include our plugin sources
include(${CMAKE_CURRENT_LIST_DIR}/lib/build.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/tools/build.cmake)
