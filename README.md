# microCL Compiler Toolchain

...

## Environment Setup

> :warning: **Check your disk space!** :warning: The LLVM project can take up *quite a lot of disk space* when build (~60 GiB for debug builds, on my M4 MBP). Additionally, this project takes ~5 GiB in debug builds.

**Step 1:** Make sure you have CMake, Ninja and the LLVM toolchain set up. See the instructions in [gpu-design-rtl](https://github.com/open-gpu-project/gpu-design-rtl/blob/main/README.md) for more information.

**Step 2:** Build `llvm-project`. Before you run the given commands, *see the note below*.
```bash
cd llvm-project
mkdir build
cd build
cmake -G Ninja ../llvm \
        -DLLVM_ENABLE_PROJECTS="clang;mlir" \
        -DLLVM_TARGETS_TO_BUILD="host" \
        -DLLVM_ENABLE_ASSERTIONS=ON \
        -DCMAKE_BUILD_TYPE=DEBUG
ninja
```

> :bulb: **Note** :bulb: You may need to add additional CMake arguments to force the toolchain to use clang and lld. For example:
>  ```
>  -DCMAKE_CXX_COMPILER=/path/to/clang++
>  -DCMAKE_C_COMPILER=/path/to/clang
>  -DLLVM_USE_LINKER=lld
>  ```
> It's highly recommended you build using the clang toolchain.

This will take a while. Make sure the build succeeds (warnings are OK). Also, you will only need to build LLVM once as we will be using it as a library.

**Step 3:** Copy `CMakeUserPresets.template.json` into `CMakeUserPresets.json` and populate the required fields.

**Step 4:** If using VSCode, install the required workspace extensions and restart.

## Building the Project

In VSCode, you can simply run the default build task. However, you can set up the CMake configuration from the CLI if you wish:
```bash
cmake --preset default-debug
cmake --build build
```
