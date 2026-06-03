#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <algorithm>
#include <atomic>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

#include "mpv/client.h"

#define SWITCH_INTERVAL 15000
#define MAX_RETRY_COUNT 3

HWND g_hWnd = NULL;
mpv_handle* g_mpv = NULL;
HANDLE g_timerThread = NULL;
HANDLE g_eventThread = NULL;
std::vector<std::wstring> g_videoFiles;
int g_currentIndex = 0;
CRITICAL_SECTION g_cs;
std::atomic<bool> g_isPlaying(false);
std::atomic<int> g_retryCount(0);
std::atomic<bool> g_mpvError(false);
std::atomic<bool> g_isResetting(false);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
DWORD WINAPI TimerThread(LPVOID lpParam);
DWORD WINAPI EventThread(LPVOID lpParam);
void ScanVideos();
bool PlayNextVideo();
bool ResetMPV();

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    InitializeCriticalSection(&g_cs);

    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);

    WCHAR dllPath[MAX_PATH];
    BOOL isWin7 = FALSE;

    OSVERSIONINFOEXW osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    GetVersionExW((LPOSVERSIONINFOW)&osvi);

    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
        isWin7 = TRUE;
        swprintf(dllPath, sizeof(dllPath)/sizeof(WCHAR), L"%s\\..\\lib\\lib_win7", exePath);
    } else {
        swprintf(dllPath, sizeof(dllPath)/sizeof(WCHAR), L"%s\\..\\lib\\lib_win10", exePath);
    }
    SetDllDirectoryW(dllPath);

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
        MessageBox(NULL, _T("Window Registration Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    g_hWnd = CreateWindowEx(WS_EX_TOPMOST,
                           _T("VideoPlayer"),
                           _T("Fullscreen Video Player"),
                           WS_POPUP,
                           0, 0,
                           GetSystemMetrics(SM_CXSCREEN),
                           GetSystemMetrics(SM_CYSCREEN),
                           NULL, NULL, hInstance, NULL);

    if (!g_hWnd) {
        MessageBox(NULL, _T("Window Creation Failed!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
        DeleteCriticalSection(&g_cs);
        return 1;
    }

    ScanVideos();
    if (g_videoFiles.empty()) {
        MessageBox(NULL, _T("No video files found in video folder!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
        DeleteCriticalSection(&g_cs);
        return 1;
    }

    if (!ResetMPV()) {
        MessageBox(NULL, _T("Failed to initialize MPV!"), _T("Error!"), MB_ICONEXCLAMATION | MB_OK);
        DeleteCriticalSection(&g_cs);
        return 1;
    }

    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);

    PlayNextVideo();

    g_timerThread = CreateThread(NULL, 0, TimerThread, NULL, 0, NULL);
    g_eventThread = CreateThread(NULL, 0, EventThread, NULL, 0, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteCriticalSection(&g_cs);
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
        if (g_timerThread) {
            TerminateThread(g_timerThread, 0);
            CloseHandle(g_timerThread);
        }
        if (g_eventThread) {
            TerminateThread(g_eventThread, 0);
            CloseHandle(g_eventThread);
        }
        if (g_mpv) {
            mpv_terminate_destroy(g_mpv);
            g_mpv = NULL;
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void ScanVideos()
{
    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);
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
                _wcsicmp(ext, L".mov") == 0) {

                WCHAR fullPath[MAX_PATH];
                swprintf(fullPath, sizeof(fullPath)/sizeof(WCHAR), L"%s\\video\\%s", exePath, findData.cFileName);
                g_videoFiles.push_back(fullPath);
            }
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);

    std::sort(g_videoFiles.begin(), g_videoFiles.end());
}

bool ResetMPV()
{
    EnterCriticalSection(&g_cs);

    while (g_isResetting.load()) {
        LeaveCriticalSection(&g_cs);
        Sleep(10);
        EnterCriticalSection(&g_cs);
    }
    g_isResetting.store(true);

    if (g_mpv) {
        mpv_terminate_destroy(g_mpv);
        g_mpv = NULL;
    }

    g_mpv = mpv_create();
    if (!g_mpv) {
        g_isResetting.store(false);
        LeaveCriticalSection(&g_cs);
        return false;
    }

    char widStr[32];
    sprintf(widStr, "%lld", (LONG_PTR)g_hWnd);
    mpv_set_option_string(g_mpv, "wid", widStr);

    mpv_set_option_string(g_mpv, "vo", "gpu");

    OSVERSIONINFOEXW osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    GetVersionExW((LPOSVERSIONINFOW)&osvi);
    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
        mpv_set_option_string(g_mpv, "gpu-api", "angle");
    }

    mpv_set_option_string(g_mpv, "hwdec", "auto");
    mpv_set_option_string(g_mpv, "input-default-bindings", "yes");
    mpv_set_option_string(g_mpv, "input-vo-keyboard", "yes");
    mpv_set_option_string(g_mpv, "osc", "no");
    mpv_set_option_string(g_mpv, "border", "no");
    mpv_set_option_string(g_mpv, "cursor-autohide", "no");
    mpv_set_option_string(g_mpv, "keep-open", "no");
    mpv_set_option_string(g_mpv, "ytdl", "no");
    mpv_set_option_string(g_mpv, "script-opts", "ytdl_hook-disabled=yes");
    mpv_set_option_string(g_mpv, "force-window", "immediate");
    mpv_set_option_string(g_mpv, "video-margin-ratio-left", "0");
    mpv_set_option_string(g_mpv, "video-margin-ratio-right", "0");
    mpv_set_option_string(g_mpv, "video-margin-ratio-top", "0");
    mpv_set_option_string(g_mpv, "video-margin-ratio-bottom", "0");

    mpv_request_log_messages(g_mpv, "warn");

    if (mpv_initialize(g_mpv) < 0) {
        mpv_terminate_destroy(g_mpv);
        g_mpv = NULL;
        g_isResetting.store(false);
        LeaveCriticalSection(&g_cs);
        return false;
    }

    g_mpvError.store(false);
    g_retryCount.store(0);

    g_isResetting.store(false);
    LeaveCriticalSection(&g_cs);
    return true;
}

bool PlayNextVideo()
{
    EnterCriticalSection(&g_cs);

    while (g_isResetting.load()) {
        LeaveCriticalSection(&g_cs);
        Sleep(10);
        EnterCriticalSection(&g_cs);
    }

    if (!g_mpv || g_videoFiles.empty()) {
        LeaveCriticalSection(&g_cs);
        return false;
    }

    const wchar_t* path = g_videoFiles[g_currentIndex].c_str();

    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
    if (utf8Len <= 0) {
        LeaveCriticalSection(&g_cs);
        return false;
    }

    char* cpath = new char[utf8Len];
    if (!cpath) {
        LeaveCriticalSection(&g_cs);
        return false;
    }

    WideCharToMultiByte(CP_UTF8, 0, path, -1, cpath, utf8Len, NULL, NULL);

    const char* stopArgs[] = {"stop", NULL};
    mpv_command_async(g_mpv, -1, stopArgs);

    const char* args[] = {"loadfile", cpath, "replace", NULL};
    int result = mpv_command_async(g_mpv, -1, args);

    delete[] cpath;

    if (result < 0) {
        g_mpvError.store(true);
        LeaveCriticalSection(&g_cs);
        return false;
    }

    g_currentIndex++;
    if (g_currentIndex >= (int)g_videoFiles.size()) {
        g_currentIndex = 0;
    }

    g_isPlaying.store(true);
    g_retryCount.store(0);

    LeaveCriticalSection(&g_cs);
    return true;
}

DWORD WINAPI TimerThread(LPVOID lpParam)
{
    UNREFERENCED_PARAMETER(lpParam);

    while (true) {
        Sleep(SWITCH_INTERVAL);

        bool needsReset = g_mpvError.load() || !g_isPlaying.load();
        int retry = g_retryCount.load();

        if (needsReset && retry < MAX_RETRY_COUNT) {
            if (ResetMPV()) {
                PlayNextVideo();
            } else {
                g_retryCount.fetch_add(1);
            }
        } else if (!needsReset) {
            PlayNextVideo();
        }
    }

    return 0;
}

DWORD WINAPI EventThread(LPVOID lpParam)
{
    UNREFERENCED_PARAMETER(lpParam);

    while (true) {
        EnterCriticalSection(&g_cs);

        if (g_isResetting.load() || !g_mpv) {
            LeaveCriticalSection(&g_cs);
            Sleep(50);
            continue;
        }

        mpv_handle* mpv = g_mpv;
        LeaveCriticalSection(&g_cs);

        if (!mpv) {
            Sleep(50);
            continue;
        }

        mpv_event* event = mpv_wait_event(mpv, 100);

        EnterCriticalSection(&g_cs);

        if (g_isResetting.load() || g_mpv != mpv) {
            LeaveCriticalSection(&g_cs);
            continue;
        }

        switch (event->event_id) {
        case MPV_EVENT_START_FILE:
            g_isPlaying.store(true);
            g_mpvError.store(false);
            break;
        case MPV_EVENT_END_FILE: {
            g_isPlaying.store(false);
            mpv_event_end_file* ef = (mpv_event_end_file*)event->data;
            if (ef->error != MPV_ERROR_SUCCESS) {
                g_mpvError.store(true);
            }
            break;
        }

        case MPV_EVENT_LOG_MESSAGE:
            break;
        default:
            break;
        }

        LeaveCriticalSection(&g_cs);
    }

    return 0;
}