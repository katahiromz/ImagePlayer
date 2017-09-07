// ImagePlayerApp.cpp
//////////////////////////////////////////////////////////////////////////////

#include "MWindowBase.hpp"
#include "resource.h"
#include "MTextToText.hpp"
#include "MBitmapDx.hpp"

#define TIMER_ID    999

//////////////////////////////////////////////////////////////////////////////

class MImagePlayerApp : public MWindowBase
{
protected:
    HINSTANCE   m_hInst;
    HICON       m_hIcon;
    TCHAR       m_szFile[MAX_PATH];
    MBitmapDx   m_bitmap;

public:
    MImagePlayerApp(HINSTANCE hInst) : m_hInst(hInst)
    {
        m_hIcon = LoadIcon(hInst, MAKEINTRESOURCE(1));
        m_szFile[0] = 0;
    }

    virtual ~MImagePlayerApp()
    {
        DestroyIcon(m_hIcon);
    }

    virtual LPCTSTR GetWndClassNameDx() const
    {
        return TEXT("MZC4 MImagePlayerApp Class");
    }

    virtual VOID ModifyWndClassDx(WNDCLASSEX& wcx)
    {
        wcx.lpszMenuName = MAKEINTRESOURCE(1);
        wcx.hIcon = m_hIcon;
        wcx.hIconSm = m_hIcon;
    }

    BOOL DoLoadFile(HWND hwnd, LPCTSTR pszFile)
    {
        KillTimer(hwnd, TIMER_ID);

        using namespace Gdiplus;
        MStringW strTextW = MTextToWide(pszFile);
        Bitmap *pBitmap = Bitmap::FromFile(strTextW.c_str());
        BOOL bOK = m_bitmap.SetBitmap(pBitmap);
        if (bOK)
        {
            SetTimer(hwnd, TIMER_ID, 0, NULL);
        }
        InvalidateRect(hwnd, NULL, TRUE);
        return bOK;
    }

    BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
    {
        DragAcceptFiles(hwnd, TRUE);
        return TRUE;
    }

    void OnExit(HWND hwnd)
    {
        DestroyWindow(hwnd);
    }

    void OnAbout(HWND hwnd)
    {
        MsgBoxDx(IDS_VERSION, IDS_APPNAME, MB_ICONINFORMATION);
    }

    void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
    {
        switch (id)
        {
        case CMDID_EXIT:
            OnExit(hwnd);
            break;
        case CMDID_ABOUT:
            OnAbout(hwnd);
            break;
        }
    }

    void OnDropFiles(HWND hwnd, HDROP hdrop)
    {
        TCHAR szFile[MAX_PATH];
        DragQueryFile(hdrop, 0, szFile, _countof(szFile));
        DoLoadFile(hwnd, szFile);
    }

    void OnPaint(HWND hwnd)
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hwnd, &ps);
        if (hDC)
        {
            if (m_bitmap.GetBitmap())
            {
                LONG cx, cy;
                HBITMAP hbm = m_bitmap.GetHBITMAP(cx, cy);
                if (hbm)
                {
                    HDC hMemDC = CreateCompatibleDC(NULL);
                    HGDIOBJ hbmOld = SelectObject(hMemDC, hbm);
                    BitBlt(hDC, 0, 0, cx, cy, hMemDC, 0, 0, SRCCOPY);
                    SelectObject(hMemDC, hbmOld);

                    DeleteDC(hMemDC);
                    DeleteObject(hbm);
                }
            }

            EndPaint(hwnd, &ps);
        }
    }

    void OnTimer(HWND hwnd, UINT id)
    {
        KillTimer(hwnd, id);
        if (id != TIMER_ID)
            return;

        DWORD dwDelay;
        if (m_bitmap.Step(dwDelay))
        {
            SetTimer(hwnd, TIMER_ID, dwDelay, NULL);
            InvalidateRect(hwnd, NULL, TRUE);
        }
    }

    virtual LRESULT CALLBACK
    WindowProcDx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hwnd, WM_DROPFILES, OnDropFiles);
        HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hwnd, WM_TIMER, OnTimer);
        default:
            return DefaultProcDx();
        }
    }

    void OnDestroy(HWND hwnd)
    {
        PostQuitMessage(0);
    }

    BOOL StartDx(INT nCmdShow)
    {
        if (!CreateWindowDx(NULL, LoadStringDx(IDS_APPNAME), WS_OVERLAPPEDWINDOW, 0,
                            CW_USEDEFAULT, CW_USEDEFAULT, 640, 400))
        {
            MessageBoxA(NULL, "CreateWindow failed.", NULL, MB_ICONERROR);
            return FALSE;
        }

        ShowWindow(m_hwnd, nCmdShow);
        UpdateWindow(m_hwnd);
        return TRUE;
    }

    INT RunDx()
    {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return (INT)msg.wParam;
    }
};

//////////////////////////////////////////////////////////////////////////////

extern "C" INT WINAPI
_tWinMain(HINSTANCE   hInstance,
          HINSTANCE   hPrevInstance,
          LPTSTR      lpCmdLine,
          INT         nCmdShow)
{
    INT ret = -1;

    InitCommonControls();

    Gdiplus::GdiplusStartupInput gp_startup_input;
    ULONG_PTR gp_token;
    Gdiplus::GdiplusStartup(&gp_token, &gp_startup_input, NULL);

    {
        MImagePlayerApp app(hInstance);
        if (app.StartDx(nCmdShow))
        {
            ret = app.RunDx();
        }
    }

    Gdiplus::GdiplusShutdown(gp_token);

    return ret;
}

//////////////////////////////////////////////////////////////////////////////