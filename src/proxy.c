#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "exports.generated.h"

/* Match the UCRT getenv used by the game, without a separate CRT startup. */
__declspec(dllimport) int __cdecl _putenv_s(const char *, const char *);
static INIT_ONCE initialized = INIT_ONCE_STATIC_INIT;
static FARPROC addresses[EXPORT_COUNT];

static BOOL CALLBACK initialize_exports(PINIT_ONCE once, PVOID parameter,
                                       PVOID *context)
{
    WCHAR path[MAX_PATH + 16];
    static const WCHAR suffix[] = L"\\winmm.dll";
    UINT length = GetSystemDirectoryW(path, MAX_PATH);
    HMODULE original;
    unsigned int i;
    (void)once;
    (void)parameter;
    (void)context;
    if (length == 0 || length >= MAX_PATH) return FALSE;
    for (i = 0; i < sizeof(suffix) / sizeof(suffix[0]); ++i)
        path[length + i] = suffix[i];
    /* An absolute path prevents resolving our own proxy as the real DLL. */
    original = LoadLibraryExW(path, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (original == NULL) return FALSE;
    for (i = 0; i < EXPORT_COUNT; ++i) {
        LPCSTR symbol = export_names[i];
        if (symbol == NULL) symbol = MAKEINTRESOURCEA(export_ordinals[i]);
        addresses[i] = GetProcAddress(original, symbol);
    }
    /* Keep the system DLL loaded for the lifetime of this process. */
    return TRUE;
}

FARPROC resolve_export(unsigned int index)
{
    if (index >= EXPORT_COUNT ||
        !InitOnceExecuteOnce(&initialized, initialize_exports, NULL, NULL) ||
        addresses[index] == NULL) {
        /* Never jump to a null address or continue with a missing API. */
        RaiseException(0xC0000139UL, EXCEPTION_NONCONTINUABLE, 0, NULL);
        TerminateProcess(GetCurrentProcess(), ERROR_PROC_NOT_FOUND);
        for (;;) Sleep(INFINITE);
    }
    return addresses[index];
}

static BOOL is_game_process(void)
{
    static WCHAR path[32768];
    static const WCHAR *const names[] = {
        L"Blairwitch-Win64-Shipping.exe",
        L"Blairwitch-Win64-Shipping-Win7.exe"
    };
    DWORD length = GetModuleFileNameW(NULL, path, 32768);
    DWORD offset, i, j;
    if (length == 0 || length >= 32768) return FALSE;
    offset = length;
    while (offset > 0 && path[offset - 1] != L'\\' && path[offset - 1] != L'/')
        --offset;
    for (i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        for (j = 0; ; ++j) {
            WCHAR left = path[offset + j], right = names[i][j];
            if (left >= L'A' && left <= L'Z') left += L'a' - L'A';
            if (right >= L'A' && right <= L'Z') right += L'a' - L'A';
            if (left != right) break;
            if (left == L'\0') return TRUE;
        }
    }
    return FALSE;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        WCHAR existing[2];
        DisableThreadLibraryCalls(instance);
        /* Only touch the target game and honor an existing explicit mask. */
        if (is_game_process() &&
            GetEnvironmentVariableW(L"OPENSSL_ia32cap", existing, 2) == 0) {
            if (!SetEnvironmentVariableW(L"OPENSSL_ia32cap", L":~0x20000000"))
                return FALSE;
            /* Windows' environment and the CRT's cached table are distinct.
             * This direct import makes UCRT initialize before our DllMain.
             * No LoadLibrary, thread creation, or file I/O occurs here. */
            return _putenv_s("OPENSSL_ia32cap", ":~0x20000000") == 0;
        }
    }
    return TRUE;
}
