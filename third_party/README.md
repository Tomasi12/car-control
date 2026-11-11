# third_party — project-local build environment

Everything needed to build `dummy_project`, so nothing has to be on the system PATH.
`build.ps1` puts `third_party/mingw64/bin` first on PATH automatically.

| Folder | What | Version | Source | License |
|---|---|---|---|---|
| `mingw64/` | WinLibs MinGW-w64 toolchain (UCRT, POSIX threads) | GCC 14.2.0, LLVM/Clang 19.1.1, MinGW-w64 12.0.0 r2 | https://github.com/brechtsanders/winlibs_mingw (via winget `BrechtSanders.WinLibs.POSIX.UCRT.LLVM`) | GPL/LGPL/Apache-2.0 (per tool) |

Main tools in `mingw64/bin/`: `g++`, `gcc`, `clang++`, `clang-tidy`, `clangd`, `cppcheck`,
`cmake` (3.30.4), `ninja`, `mingw32-make`, `gdb`.

About 1.5 GB — keep it out of Git (`.gitignore` excludes it).

## Use it in a terminal
```powershell
$env:PATH = "$PWD\third_party\mingw64\bin;$env:PATH"
```

## Get the toolchain after cloning
`mingw64/` is not in Git. Download it once:
1. Download `winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-19.1.1-mingw-w64ucrt-12.0.0-r2.zip` from
   https://github.com/brechtsanders/winlibs_mingw/releases/tag/14.2.0posix-19.1.1-12.0.0-ucrt-r2
2. Extract it so that `third_party/mingw64/bin/g++.exe` exists.

(Or `winget install BrechtSanders.WinLibs.POSIX.UCRT.LLVM` and copy its `mingw64` folder here.)
