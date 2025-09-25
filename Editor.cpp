#include <windows.h>
#include <tchar.h>
#include <ctime>
#include "resource.h"

#define IDM_FILE_OPEN    1001
#define IDM_FILE_SAVE    1002
#define IDM_FILE_EXIT    1003

#define IDM_EDIT_CUT     2001
#define IDM_EDIT_COPY    2002
#define IDM_EDIT_PASTE   2003

#define IDM_HELP_ABOUT   3001

#define TIMER_IDLE       1

time_t LastActivity = 0;
bool DarkScreenActive = false;

int height = 700;
int width = 1200;

// функция сброса таймера
void ResetIdleTimer() { LastActivity = time(nullptr); }

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ScreenSaverDlgProc(HWND, UINT, WPARAM, LPARAM);

HMENU CreateMainMenu() {
    HMENU hMenu = CreateMenu();
    HMENU hFile = CreateMenu(), hEdit = CreateMenu(), hHelp = CreateMenu();
    AppendMenu(hFile, MF_STRING, IDM_FILE_OPEN, _T("Открыть"));
    AppendMenu(hFile, MF_STRING, IDM_FILE_SAVE, _T("Сохранить"));
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_FILE_EXIT, _T("Выход"));
    AppendMenu(hEdit, MF_STRING, IDM_EDIT_CUT, _T("Вырезать"));
    AppendMenu(hEdit, MF_STRING, IDM_EDIT_COPY, _T("Копировать"));
    AppendMenu(hEdit, MF_STRING, IDM_EDIT_PASTE, _T("Вставить"));
    AppendMenu(hHelp, MF_STRING, IDM_HELP_ABOUT, _T("О программе"));
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, _T("Файл"));
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEdit, _T("Правка"));
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelp, _T("Справка"));
    return hMenu;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = _T("EditorMainWnd");
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    HWND hwnd = CreateWindowEx(
        0, _T("EditorMainWnd"), _T("Text Editor"),
        WS_OVERLAPPEDWINDOW, x, y, width, height,
        NULL, NULL, hInstance, NULL);

    SetMenu(hwnd, CreateMainMenu());
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    SetTimer(hwnd, TIMER_IDLE, 1000, NULL);
    ResetIdleTimer();

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!IsDialogMessage(hwnd, &msg)) {

            TranslateMessage(&msg);
            DispatchMessage(&msg);

        }
    }
    return (int)msg.wParam;
}

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG: {

        HWND hParent = GetParent(hDlg);
        if (!hParent) hParent = GetDesktopWindow();

        RECT rcParent, rcDlg;
        GetWindowRect(hParent, &rcParent);
        GetWindowRect(hDlg, &rcDlg);

        int dlgWidth = rcDlg.right - rcDlg.left;
        int dlgHeight = rcDlg.bottom - rcDlg.top;

        int x = rcParent.left + ((rcParent.right - rcParent.left) - dlgWidth) / 2;
        int y = rcParent.top + ((rcParent.bottom - rcParent.top) - dlgHeight) / 2;

        SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK ScreenSaverDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // параметры спрайта
    static int spriteX = 100, spriteY = 100, spriteDX = 6, spriteDY = 5;
    static POINT lastMouseScreen = { 0, 0 };
    static bool firstRun = true;
    static COLORREF spriteColor = RGB(100, 200, 250); // новый цвет
    static HBRUSH hBrush = NULL; // кисть для заливки

    switch (msg)
    {
    case WM_INITDIALOG: {
        SetTimer(hDlg, 1, 16, NULL);
        ShowWindow(hDlg, SW_SHOWMAXIMIZED);
        spriteX = 100; spriteY = 100; spriteDX = 6; spriteDY = 5;
        SetClassLongPtr(hDlg, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(0, 0, 0)));
        GetCursorPos(&lastMouseScreen); // запоминаем позицию мыши при запуске
        firstRun = true;
        ShowCursor(FALSE);
        // инициализация цвета и кисти
        spriteColor = RGB(100, 200, 250);
        if (hBrush) DeleteObject(hBrush);
        hBrush = CreateSolidBrush(spriteColor);
        return TRUE;
    }
    case WM_TIMER: {
        RECT rc;
        GetClientRect(hDlg, &rc);
        bool hit = false;
        spriteX += spriteDX; spriteY += spriteDY;
        if (spriteX < 0 || spriteX > rc.right - 70) {
            spriteDX = -spriteDX;
            hit = true;
        }
        if (spriteY < 0 || spriteY > rc.bottom - 70) {
            spriteDY = -spriteDY;
            hit = true;
        }
        if (hit) {
            spriteColor = RGB(rand() % 256, rand() % 256, rand() % 256);
            if (hBrush) DeleteObject(hBrush);
            hBrush = CreateSolidBrush(spriteColor);
        }
        InvalidateRect(hDlg, NULL, FALSE);
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);
        RECT rc;
        GetClientRect(hDlg, &rc);
        FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
        RECT sprite = { spriteX, spriteY, spriteX + 70, spriteY + 70 };
        FillRect(hdc, &sprite, hBrush);
        EndPaint(hDlg, &ps);
        break;
    }
    case WM_MOUSEMOVE: {
        POINT pt;
        GetCursorPos(&pt); //тк mousemove срабатывает при открытии окна с чёрным фоном, пришлось проверять координаты
        if (firstRun) {
            firstRun = false;
            lastMouseScreen = pt;
            break;
        }
        if (pt.x != lastMouseScreen.x || pt.y != lastMouseScreen.y) {
            ShowCursor(TRUE);
            KillTimer(hDlg, 1);
            if (hBrush) DeleteObject(hBrush);
            EndDialog(hDlg, 0);
            return 0;
        }
        return 0;
    }
    case WM_KEYDOWN:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_CLOSE:
        ShowCursor(TRUE);
        KillTimer(hDlg, 1);
        if (hBrush) DeleteObject(hBrush);
        EndDialog(hDlg, 0);
        return 0;
    }
    return FALSE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE: ResetIdleTimer(); break;

    case WM_COMMAND:
        ResetIdleTimer();
        switch (LOWORD(wParam))
        {
        case IDM_FILE_OPEN:  break;
        case IDM_FILE_SAVE:  break;
        case IDM_FILE_EXIT:  PostQuitMessage(0); break;
        case IDM_EDIT_CUT:   break;
        case IDM_EDIT_COPY:  break;
        case IDM_EDIT_PASTE: break;
        case IDM_HELP_ABOUT: DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, AboutDlgProc); break;
        }
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect; GetClientRect(hwnd, &rect);
        EndPaint(hwnd, &ps);
    } break;
    case WM_DESTROY: KillTimer(hwnd, TIMER_IDLE); PostQuitMessage(0); break;
    case WM_TIMER:
        if (wParam == TIMER_IDLE && !DarkScreenActive) {
            if (difftime(time(nullptr), LastActivity) >= 15) {
                DarkScreenActive = true;
                DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDDS_SCREENSAVER), hwnd, ScreenSaverDlgProc);
                DarkScreenActive = false;
                ResetIdleTimer();
            }
        }
        break;
    case WM_MOUSEMOVE:
    case WM_KEYDOWN:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        ResetIdleTimer();
        return 0;
    default: return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}