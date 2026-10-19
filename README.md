# Blair Witch OpenSSL Fix

A small Windows x64 compatibility mod for **Blair Witch (2019)**. Install one DLL and launch the game with Steam's **Play** button, without a batch file or custom launch command.

[Português brasileiro](README.pt-BR.md) · [Download](https://github.com/marcosmaldonadoo/blair-witch-openssl-fix/releases) · [Validation](VALIDATION.md)

[Detailed installation and troubleshooting guide (Brazilian Portuguese)](INSTALL.pt-BR.md)

## Install

1. Close the game.
2. Download the ZIP from **Releases** and extract it.
3. In Steam, open **Blair Witch → Properties → Installed Files → Browse**.
4. Open `Blairwitch\Binaries\Win64` and place **`winmm.dll`** next to `Blairwitch-Win64-Shipping.exe`.
5. In **Properties → General → Launch Options**, remove the previous `cmd /c ... OPENSSL_ia32cap ...` workaround. Keep any unrelated options you need.
6. Click **Play** in Steam.

If there is already a `winmm.dll` in that game folder, do not overwrite it: another mod may be using it. This release does not implement mod chaining.

The game executable and Windows files are not replaced. To uninstall, close the game and delete only this mod's `winmm.dll` from the game's `Win64` folder. Steam file verification may leave added mod files in place.

## What it fixes

Some older OpenSSL versions crash when selecting SHA instruction routines on affected CPUs. Intel documents the issue in older Unreal Engine games and the workaround `OPENSSL_ia32cap=:~0x20000000`:

[Intel: OpenSSL SHA Crash Bug Requires Application Update](https://www.intel.com/content/www/us/en/developer/articles/troubleshooting/openssl-sha-crash-bug-requires-application-update.html)

This mod applies that same workaround inside the game process. It changes no persistent user or system environment variables. An existing nonempty `OPENSSL_ia32cap` value is respected.

It addresses this startup problem. Display resolution, mouse alignment, gameplay bugs, and crashes with other causes need their own fixes.

## How it works

The tested Steam executable imports `WINMM.dll` at startup. Windows loads this local proxy from the executable's directory. On process attachment, the proxy recognizes the two Blair Witch shipping executable names and updates both the Windows environment and the **UCRT environment table** used by the game's `getenv`.

Updating only the Windows environment proved insufficient in the native test: UCRT may already have cached it. A direct import of `_putenv_s` ensures UCRT is initialized before the proxy's entry point, so the cached table is updated before executable initialization.

The proxy preserves 181 export slots, including the ordinal-only export. API calls are forwarded to the original `winmm.dll` loaded from an absolute Windows system-directory path. Export resolution is lazy and synchronized; it does not load libraries from `DllMain`. The x64 forwarding stubs preserve integer/vector argument registers and stack arguments.

Only the mod's original code is distributed. No game executable, OpenSSL library, or Windows DLL is included.

## Build and test

Requirements: Windows x64, PowerShell 5.1 or newer, and an x86_64 MinGW-w64 GCC toolchain with UCRT import libraries. GCC 14 or newer is recommended for the test script's `-mcrtdll=ucrt` option.

```powershell
.\build.ps1
.\test.ps1
```

If GCC is not on `PATH`:

```powershell
.\build.ps1 -Compiler 'C:\msys64\ucrt64\bin\gcc.exe'
.\test.ps1 -Compiler 'C:\msys64\ucrt64\bin\gcc.exe'
```

Output: `build\winmm.dll`. Copy only this DLL into the game, not the smoke-test executables. The proxy imports Kernel32 and the UCRT environment API already used by the tested game; it needs no GCC DLL runtime.

Tests cover both recognized executable names, an unrelated executable, preservation of an existing mask, all 181 exports, concurrent first calls, and forwarding of timer/audio query functions, including six-argument calls. They also check that `getenv` sees the value before `main`.

## Compatibility

Initially tested with Steam build **5340689** on Windows 11 x64. The alternate `-Win7` executable name is recognized, but Windows 7 itself has not been tested. Other storefronts, other builds, and combinations with other DLL mods are unverified.

The user also confirmed startup through Steam's **Play** button and a working menu after following the instructions to remove the old launch workaround on 2026-10-19.

See [VALIDATION.md](VALIDATION.md) for the exact scope of testing. A successful startup check does not establish full playthrough stability.

## License

MIT. This is an unofficial community compatibility mod, with no affiliation with the game's developers or publishers, Intel, Microsoft, or Valve.
