# Vernam Cipher (5-bit XOR)

This project is a C implementation of a modified Vernam cipher. It encrypts and decrypts text files by applying a bitwise XOR operation to the 5-bit binary representation of alphabetic characters. The tool features standardized POSIX command-line flags and an optional verbose mode to trace the binary transformations step-by-step.

## Features

* **5-Bit XOR Logic:** Projects A-Z into a 0-25 numeric range, applies XOR with the key, and maps the 0-31 result back to valid ASCII characters (A-Z plus `[`, `\`, `]`, `^`, `_`, and `` ` ``) to guarantee mathematically reversible encryption.
* **Standard CLI Flags:** Uses `getopt` for flexible, order-independent argument parsing.
* **Verbose Tracing:** View the character-to-binary conversions and XOR mathematical outputs in real-time.
* **Safe Character Handling:** Non-alphabetic characters (spaces, newlines, punctuation) are ignored by the cipher and passed directly to the output file unaltered.

## Compilation

Compile the source code using standard GCC (or any compatible C compiler):

```bash
gcc vernam.c -o vernam
```

## Usage

The program requires input and output files, a mode of operation, and an alphabetical key. The arguments can be passed in any order.

```bash
./vernam -m <c|d> -i <input_file> -o <output_file> -k <key> [-v]
```

## Command-Line Flags

| Flag | Argument | Description |
| --- | --- | --- |
| `-m` | `c` or `d` | Mode: Set to `c` to encrypt (cifrar) or `d` to decrypt (descifrar). |
| `-i` | `<file>` | Input: The path to the source file you want to process. |
| `-o` | `<file>` | Output: The path where the processed text will be saved. Must be different from the input file. |
| `-k` | `<string>` | Key: The string used for the XOR operation. Must contain only alphabetical characters (A-Z). |
| `-v` | None | Verbose: (Optional) Prints the step-by-step binary XOR operation to the console. |