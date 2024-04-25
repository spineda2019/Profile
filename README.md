# Profile
Development CLI to track todos, fixmes, and more!

## What This Tool Does
This tool tracks FIXMEs, TODOs, HACKS, and more important comments in your
project codebases and prints a summary to your standard output. 

## Currently Supported Filetypes
| Comment Format | Language           |
| -------------- | ------------------ |
| "//"           | C                  |
| "//"           | C++                |
| "//"           | C Headers          |
| "//"           | C++ Headers (.hpp) |
| "//"           | Javascript         |
| "//"           | Rust               |
| "//"           | Typescript         |
| "//"           | Zig                |
| "//"           | C#                 |
| "#"            | Python             |

Unrecognized file types are simply skipped over and do not effect the state of
the program. This way, all of your config files, txt test files, or whatever
else useful "non-code" you may have lying in your codebase will not effect this
program!

## Currently Supported Special Comments
| Comment |
| ------- |
| TODO    |
| HACK    |
| FIXME   |
| BUG     |

These special comments will always be caught and reported to you. You may also
pass custom regexes to Profile and these will <em>also</em> be searched for in
file comments and reported like anything else. Examples on how to do so are
below.

## Examples
A sample output could look like the following:

```txt
Profile -l

TODO Found:
File: "foo/bar.cpp"
Line Number: 144
Line:   std::size_t length_; // TODO: Change Type

Files Profiled: 1212
TODOs Found: 1
FIXMEs Found: 0
BUGs Found: 0
HACKs Found: 0

File Extension     |Files               
--------------------------------------------------------------------------------
.hpp               |2                   
--------------------------------------------------------------------------------
.cpp               |3                   
--------------------------------------------------------------------------------
```

This tool recursively profiles every file in the directory you specify, even
files in other sub-directories! You can have these diagnostics print to standard
output by specifying the <code>-l</code> flag. If you are working in a large
codebase, you can easily pipe this output to a file:

```zsh
Profile -d path/to/directory -l > log.txt
```

Any resulting errors are printed to standard error so you can easily see if
something fails without digging into the log file (usually occurs when trying
to profile a locked file without admin or sudo priviledges).

## How to Use
Simply call the executable and give it a path on the command line like so:

```zsh
Profile -d path/to/dir
```

Remember that the <code>-l</code> or <code>--log</code> flag enables reporting
of where keywords are found in your codebase!

```zsh
Profile -d path/to/dir -l
```

When built on Windows, backslash notation also works:

```zsh
Profile.exe -d path\to\dir
```

If no path is specified, the current directory is chosen by default. That means

```zsh
Profile
```

is equivalent to

```zsh
Profile -d .
```

For other flags, see the help message:

```zsh
Profile -h
```

## Custom Search Regexes
By using the <code>-c</code> or <code>--custom</code> flag, you can specify
custom desired regexes to search for in file comments! Here is what it may look
like:


```txt
Profile -l -c foo

TODO Found:
File: "foo/bar.cpp"
Line Number: 144
Line:   std::size_t length_; // TODO: Change Type

Regex foo Found:
File: "foo/bar.cpp"
Line Number: 145
Line:   std::size_t width; // Here's a foo!

Files Profiled: 1212
TODOs Found: 1
FIXMEs Found: 0
BUGs Found: 0
HACKs Found: 0

------------------------------------ Customs -----------------------------------
Amount of foo Found: 1

File Extension     |Files               
--------------------------------------------------------------------------------
.hpp               |2                   
--------------------------------------------------------------------------------
.cpp               |3                   
--------------------------------------------------------------------------------
```

you can even pass in as many regexes as you please. Just reuse the same flag:

```txt
Profile -l -c foo -c bar
```

## Compiling From Source
This is a cross platform CLI using CMake. In its current state, everything use
the C++20 standard library to ensure easy portability. Simply create your build
directory by typing the following in the Profile root directory:

```zsh
cmake -S . -B build
```

and build the program:

```zsh
cd build
cmake --build build
```

pass whichever cmake flags you would like. I like using these for release builds:

```zsh
cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS="-O3" -DCMAKE_BUILD_TYPE=Release
```

You can then install the program to your machine's binary folder with:

```zsh
cmake --install build
```

If you are on windows, you will need to do this with admin priviledges. Also
note that on Windows, you may need to add the <code>Profile</code> directory
(created in your Program Files directory) to your path. Be sure to use a single
configuration cmake generator, otherwise these flags won't do anything (using
the default Visual Studio generator requires this for example): 

```zsh
cmake -S . -B build
cmake --build build --config Release
```
