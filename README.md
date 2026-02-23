![Titan](https://github.com/user-attachments/assets/3fa8753b-9174-4dbf-9fd6-63fadf4a4beb)
# Titan

This is the new Titan langauge, made to be simple, to be useful for making applications, 
having a classic look, and being safe, strict and readable.
-

**Titan** aims to make a better proggramming world for coders.

**⚠ Important Note: README.md will be showing the instructions for the most recent version.**

**⚠ Important Note: This will probably only work on `Windows` computers, not `MacOS`, `Linux` or any other OS.**

**⚠ Important Note: To run Titan, please install GCC:**
> To compile and run Titan code, you need GCC installed via MSYS2.
> ## Step 1: Install MSYS2
> Go to https://www.msys2.org/.
> Download the installer (msys2-x86_64-YYYYMMDD.exe).
> Run the installer. Keep the default installation path (C:\msys64).
> Step 2: Install GCC (MinGW-w64)
> Open the "MSYS2 UCRT64" terminal from your Start Menu.
> Run the following command to update the package database:
> ```
> pacman -Syu
> ```
> If the terminal closes, reopen it and run this command to install the GCC compiler toolchain:
> ```
> pacman -S mingw-w64-ucrt-x86_64-gcc
> ```
> (Press Y to confirm installation).

# Titan features
- `Titan` compiles directly into C, then into a .EXE, making it very fast.
- `Titan` does not leave clutter in the same place where your projects are.
- Instead, it makes a folder named `titan-projects`, makes more folders with your script name, and puts the clutter there.

# Installation
> **Coming soon!**

# Starting Titan

1. Open Visual Studio Code. If you don't have it, download it [here.](https://code.visualstudio.com/)

2. Paste (or write) this code into the editor:
```
console.typeln("Hello, World!") // Prints a message (semi-colons are optional in Titan.)
```
Save it as `HelloWorld.tn` in a folder called `TitanScripts`.

# Running code:
**⚠ Important Note: Go to [Adding Titan to PATH](https://example.com) before continuing.**

After Titan is in PATH, open the command prompt and type this:
``` bash
cd Documents\TitanScripts
titan HelloWorld.tn
```
It should print something like this:
```
Titan Compiler v0.9.0
----------------------
A subdirectory or file titan_projects already exists.
Output: titan_projects\HelloWorld\HelloWorld.exe
Compiling...
Success! Running...

Hello, World!
```

If so, Well Done! You've wrote your first Titan Script.

Go to the [Titan PL Wiki]()





