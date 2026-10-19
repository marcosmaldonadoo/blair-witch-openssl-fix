# Validation

Initial local validation: **2026-10-19**.

## Native tests

Built with MinGW-w64 GCC 16.1.0 targeting Windows x64. Smoke-test executables use UCRT, matching the `getenv` import observed in the game.

All four cases passed:

| Executable | Inherited mask | Expected result |
| --- | --- | --- |
| `Blairwitch-Win64-Shipping.exe` | Absent | `:~0x20000000` visible to Windows and UCRT before `main` |
| `Blairwitch-Win64-Shipping-Win7.exe` | Absent | Same mask applied |
| `other-app.exe` | Absent | Variable remains absent |
| `Blairwitch-Win64-Shipping.exe` | `:~0x12345678` | Existing value preserved |

Each case also verifies the local proxy is actually loaded, the presence of all 181 exports, 16 concurrent first callers, timer APIs, audio-device counts, and a six-argument audio-format query compared with the original system DLL. The tests query formats without playing audio.

## Game startup

Game: Blair Witch, Steam app 1092660, build 5340689; Windows 11 x64.

The original `Blairwitch.exe` bootstrap launcher was started with `OPENSSL_ia32cap` removed from its child environment. No batch wrapper was used. The shipping process loaded both the local proxy and the Windows system `winmm.dll`, created a game window, and was responding when inspected after 15 seconds.

This verifies startup via the original launcher on one installation. It does not verify a full playthrough, visual rendering, all audio APIs, other Windows versions, or other game builds. Steam's Play button with the old launch options removed still needs a separate manual check.

Existing display settings on the test installation had already been adjusted before this mod was developed. This mod does not modify display settings.
