# Profile
Development CLI to track todos, fixmes, and more

## What This Tool Does
This tool tracks FIXMEs and TODOs in your project codebases and prints a
summary to your standard output. A sample output could look like the following:

```txt
TODO Found:
File: "foo/bar.cpp"
Line Number: 144
Line:   std::size_t length_; // TODO: Change Type

Files Profiled: 1212
TODOs Found: 1
FIXMEs Found: 0
```

This tool recursively profiles every file in the directory you specify, even
files in other sub-directories! By default, these diagnostics print to standard
output. However, if you are working in a large codebase, you can easily pipe
this output to a file:

```zsh
Profile path/to/directory > log.txt
```

Any resulting errors are printed to standard error so you can easily see if
something fails without digging into the log file (usually occurs when trying
to profile a locked file without admin or sudo priviledges).

## How to Use
Simply call the executable and give it a path on the command line like so:

```zsh
Profile path/to/dir
```

When built on Windows, backslash notation also works:

```zsh
Profile.exe path\to\dir
```

If no path is specified, the current directory is chosen by default. That means

```zsh
Profile
```

is equivalent to

```zsh
Profile .
```

## Compiling From Source
This is a cross platform CLI using CMake. In its current state, everything use
the C++20 standard library to ensure easy portability. Simply create your build
directory by typing the following in the Profile root directory:

```zsh
cmake -S . -B build
```

and move to the build directory and build the program:

```zsh
cd build
cmake --build . --config Release
```

You can then install the program to your machine's binary folder with:

```zsh
sudo --install .
```

If you are on windows, you will need to do this with admin priviledges. Also
note that on Windows, you may need to add the <code>Profile</code> directory
(created in your Program Files directory) to your path.
