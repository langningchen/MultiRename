[![wakatime](https://wakatime.com/badge/github/langningchen/MultiRename.svg)](https://wakatime.com/badge/github/langningchen/MultiRename.git)

# MultiRename

## Introduction

Rename multiple files on Windows with a pattern.

## Usage

1. Clone this repository
2. Make sure you have `gcc` and `cmake` installed.
3. Run `cmake -B build` and `cmake --build build` to build the project.
4. Run `build\MultiRename.exe` to rename files.

## Notes

Rename rules:
- `[F]`: Full file name (e.g. "file.txt")
- `[.]`: File extension dot (if file has extension, then "."; otherwise, "")
- `[E]`: File extension (e.g. "txt")
- `[N]`: File name, same as "[F][.][E]" (e.g. "file")
- `[D]`: File directory (e.g. "Folder")
- `[N:a~b]` `[F:a~b]` `[E:a~b]` `[D:a~b]` `[P:a~b]`: File name from a to b, if a or b is less than 0, then count from the end of the string (e.g. `[N1~3]`="fil"   `[N1~-2]`="fi"   `[N-3~-1]`="ile")
- `[S]`: File size in bytes (e.g. "1024")
- `[T]` `[C]` `[W]` `[A]`: Current time or file last create/write/access time (e.g. "2019-01-01 00:00:00")
- `[T]` `[C:format]` `[W:format]` `[A:format]`: Current time or file last create/write/access time with format, format is the same as strftime (e.g. "2019-01-01 00:00:00")
- `[[]`: Character '['
- `[]]`: Character ']'
- `[U]`: Random UUID (e.g. "123e4567-e89b-12d3-a456-426655440000")
- `[I]`: File index
- `[I:Start,Step,Digits]`: File index with start, step and digits

## License

This project is licensed under the terms of the GNU General Public License v3.0.
