#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// #include <SDL2/SDL.h>
#include "SDL.h"

#include "resources.h"

#ifndef _WIN32
#include <sys/time.h>
#endif

#ifdef _WIN32
#include "windows.h"
#include <tlhelp32.h>

// Keyboard hook

#ifdef _WIN32

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

HHOOK hHook;
int modkeystate[255];

BOOL bLWinKeyDown = 0;
BOOL bRWinKeyDown = 0;
BOOL bAppsKeyDown = 0;

HWND tid;

void installKeyboardHook(void)
{
    printf("installKeyboardHook 13\n");
    HINSTANCE app = GetModuleHandle(NULL);

    tid = GetForegroundWindow();

    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, app, 0); // Global Only
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0 || nCode != HC_ACTION) {
        return CallNextHookEx(hHook, nCode, wParam, lParam);
    }

    if (GetForegroundWindow() != tid) {
        return CallNextHookEx(hHook, nCode, wParam, lParam);
    }

    KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;

    if (p->vkCode == VK_TAB && p->flags & LLKHF_ALTDOWN) return 1;     // disable alt-tab
    if (p->vkCode == VK_SPACE && p->flags & LLKHF_ALTDOWN) return 1;   // disable alt-tab

    if (p->vkCode == VK_LWIN) {
        bLWinKeyDown = !(p->flags & LLKHF_UP);
    }
    if (p->vkCode == VK_RWIN) {
        bRWinKeyDown = !(p->flags & LLKHF_UP);
    }
    if (p->vkCode == VK_APPS) {
        bAppsKeyDown = !(p->flags & LLKHF_UP);
    }

    if ((p->vkCode == VK_LWIN) || (p->vkCode == VK_RWIN)) {
        int key_state = !(p->flags & LLKHF_UP);

        SDL_Event event;
        event.key.keysym.scancode = SDL_SCANCODE_RGUI;
        event.key.timestamp = p->time;
        event.key.repeat = 0;
        event.type = key_state ? SDL_KEYDOWN : SDL_KEYUP;
        SDL_PushEvent(&event);

        return 1;// disable windows keys
    }

    printf("vKcode: %d - lwin: %d\n", (int)p->vkCode, (int)bLWinKeyDown);

    if ((bLWinKeyDown) && ((p->vkCode >= 'A') || (p->vkCode <= 'Z'))) {
        int key_state = !(p->flags & LLKHF_UP);

        SDL_Event event;
        event.key.keysym.sym = p->vkCode + 'a' - 'A';
        event.key.keysym.mod = KMOD_LGUI;
        event.key.repeat = 0;
        event.key.timestamp = p->time;
        event.type = key_state ? SDL_KEYDOWN : SDL_KEYUP;
        SDL_PushEvent(&event);

        return 1;
    }

    if (p->vkCode == VK_ESCAPE && p->flags & LLKHF_ALTDOWN) return 1;  // disable alt-escape
    BOOL bControlKeyDown = GetAsyncKeyState(VK_CONTROL) >> ((sizeof(SHORT) * 8) - 1);     // checks ctrl key pressed

    if (p->vkCode == VK_ESCAPE && bControlKeyDown) return 1;           // disable ctrl-escape

    return CallNextHookEx(hHook, nCode, wParam, lParam);
} /* LowLevelKeyboardProc */

/*
 * LRESULT CALLBACK LowLevelKeyboardProc(INT nCode, WPARAM wParam, LPARAM lParam)
 * {
 * // By returning a non-zero value from the hook procedure, the
 * // message does not get passed to the target window
 *  KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT *)lParam;
 *
 *  printf("wParam: %s\t", (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) ? "DOWN" : "UP");
 *
 *  printf("scanCode: %d\t", (int)pkbhs->scanCode);
 *  printf("vkCode: %d\t", (int)pkbhs->vkCode);
 *  printf("ASCII: %c\t", (int)pkbhs->vkCode);
 *  printf("FLAGS: %d\t", (int)pkbhs->flags);
 *
 *  switch (nCode) {
 *      case HC_ACTION:
 *      {
 * // Check to see if the CTRL key is pressed
 *          BOOL bControlKeyDown = GetAsyncKeyState(VK_CONTROL) >> ((sizeof(SHORT) * 8) - 1);
 *
 *
 *          BOOL bLWinKeyDown = GetAsyncKeyState(VK_LWIN) >> ((sizeof(SHORT) * 8) - 1);
 *          BOOL bRWinKeyDown = GetAsyncKeyState(VK_RWIN) >> ((sizeof(SHORT) * 8) - 1);
 *          BOOL bAppsKeyDown = GetAsyncKeyState(VK_APPS) >> ((sizeof(SHORT) * 8) - 1);
 *
 *          printf("FLAGS2: %d %d %d\n", (int)bLWinKeyDown, (int)bRWinKeyDown, (int)bAppsKeyDown);
 *
 * // Disable CTRL+ESC
 *          if (pkbhs->vkCode == VK_ESCAPE && bControlKeyDown)
 *              return 1;
 *
 * // Disable ALT+TAB
 *          if (pkbhs->vkCode == VK_TAB && pkbhs->flags & LLKHF_ALTDOWN)
 *              return 1;
 *
 * // Disable ALT+ESC
 *          if (pkbhs->vkCode == VK_ESCAPE && pkbhs->flags & LLKHF_ALTDOWN)
 *              return 1;
 *
 *          if (pkbhs->vkCode == 0x52 && (bLWinKeyDown||bRWinKeyDown||bAppsKeyDown))
 *          return 1;
 *
 *          if (bLWinKeyDown||bRWinKeyDown||bAppsKeyDown) {
 *              return 1;
 *          }
 *
 *          break;
 *      }
 *
 *      default:
 *          break;
 *  }
 *
 *  printf("\n");
 *
 *  return CallNextHookEx(hHook, nCode, wParam, lParam);
 * }
 */

#endif /* ifdef _WIN32 */

static void *resourcesCore = NULL;

void UseResources(void *core, void *bytes, int len);

BOOL EnumTypesFunc(HANDLE hModule, LPSTR lpType, LONG lParam);
BOOL EnumNamesFunc(HANDLE hModule, LPSTR lpType, LPSTR lpName, LONG lParam);
BOOL EnumLangsFunc(HANDLE hModule, LPSTR lpType, LPSTR lpName, WORD language, LONG lParam);

void listResources(void *core)
{
#ifdef _WIN32
    installKeyboardHook();  // Disable windows shortcut (alt+tab, ...)
#endif

    LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    resourcesCore = core;

    HINSTANCE hInst = GetModuleHandle(NULL);

    printf("Begin of search resources\n");

    EnumResourceTypes(hInst,
                      (FARPROC)EnumTypesFunc,
                      -1L
                      );

    printf("End of search resources\n");
}

static CHAR *pTypeName[] = {
    NULL,                               /* 0 */
    "CURSOR",                           /* 1 */
    "BITMAP",                           /* 2 */
    "ICON",                             /* 3 */
    "MENU",                             /* 4 */
    "DIALOG",                           /* 5 */
    "STRING",                           /* 6 */
    "FONTDIR",                          /* 7 */
    "FONT",                             /* 8 */
    "ACCELERATOR",                      /* 9 */
    "RCDATA",                           /* 10 */
    "MESSAGETABLE",                     /* 11 */
    "GROUP_CURSOR",                     /* 12 */
    NULL,                               /* 13 */
    "GROUP_ICON",                       /* 14 */
    NULL,                               /* 15 */
    "VERSION",                          /* 16 */
    "DLGINCLUDE"                        /* 17 */
};

BOOL EnumTypesFunc(HANDLE hModule, LPSTR lpType, LONG lParam)
{
    if (lParam != -1L) {
        printf("RCDUMP: EnumTypesFunc lParam value incorrect (%ld)\n", lParam);
    }

    printf("Type: ");
    if ((ULONG)lpType & 0xFFFF0000) {
        printf("%s\n", lpType);
    } else {
        if ((intptr_t)lpType > 17) printf("%u\n", (intptr_t)lpType);
        else printf("%s\n", pTypeName[(intptr_t)lpType]);
    }

    EnumResourceNames(hModule,
                      lpType,
                      (FARPROC)EnumNamesFunc,
                      -2L
                      );

    return TRUE;
} /* EnumTypesFunc */

BOOL EnumNamesFunc(HANDLE hModule, LPSTR lpType, LPSTR lpName, LONG lParam)
{
    if (lParam != -2L) {
        printf("RCDUMP: EnumNamesFunc lParam value incorrect (%ld)\n", lParam);
    }

    printf("    Name: ");
    if ((ULONG)lpName & 0xFFFF0000) {
        printf("%s\n", lpName);
    } else {
        printf("%u\n", (intptr_t)lpName);
    }

    EnumResourceLanguages(hModule,
                          lpType,
                          lpName,
                          (ENUMRESLANGPROCA)EnumLangsFunc,
                          -3L
                          );

    return TRUE;
} /* EnumNamesFunc */

BOOL EnumLangsFunc(HANDLE hModule, LPSTR lpType, LPSTR lpName, WORD language, LONG lParam)
{
    HANDLE hResInfo;
    PVOID pv;
    HRSRC hr;

    if (lParam != -3L) {
        printf("RCDUMP: EnumLangsFunc lParam value incorrect (%ld)\n", lParam);
    }

    printf("        Resource: ");
    if ((ULONG)lpName & 0xFFFF0000) {
        printf("%s . ", lpName);
    } else {
        printf("%u . ", (intptr_t)lpName);
    }

    if ((ULONG)lpType & 0xFFFF0000) {
        printf("%s . ", lpType);
    } else {
        if ((intptr_t)lpType > 17) printf("%u . ", (intptr_t)lpType);
        else printf("%s . ", pTypeName[(intptr_t)lpType]);
    }

    printf("%08x", language);
    hResInfo = FindResourceEx(hModule, lpType, lpName, language);
    if (hResInfo != NULL) {
        hr = LoadResource(hModule, hResInfo);
        pv = LockResource(hr);

        printf(" - hResInfo == %p,\n\t\tAddress == %p - Size == %lu\n",
               hResInfo,
               pv, SizeofResource(hModule, hResInfo)
               );

        if (!strcmp(lpType, "DSK")) {
            UseResources(resourcesCore, pv, SizeofResource(hModule, hResInfo));
        }
    }

    return TRUE;
} /* EnumLangsFunc */

#else  /* ifdef _WIN32 */

void listResources(void *core)
{
    printf("No resources (no win32)\n");
}

#endif /* ifdef _WIN32 */
