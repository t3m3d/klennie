# ktop

A terminal process viewer written in [Krypton](https://github.com/t3m3d/krypton). Shows live CPU%, memory, and process info. Works on Windows and Linux.

## Requirements

- [Krypton compiler (kcc)](https://github.com/t3m3d/krypton) — cloned alongside this repo at `../krypton/`
- GCC — TDM-GCC or MinGW-w64 on Windows, system gcc on Linux

## Building

### Windows

```bat
build.bat
```

Compiles `ktop.exe`, installs it to `C:\krypton\bin\`, and launches it.

First time only — add `C:\krypton\bin` to your PATH (admin cmd):

```bat
setx /M PATH "%PATH%;C:\krypton\bin"
```

Link flags used: `-lpsapi -lpdh -lm`

### Linux

```bash
bash build.sh
```

Produces `./ktop` in the current directory.

## Running

```
ktop [--sort cpu|mem|pid|name] [--refresh ms] [--no-color]
```

| Key | Action |
|-----|--------|
| `c` | Sort by CPU |
| `m` | Sort by memory |
| `p` | Sort by PID |
| `n` | Sort by name |
| Up / Down | Scroll one row |
| PgUp / PgDn | Scroll one page |
| `q` | Quit |

## Source files

| File | Purpose |
|------|---------|
| `run.k` | Entry point, arg parsing, main loop |
| `process.k` | Process collection and sorting |
| `sysinfo.k` | Uptime, memory, load average |
| `ui.k` | Terminal drawing (ANSI escape codes) |
| `utils.k` | String utilities |
| `platform.h` | C shim — keyboard, terminal size, sleep, process snapshot (Windows + Linux) |
| `platform.krh` | Krypton declarations for `platform.h` |
| `build.bat` | Windows build script |
| `build.sh` | Linux build script |
