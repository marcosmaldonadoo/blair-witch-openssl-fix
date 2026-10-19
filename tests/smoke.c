#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exports.generated.h"

#define REQUIRE(condition, message) do { if (!(condition)) { \
    fprintf(stderr, "FAIL: %s\n", message); return 1; } } while (0)

static DWORD WINAPI concurrent_call(LPVOID ignored)
{
    unsigned int i;
    (void)ignored;
    for (i = 0; i < 1000; ++i) (void)timeGetTime();
    return 0;
}

int main(int argc, char **argv)
{
    char value[512], module_path[MAX_PATH], expected_path[MAX_PATH];
    WCHAR system_path[MAX_PATH + 16];
    const char *crt_value;
    DWORD length;
    HANDLE threads[16];
    TIMECAPS caps, original_caps;
    WAVEFORMATEX format = { WAVE_FORMAT_PCM, 2, 44100, 176400, 4, 16, 0 };
    HMODULE original;
    UINT directory_length, i;
    MMRESULT (WINAPI *original_caps_fn)(LPTIMECAPS, UINT);
    UINT (WINAPI *original_count_fn)(void);
    MMRESULT (WINAPI *original_open_fn)(LPHWAVEOUT, UINT, LPCWAVEFORMATEX,
                                      DWORD_PTR, DWORD_PTR, DWORD);
    REQUIRE(argc == 2, "expected one argument: expected mask or ABSENT");
    length = GetEnvironmentVariableA("OPENSSL_ia32cap", value, sizeof(value));
    crt_value = getenv("OPENSSL_ia32cap");
    if (strcmp(argv[1], "ABSENT") == 0) {
        REQUIRE(length == 0 && crt_value == NULL, "non-game process changed");
    } else {
        REQUIRE(length > 0 && length < sizeof(value), "missing Windows mask");
        REQUIRE(strcmp(value, argv[1]) == 0, "incorrect Windows mask");
        REQUIRE(crt_value && strcmp(crt_value, argv[1]) == 0,
                "CRT getenv did not see mask before main");
    }
    REQUIRE(GetModuleFileNameA(GetModuleHandleA("winmm.dll"), module_path,
                              MAX_PATH) > 0, "proxy not loaded");
    REQUIRE(GetFullPathNameA("winmm.dll", MAX_PATH, expected_path, NULL) > 0,
            "could not resolve proxy path");
    REQUIRE(lstrcmpiA(module_path, expected_path) == 0, "system DLL bypassed proxy");
    for (i = 0; i < EXPORT_COUNT; ++i) {
        LPCSTR symbol = export_names[i];
        if (symbol == NULL) symbol = MAKEINTRESOURCEA(export_ordinals[i]);
        REQUIRE(GetProcAddress(GetModuleHandleA("winmm.dll"), symbol) != NULL,
                "missing proxy export");
    }
    /* Exercise the lazy resolver concurrently on the very first API call. */
    for (i = 0; i < 16; ++i) {
        threads[i] = CreateThread(NULL, 0, concurrent_call, NULL, 0, NULL);
        REQUIRE(threads[i] != NULL, "CreateThread failed");
    }
    REQUIRE(WaitForMultipleObjects(16, threads, TRUE, 10000) == WAIT_OBJECT_0,
            "concurrent resolver failed or hung");
    for (i = 0; i < 16; ++i) CloseHandle(threads[i]);
    directory_length = GetSystemDirectoryW(system_path, MAX_PATH);
    REQUIRE(directory_length > 0 && directory_length < MAX_PATH, "system path");
    lstrcpyW(system_path + directory_length, L"\\winmm.dll");
    original = LoadLibraryExW(system_path, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    REQUIRE(original != NULL, "original system DLL unavailable");
    /* FARPROC is generic; memcpy avoids incompatible function pointer casts. */
    {
        FARPROC symbol = GetProcAddress(original, "timeGetDevCaps");
        memcpy(&original_caps_fn, &symbol, sizeof(symbol));
        symbol = GetProcAddress(original, "waveOutGetNumDevs");
        memcpy(&original_count_fn, &symbol, sizeof(symbol));
        symbol = GetProcAddress(original, "waveOutOpen");
        memcpy(&original_open_fn, &symbol, sizeof(symbol));
    }
    REQUIRE(original_caps_fn && original_count_fn && original_open_fn,
            "missing original exports");
    REQUIRE(timeGetDevCaps(&caps, sizeof(caps)) == TIMERR_NOERROR,
            "timeGetDevCaps failed");
    REQUIRE(original_caps_fn(&original_caps, sizeof(original_caps)) == TIMERR_NOERROR,
            "original timeGetDevCaps failed");
    REQUIRE(memcmp(&caps, &original_caps, sizeof(caps)) == 0,
            "argument or return value changed");
    REQUIRE(waveOutGetNumDevs() == original_count_fn(), "device count changed");
    REQUIRE(timeBeginPeriod(1) == TIMERR_NOERROR, "timeBeginPeriod failed");
    REQUIRE(timeEndPeriod(1) == TIMERR_NOERROR, "timeEndPeriod failed");
    REQUIRE(waveOutOpen(NULL, WAVE_MAPPER, &format, 0, 0, WAVE_FORMAT_QUERY) ==
            original_open_fn(NULL, WAVE_MAPPER, &format, 0, 0, WAVE_FORMAT_QUERY),
            "six-argument forwarding failed");
    printf("PASS: %s; environment visible before main; proxy and system APIs agree\n",
           argv[1]);
    return 0;
}
