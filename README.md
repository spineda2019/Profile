# Profile
Development CLI to track todos, fixmes, and more

## Use
Simply call the executable and give it a path on the command line like so:

```zsh
Profile path/to/dir
```

When built on Windows, backslash notation also works:

```zsh
Profile.exe path\to\dir
```

## Compiling From Source
This is a cross platform CLI using CMake. Simply create your build directory:

```zsh
cmake -S . -B build
```

move to the build directory and build the program:

```zsh
cd build
cmake --build . --config Release
```