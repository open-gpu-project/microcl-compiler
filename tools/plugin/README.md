## Plugins Description

This directory contains plugins for both `clang` and `opt` (the LLVM optimizer tool). The plugin functions are as follows:

- `microcl-plugin` will have the full-featured compilation flow
   - include the LLVM transformation passes, as well as the CIR and MLIR transformation passes
- `opt-plugin` will only have LLVM transformation passes, so you can test each pass with the opt tool
   - this is because clang doesn't let you pick and choose which optimization passes to run

## Build targets

The build targets (found in the CMake build directory) is as follows:
- **clang** plugin is `microcl-plugin`
- **opt** plugin is `opt-plugin`

## Usage

Your VSCode environment should already have the following environment variables declared (echo just to make sure):

```bash
# Check to make sure VSCode has set it up properly
echo $MICROCL_BIN
echo $LLVM_BIN
echo $UCL_CLANG
echo $UCL_OPT

# Create your aliases
alias ucl_clang $UCL_CLANG
alias ucl_opt $UCL_OPT

# Double check if it works
ucl_clang --version
ucl_opt --version
```

If the previous step doesn't work, setup (in bash syntax)

```bash
export LLVM_BIN "/path/to/llvm/build/bin"
export MICROCL_BIN "/path/to/microcl/build/bin"
alias ucl_clang "$LLVM_BIN/clang++ -fplugin=$MICROCL_BIN/microcl-plugin.so"
alias ucl_opt "$LLVM_BIN/opt --load-pass-plugin=$MICROCL_BIN/opt-plugin.so"
```

where you should replace `.so` with `.dylib` on MacOS and `.dll` on Windows.

### Example: Compiling with the clang plugin

Run this command and you should see the resulting LLVM `.ll` file

```bash
ucl_clang -S -emit-llvm tests/examples/simple.cc
```

### Example: Using plugin arguments

Clang supports passing arguments to plugins via the `-fplugin-arg-microcl-*` option. For example, let's print out all the optimization passes in clang:
```bash
ucl_clang -mllvm -print-pipeline-passes -c -x c /dev/null -o /dev/null
```
The output should contain our `RewriteRuntimeFunctionsPass` transformation (as well as other passes)
```
verify,...,microcl::llvm::RewriteRuntimeFunctionsPass,...
```
But sometimes you may want to compile without ANY microCL LLVM passes. To do so, you can pass in the `disable-llvm-passes` option like this:
```bash
ucl_clang -mllvm -print-pipeline-passes -c -x c /dev/null -o /dev/null -fplugin-arg-microcl-disable-llvm-passes
```
and the output should not contain any `microcl::llvm` passes.

### Example: Using opt plugin

First, you need to compile it to the `.ll` file:
```bash
ucl_clang -fplugin-arg-microcl-disable-llvm-passes -S -emit-llvm tests/examples/simple.cc
```

This will emit a `.ll` file from the compiled C++ file. Then you can run a pass, for example let's print out the annotated functions:
```bash
ucl_opt --passes="microcl-print-annotated-functions" simple.ll -S > /dev/null
```
See the LLVM opt user manual for more details.
