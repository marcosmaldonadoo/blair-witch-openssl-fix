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

On 2026-10-19, the user also confirmed that, after following the instructions to remove the previous OpenSSL launch command and click Steam's **Play** button, the game opened and its menu worked normally. This is user-reported manual confirmation on the same installation.

Together, these checks verify startup through the original launcher and Steam on one installation. They do not verify a full playthrough, all audio APIs, other Windows versions, or other game builds.

## Short in-game interaction check

On 2026-10-19, a further UI check launched the shipping executable with Steam running, entered the main menu, and selected **New Game**. In the opening car sequence, pressing Tab brought up the phone and a left mouse click answered the fictional in-game call. The scene updated in response to both inputs. Escape opened the pause menu, where the game was left paused.

This verifies rendering and basic keyboard/mouse interaction in one short opening sequence. It is not a full playthrough, an audio-listening test, or a performance benchmark. A local backup of existing profile/input save files was made before starting the test; no existing campaign save was present in the inspected SaveGames folder.

Existing display settings on the test installation had already been adjusted before this mod was developed. This mod does not modify display settings.
