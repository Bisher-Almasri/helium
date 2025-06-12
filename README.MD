<div align="center">
<img src="Logo.svg" alt="Helium Logo" width="100"/>

# Helium
</div>

A lightweight compiler that translates a simple programming language into x86-64 assembly code.

## Features

- Simple syntax with basic programming constructs
- Support for integer literals and variables
- Variable declaration and assignment
- Program exit with return values
- Direct compilation to x86-64 assembly

## Language Syntax

The Helium language supports the following features:

```he
// Variable declaration and assignment
let x = 42;

// Program exit with return value
exit(x);
```

## Requirements

- C++17 or later
- NASM (Netwide Assembler)
- GNU Linker (ld)
- CMake 3.10 or later

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

1. Write your Helium code in a file with `.he` extension
2. Compile using the Helium compiler:
   ```bash
   ./hlc your_program.he
   ```
3. The compiler will generate an executable with the same name as your input file

## Example

Input file (`example.he`):
```he
let x = 42;
exit(x);
```

Compile and run:
```bash
./hlc example.he
./example
```

## Project Structure

- `src/` - Source code
  - `main.cpp` - Main compiler driver
  - `tokenization.hpp` - Lexical analysis
  - `parser.hpp` - Syntax analysis
  - `generation.hpp` - Code generation
