# Semestral Work

- CMakeLists.txt - CMake source file
- main.hpp - main function definition
- Lexan.hpp, Lexan.cpp - Lexan related sources
- Parser.hpp, Parser.cpp - Parser related sources
- fce.c - grue for write, writeln, read function, it is compliled together with the program
- samples - directory with samples desribing syntax
- mila - wrapper script for your compiler
- test - test script with comiples all samples

## Literature ##

LLVM supplies a set of tutorials which is available here: https://llvm.org/docs/tutorial/

## Dependencies ##

LLVM including headers. Bases on your OS distribution, it would be usually packages like `llvm`, `llvm-dev`.

## Build ##

```
mkdir build &&
cd build &&
cmake ..
make
```

**To rebuild:**
```
cd build &&
make
```

## Test samples ##

```
./test
```

## Compile a program ##
```
./mila test.mila -o test
```

**How does mila wrapper script works?**

It runs `build/sfe` on the source code, then `llc` and `clang` (with the fce.c file added):

```
rm -f "$OutputFileBaseName.ir"
> "$OutputFileBaseName.ir" < "$InputFileName" ${DIR}/build/sfe &&
rm -f "$OutputFileBaseName.s"
llc "$OutputFileBaseName.ir" -o "$OutputFileBaseName.s" &&
clang "$OutputFileBaseName.s" "${DIR}/fce.c" -o "$OutputFileName"
```

## Compiler requirements ##
Compiler process source code supplied on the stdin and produces LLVM ir on its stdout.
All errors are should be written to the stdin, non zero return code should be return in case of error.
No arguments are required, but the mila wrapper is prepared for -v/--verbose, -d/--debug options which can be passed to the compiler.
Other arguments can be also added for various purposes.

## Template status ##
Regardless of the source code supplied, all produced binaries gives "Answer to the Ultimate Question of Life, the Universe, and Everything":
```
42
```