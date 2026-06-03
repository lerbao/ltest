#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <algorithm>
#include <Shlwapi.h>
#include <Tlhelp32.h>

#pragma comment(lib, "Shlwapi.lib")

#define SWITCH_INTERVAL 15000

HWND g_hWnd = NULL;
HMODULE g_mpvModule = NULL;
void* g_mpv = NULL;
std::vector<std::wstring> g_videoFiles;
CRITICAL_SECTION g_cs;

typedef void* (*mpv_create_func)(void);
typedef int (*mpv_initialize_func)(void*);
typedef void (*mpv_terminate_destroy_func)(void*);
typedef void (*mpv_set_option_string_func)(void*, const char*, const char*);
typedef void (*mpv_command_async_func)(void*, int, const char**);
typedef int (*mpv_command_func)(void*, const char**);
typedef struct mpv_event* (*mpv_wait_event_func)(void*, double);
typedef int (*mpv_get_property_func)(void*, const char*, int, void*);

#define MPV_FORMAT_INT64 6
#define MPV_EVENT_START_FILE 7

mpv_create_func mpv_create = NULL;
mpv_initialize_func mpv_initialize = NULL;
mpv_terminate_destroy_func mpv_terminate_destroy = NULL;
mpv_set_option_string_func mpv_set_option_string = NULL;
mpv_command_async_func mpv_command_async = NULL;
mpv_command_func mpv_command = NULL;
mpv_wait_event_func mpv_wait_event = NULL;
mpv_get_property_func mpv_get_property = NULL;

FILE* g_logFile = NULL;

void Log(const char* format, ...) {
    if (!g_logFile) return;
    va_list args;
    va_start(args, format);
    vfprintf(g_logFile, format, args);
    va_end(args);
    fprintf(g_logFile, "\n");
    fflush(g_logFile);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void ScanVideos();
BOOL IsWindows7OrLower();
BOOL LoadMpvLibrary();

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    g_logFile = fopen("player_log.txt", "w");
    Log("=== Video Player Start ===");

    DWORD currentProcessId = GetCurrentProcessId();
    Log("Current process ID: %lu", currentProcessId);

    while (true) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            Log("Failed to create process snapshot");
            break;
        }

        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(PROCESSENTRY32W);

        BOOL found = FALSE;
        if (Process32FirstW(hSnapshot, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, L"VideoPlayer.exe") == 0 && pe.th32ProcessID != currentProcessId) {
                    Log("Found existing VideoPlayer instance, terminating...");
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
                    if (hProcess) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                        found = TRUE;
                        Log("Terminated existing instance");
                    }
                }
            } while (Process32NextW(hSnapshot, &pe));
        }

        CloseHandle(hSnapshot);

        if (!found) {
            break;
        }

        Sleep(500);
    }

    Sleep(1000);

    InitializeCriticalSection(&g_cs);

    if (!LoadMpvLibrary()) {
        Log("Failed to load MPV library!");
        MessageBox(NULL, _T("Failed to load MPV library!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
        DeleteCriticalSection(&g_cs);
        if (g_logFile) fclose(g_logFile);
        return 1;
    }
    Log("MPV library loaded successfully");

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(BLACK_BRUSH);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = _T("VideoPlayer");
    wcex.hIconSm        = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex)) {
        Log("Window Registration Failed!");
        MessageBox(NULL, _T("Window Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
        DeleteCriticalSection(&g_cs);
        if (g_logFile) fclose(g_logFile);
        return 1;
    }
    Log("Window class registered");

    g_hWnd = CreateWindowEx(WS_EX_TOPMOST,
                           _T("VideoPlayer"),
                           _T("Fullscreen Video Player"),
                           WS_POPUP,
                           0, 0,
                           GetSystemMetrics(SM_CXSCREEN),
                           GetSystemMetrics(SM_CYSCREEN),
                           NULL, NULL, hInstance, NULL);

    if (!g_hWnd) {
        Log("Window Creation Failed!");
        MessageBox(NULL, _T("Window Creation Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
        DeleteCriticalSection(&g_cs);
        if (g_logFile) fclose(g_logFile);
        return 1;
    }
    Log("Window created, HWND: %p", g_hWnd);

    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);
    Log("Window shown");

    ScanVideos();
    if (g_videoFiles.empty()) {
        Log("No video files found!");
        MessageBox(NULL, _T("No video files found in video folder!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
        DeleteCriticalSection(&g_cs);
        if (g_logFile) fclose(g_logFile);
        return 1;
    }
    Log("Found %d video files", (int)g_videoFiles.size());
    for (size_t i = 0; i < g_videoFiles.size(); i++) {
        char path[512];
        WideCharToMultiByte(CP_UTF8, 0, g_videoFiles[i].c_str(), -1, path, 512, NULL, NULL);
        Log("  Video %d: %s", (int)i, path);
    }

    g_mpv = mpv_create();
    if (!g_mpv) {
        Log("Failed to create MPV instance!");
        MessageBox(NULL, _T("Failed to create MPV instance!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
        DeleteCriticalSection(&g_cs);
        if (g_logFile) fclose(g_logFile);
        return 1;
    }
    Log("MPV instance created: %p", g_mpv);

    char widStr[32];
    sprintf(widStr, "%lld", (LONG_PTR)g_hWnd);
    mpv_set_option_string(g_mpv, "wid", widStr);
    Log("Set wid: %s", widStr);

    mpv_set_option_string(g_mpv, "vo", "gpu");
    mpv_set_option_string(g_mpv, "hwdec", "auto");
    mpv_set_option_string(g_mpv, "input-default-bindings", "yes");
    mpv_set_option_string(g_mpv, "input-vo-keyboard", "yes");
    mpv_set_option_string(g_mpv, "osc", "no");
    mpv_set_option_string(g_mpv, "border", "no");
    mpv_set_option_string(g_mpv, "cursor-autohide", "no");
    mpv_set_option_string(g_mpv, "background-color", "#000000");
    mpv_set_option_string(g_mpv, "keep-open", "yes");
    mpv_set_option_string(g_mpv, "prefetch", "yes");
    mpv_set_option_string(g_mpv, "start", "0");
    mpv_set_option_string(g_mpv, "loop-playlist", "inf");
    mpv_set_option_string(g_mpv, "log-file", "mpv_log.txt");
    Log("MPV options set");

    if (mpv_initialize(g_mpv) < 0) {
        Log("Failed to initialize MPV!");
        MessageBox(NULL, _T("Failed to initialize MPV!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
        mpv_terminate_destroy(g_mpv);
        DeleteCriticalSection(&g_cs);
        if (g_logFile) fclose(g_logFile);
        return 1;
    }
    Log("MPV initialized");

    Log("Waiting for MPV to be ready...");
    for (int i = 0; i < 5; i++) {
        mpv_wait_event(g_mpv, 100);
    }

    if (!g_videoFiles.empty()) {
        const wchar_t* path = g_videoFiles[0].c_str();
        int utf8Len = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
        char* cpath = new char[utf8Len];
        WideCharToMultiByte(CP_UTF8, 0, path, -1, cpath, utf8Len, NULL, NULL);

        Log("Loading video file: %s", cpath);
        const char* args[] = {"loadfile", cpath, NULL};
        int result = mpv_command(g_mpv, args);
        Log("loadfile result: %d", result);

        delete[] cpath;
    }

    for (int i = 0; i < 10; i++) {
        mpv_wait_event(g_mpv, 50);
    }

    SetWindowPos(g_hWnd, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
    RedrawWindow(g_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    Log("Entering message loop");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Log("Exiting, cleaning up");
    DeleteCriticalSection(&g_cs);
    if (g_logFile) fclose(g_logFile);
    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
        break;
    case WM_DESTROY:
        if (g_mpv && mpv_terminate_destroy) {
            mpv_terminate_destroy(g_mpv);
        }
        if (g_mpvModule) {
            FreeLibrary(g_mpvModule);
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL IsWindows7OrLower()
{
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (!GetVersionEx((LPOSVERSIONINFO)&osvi)) {
        return FALSE;
    }

    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion <= 1) {
        return TRUE;
    }
    if (osvi.dwMajorVersion < 6) {
        return TRUE;
    }
    return FALSE;
}

BOOL LoadMpvLibrary()
{
    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);

    WCHAR dllPath[MAX_PATH];

    if (IsWindows7OrLower()) {
        swprintf(dllPath, sizeof(dllPath)/sizeof(WCHAR), L"%s\\lib_win7\\mpv-2.dll", exePath);
    } else {
        swprintf(dllPath, sizeof(dllPath)/sizeof(WCHAR), L"%s\\lib_win10\\mpv-2.dll", exePath);
    }

    Log("Loading MPV library from: %ls", dllPath);
    g_mpvModule = LoadLibraryW(dllPath);
    if (!g_mpvModule) {
        swprintf(dllPath, sizeof(dllPath)/sizeof(WCHAR), L"%s\\mpv-2.dll", exePath);
        Log("Failed, trying: %ls", dllPath);
        g_mpvModule = LoadLibraryW(dllPath);
    }

    if (!g_mpvModule) {
        Log("Failed to load MPV library!");
        return FALSE;
    }

    mpv_create = (mpv_create_func)GetProcAddress(g_mpvModule, "mpv_create");
    mpv_initialize = (mpv_initialize_func)GetProcAddress(g_mpvModule, "mpv_initialize");
    mpv_terminate_destroy = (mpv_terminate_destroy_func)GetProcAddress(g_mpvModule, "mpv_terminate_destroy");
    mpv_set_option_string = (mpv_set_option_string_func)GetProcAddress(g_mpvModule, "mpv_set_option_string");
    mpv_command_async = (mpv_command_async_func)GetProcAddress(g_mpvModule, "mpv_command_async");
    mpv_command = (mpv_command_func)GetProcAddress(g_mpvModule, "mpv_command");
    mpv_wait_event = (mpv_wait_event_func)GetProcAddress(g_mpvModule, "mpv_wait_event");
    mpv_get_property = (mpv_get_property_func)GetProcAddress(g_mpvModule, "mpv_get_property");

    return (mpv_create && mpv_initialize && mpv_terminate_destroy &&
            mpv_set_option_string && mpv_command_async && mpv_command &&
            mpv_wait_event && mpv_get_property);
}

void ScanVideos()
{
    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);

    WCHAR searchPath[MAX_PATH];
    swprintf(searchPath, sizeof(searchPath)/sizeof(WCHAR), L"%s\\video\\*", exePath);

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath, &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            WCHAR* ext = PathFindExtensionW(findData.cFileName);

            if (_wcsicmp(ext, L".mp4") == 0 ||
                _wcsicmp(ext, L".mkv") == 0 ||
                _wcsicmp(ext, L".avi") == 0 ||
                _wcsicmp(ext, L".mov") == 0 ||
                _wcsicmp(ext, L".jpg") == 0 ||
                _wcsicmp(ext, L".jpeg") == 0 ||
                _wcsicmp(ext, L".png") == 0 ||
                _wcsicmp(ext, L".bmp") == 0 ||
                _wcsicmp(ext, L".gif") == 0) {

                WCHAR fullPath[MAX_PATH];
                swprintf(fullPath, sizeof(fullPath)/sizeof(WCHAR), L"%s\\video\\%s", exePath, findData.cFileName);
                g_videoFiles.push_back(fullPath);
            }
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);

    std::sort(g_videoFiles.begin(), g_videoFiles.end());
}