#include "Global_ex.h"

void Ex_SetLastError(INT nError)
{
    if (nError != 0)
    {
        Thread_EnterCriticalSection(g_Li.csError);
        g_Li.dwError = nError;
        Thread_LeaveCriticalSection(g_Li.csError);
    }
}

INT Ex_GetLastError()
{
    return g_Li.dwError;
}

BOOL Ex_Init(HINSTANCE hInstance, DWORD dwGlobalFlags, HCURSOR hDefaultCursor, LPCWSTR lpszDefaultClassName, LPVOID lpDefaultTheme, size_t dwDefaultThemeLen, LPVOID lpDefaultI18N, size_t dwDefaultI18NLen)
{
    CoInitialize(NULL);
    g_Li.csError = Thread_InitializeCriticalSection();

    g_Li.hInstance = hInstance;
    g_Li.dwFlags = dwGlobalFlags;
    if (hDefaultCursor == 0)
    {
        g_Li.hCursor = LoadCursorW(0, IDC_ARROW);
    }
    else
    {
        g_Li.hCursor = hDefaultCursor;
    }
    if (lpszDefaultClassName == NULL)
    {
        lpszDefaultClassName = L"Ex_DirectUI";
    }

    TCHAR szFilePath[MAX_PATH + 1];
    GetModuleFileName(NULL, szFilePath, MAX_PATH); //取自身完整路径

    g_Li.hIcon = ExtractIconW(g_Li.hInstance, szFilePath, 0);
    g_Li.hIconsm = ExtractIconW(g_Li.hInstance, szFilePath, 0);

    g_Li.atomClassName = Ex_WndRegisterClass(lpszDefaultClassName, NULL, NULL, NULL);

    g_Li.dwMessage = RegisterWindowMessageW(L"Ex_DirectUI");
    g_Li.hTableClass = HashTable_Create(47, pfnDefaultFreeData);
    g_Li.hTableFont = HashTable_Create(17, pfnDefaultFreeFont);
    g_Li.hTableLayout = HashTable_Create(17, NULL);
    g_Li.hMemPoolMsg = MemPool_Create(256, sizeof(mempoolmsg_s), 0);
    g_Li.hHandles = _handle_init();
    g_Li.dwClickTime = GetDoubleClickTime() * 2;

    g_Li.aryColorsAtom = {ATOM_BACKGROUND_COLOR, ATOM_COLOR_BACKGROUND, ATOM_BORDER_COLOR, ATOM_COLOR_BORDER, ATOM_COLOR, ATOM_COLOR_NORMAL, ATOM_COLOR_HOVER, ATOM_COLOR_DOWN, ATOM_COLOR_FOCUS, ATOM_COLOR_CHECKED, ATOM_COLOR_SELECTED, ATOM_COLOR_HOT, ATOM_COLOR_VISTED, ATOM_COLOR_SHADOW};
    g_Li.aryColorsOffset = {offsetof(obj_s, crBackground_), offsetof(obj_s, crBackground_), offsetof(obj_s, crBorder_), offsetof(obj_s, crBorder_), offsetof(obj_s, crNormal_), offsetof(obj_s, crNormal_), offsetof(obj_s, crHover_), offsetof(obj_s, crDown_), offsetof(obj_s, crFocus_), offsetof(obj_s, crChecked_), offsetof(obj_s, crSelected_), offsetof(obj_s, crHot_), offsetof(obj_s, crVisted_), offsetof(obj_s, crShadow_)};
    g_Li.hModuleUser = GetModuleHandleW(L"user32.dll");
    //g_Ri.hRiched20 = LoadLibraryW(L"Msftedit.dll");//编辑框空格选中首行bug
    g_Ri.hRiched20 = LoadLibraryW(L"Riched20.dll");
    LPVOID i = Ex_MemAlloc(64);
    INT len;
    len = LoadStringW(g_Li.hModuleUser, 900, (LPWSTR)i, 64);
    g_Li.lpStrMin = StrDupW((LPWSTR)i);
    RtlZeroMemory(i, 64);
    len = LoadStringW(g_Li.hModuleUser, 901, (LPWSTR)i, 64);
    g_Li.lpStrMax = StrDupW((LPWSTR)i);
    RtlZeroMemory(i, 64);
    len = LoadStringW(g_Li.hModuleUser, 902, (LPWSTR)i, 64);
    g_Li.lpStrResMin = StrDupW((LPWSTR)i);
    RtlZeroMemory(i, 64);
    len = LoadStringW(g_Li.hModuleUser, 903, (LPWSTR)i, 64);
    g_Li.lpStrResMax = StrDupW((LPWSTR)i);
    RtlZeroMemory(i, 64);
    len = LoadStringW(g_Li.hModuleUser, 904, (LPWSTR)i, 64);
    g_Li.lpStrHelp = StrDupW((LPWSTR)i);
    RtlZeroMemory(i, 64);
    len = LoadStringW(g_Li.hModuleUser, 905, (LPWSTR)i, 64);
    g_Li.lpStrClose = StrDupW((LPWSTR)i);
    Ex_MemFree(i);
    INT nError = 0;
    _canvas_init(&nError);
    SetProcessDPIAware();

    g_Li.DpiX = 1;
    g_Li.DpiY = 1;
    HDC dc = GetDC(NULL);
    g_Li.DpiX_Real = (FLOAT)GetDeviceCaps(dc, 88) / 96;
    g_Li.DpiY_Real = (FLOAT)GetDeviceCaps(dc, 90) / 96;
    ReleaseDC(NULL, dc);
    if (g_Li.DpiX_Real == 1 && g_Li.DpiY_Real == 1)
    {
        Flag_Del(ENGINE_FLAG_DPI_ENABLE);
    }
    else
    {
        if ((dwGlobalFlags & ENGINE_FLAG_DPI_ENABLE) != 0)
        {
            g_Li.DpiX = g_Li.DpiX_Real;
            g_Li.DpiY = g_Li.DpiY_Real;
        }
    }
    g_Li.lpLogFontDefault = (LOGFONTW *)Ex_MemAlloc(sizeof(LOGFONTW));
    SystemParametersInfoW(31, sizeof(LOGFONTW), g_Li.lpLogFontDefault, 0);
    if (!Flag_Query(ENGINE_FLAG_DPI_ENABLE))
    {
        g_Li.lpLogFontDefault->lfHeight = (FLOAT)g_Li.lpLogFontDefault->lfHeight / g_Li.DpiY_Real;
    }

    _object_init();

    g_Li.aryThemes.clear();
    Ex_ThemeLoadFromMemory(lpDefaultTheme, dwDefaultThemeLen, 0, 0, TRUE);
    _layout_init();
    g_Li.atomSysShadow = Ex_WndRegisterClass(L"SysShadow", 0, 0, 0);

    g_Li.hHookMsgBox = SetWindowsHookExW(WH_CBT, (HOOKPROC)_hook_proc, 0, GetCurrentThreadId());
    
    g_Li.fContext = new MFFontContext(g_Ri.pDWriteFactory);
    Ex_SetLastError(nError);
    return nError == 0;
}

void Ex_UnInit()
{   
    UnhookWindowsHookEx(g_Li.hHookMsgBox);
    Ex_MemFree((LPVOID)g_Li.lpStrMin);
    Ex_MemFree((LPVOID)g_Li.lpStrMax);
    Ex_MemFree((LPVOID)g_Li.lpStrResMin);
    Ex_MemFree((LPVOID)g_Li.lpStrResMax);
    Ex_MemFree((LPVOID)g_Li.lpStrClose);
    Ex_MemFree((LPVOID)g_Li.lpStrHelp);
    Ex_ThemeFree(g_Li.hThemeDefault);//新增
    Ex_MemFree(g_Li.lpLogFontDefault);
    _canvas_uninit();
    _handle_uninit(g_Li.hHandles);
    HashTable_Destroy(g_Li.hTableClass);
    HashTable_Destroy(g_Li.hTableFont);
    HashTable_Destroy(g_Li.hTableLayout);
    MemPool_Destroy(g_Li.hMemPoolMsg);
    Thread_DeleteCriticalSection(g_Li.csError);
    FreeLibrary(g_Ri.hRiched20);
    delete g_Li.fContext;
    CoUninitialize();
}



FLOAT Ex_Scale(FLOAT n) //OK
{
    if (g_Li.DpiX > 1)
    {
        n = round(n * g_Li.DpiX);
    }
    return n;
}

void Ex_Sleep(INT us)
{
    LARGE_INTEGER li;
    li.QuadPart = (LONGLONG)(-10 * us);
    HANDLE hTimer = CreateWaitableTimerW(0, 0, 0);
    if (hTimer)
    {
        SetWaitableTimer(hTimer, &li, 0, 0, 0, 0);
        while (MsgWaitForMultipleObjects(1, &hTimer, 0, -1, 255) != 0)
        {
            MSG msg;
            while (PeekMessageW(&msg, 0, 0, 0, 1))
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
        CloseHandle(hTimer);
    }
}

EXATOM Ex_Atom(LPCWSTR lptstring) //OK
{
    INT len = lstrlenW(lptstring);
    EXATOM ret = 1;
    if (len > 0)
    {
        PWSTR buff = (PWSTR)StrDupW(lptstring);
        CharLowerW(buff);
        ret = Crc32_Addr((UCHAR *)buff, len * 2);
        Ex_MemFree(buff);
    }
    return ret;
}

LPVOID Ex_AllocBuffer(size_t dwSize)
{
    return Ex_MemAlloc(dwSize);
}

BOOL Ex_FreeBuffer(LPVOID lpBuffer)
{
    return Ex_MemFree(lpBuffer);
}

LPVOID Ex_LoadImageFromMemory(LPVOID lpData, size_t dwLen, INT uType, INT nIndex)
{
    LPVOID ret = 0;
    if (!IsBadReadPtr(lpData, dwLen))
    {
        LPVOID hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwLen);
        if (hMem)
        {
            LPVOID lpMem = GlobalLock(hMem);
            if (lpMem)
            {
                RtlMoveMemory(lpMem, lpData, dwLen);
                GlobalUnlock(hMem);
                if (uType == IMAGE_ICON || uType == IMAGE_CURSOR)
                {
                    if (nIndex == 0)
                        nIndex = 1;
                    WORD s = ((ICONDIR *)lpData)->idCount;
                    if (s >= nIndex)
                    {
                        short type = ((ICONDIR *)lpData)->idType;
                        BOOL bIcon = (type == IMAGE_ICON);
                        LPVOID offset = (LPVOID)((size_t)lpData + 6 + (nIndex - 1) * sizeof(ICONDIRENTRY));
                        LPVOID tmp = lpMem;
                        if (!bIcon)
                        {
                            __set_short(lpMem, 0, ((ICONDIRENTRY *)offset)->wBitCount);
                            tmp = (LPVOID)((size_t)tmp + 4);
                        }
                        RtlMoveMemory(tmp, (LPVOID)((size_t)lpData + ((ICONDIRENTRY *)offset)->dwImageOffset), ((ICONDIRENTRY *)offset)->dwBytesInRes);
                        CHAR b1 = ((ICONDIRENTRY *)offset)->bWidth;
                        CHAR b2 = ((ICONDIRENTRY *)offset)->bHeight;
                        HICON hicon = CreateIconFromResourceEx((PBYTE)lpMem, ((ICONDIRENTRY *)offset)->dwBytesInRes, bIcon, 196608, b1, b2, 0);
                        ret = CopyImage(hicon, uType, 0, 0, 4);
                    }
                }
                else
                {
                    LPSTREAM lpStream;
                    if (CreateStreamOnHGlobal(hMem, FALSE, &lpStream))
                    {
                        LPVOID lpObj = nullptr;
                        if (OleLoadPicture(lpStream, dwLen, TRUE, IID_IPicture, &lpObj) == 0)
                        {
                            OLE_HANDLE hImg = 0;
                            ((IPicture *)lpObj)->get_Handle(&hImg);
                            SHORT objType = 0;
                            ((IPicture *)lpObj)->get_Type(&objType);
                            if (objType == uType)
                            {
                                ret = CopyImage((HANDLE)hImg, uType, 0, 0, 4);
                            }
                            ((IPicture *)lpObj)->Release();
                        }
                        lpStream->Release();
                    }
                }
            }
            GlobalFree(hMem);
        }
    }
    return ret;
}

BOOL Ex_LoadBitMapFromMemory(LPVOID lpData, size_t dwLen, HBITMAP *retBitMap)
{
    INT nError = 0;
    IWICStream *pIWICStream = nullptr;
    HRESULT hr = g_Ri.pWICFactory->CreateStream(&pIWICStream);
    BOOL ret = FALSE;
    if (hr == 0)
    {
        hr = pIWICStream->InitializeFromMemory((WICInProcPointer)lpData, dwLen);
        if (hr == 0)
        {
            IWICBitmapDecoder *pDecoder = NULL;
            hr = g_Ri.pWICFactory->CreateDecoderFromStream(
                pIWICStream,
                NULL,
                WICDecodeMetadataCacheOnLoad,
                &pDecoder);
            if (hr == 0)
            {
                IWICBitmapFrameDecode *pSource = NULL;
                hr = pDecoder->GetFrame(0, &pSource);
                IWICFormatConverter *pConverter = NULL;
                hr = g_Ri.pWICFactory->CreateFormatConverter(&pConverter);
                pConverter->Initialize((IWICBitmap *)pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
                UINT width = 0, height = 0;
                pConverter->GetSize(&width, &height);
                std::vector<BYTE> buffer(width * height * 4);
                pConverter->CopyPixels(0, width * 4, buffer.size(), buffer.data());
                if (retBitMap)
                {
                    *retBitMap = CreateBitmap(width, height, 1, 32, buffer.data());
                    ret = TRUE;
                }
                pSource->Release();
                pConverter->Release();
            }
            pDecoder->Release();
        }
        pIWICStream->Release();
    }
    return ret;
}



LPVOID Ex_MemAlloc(size_t dwSize, INT dwFlags)
{
    return HeapAlloc(GetProcessHeap(), 8, dwSize);
}

LPVOID Ex_MemReAlloc(LPVOID hMem, size_t dwSize)
{
    return HeapReAlloc(GetProcessHeap(), 8, hMem, dwSize);
}

BOOL Ex_MemFree(LPVOID hMem)
{
    if (hMem != nullptr)
    {
        return HeapFree(GetProcessHeap(), 0, hMem);
    }
    return FALSE;
}

BOOL Ex_ReadFile(LPCWSTR filePath, std::vector<CHAR>* retData)
{
    BOOL fOK = FALSE;
    if (filePath != L"")
    {
        HANDLE hFile = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD nSize = GetFileSize(hFile, NULL);
            if (nSize != INVALID_FILE_SIZE)
            {
                (*retData).resize(nSize);
                fOK = ReadFile(hFile, &(*retData)[0], nSize, &nSize, NULL);
            }
            CloseHandle(hFile);
        }
    }
    return fOK;
}

/* 读取RC资源 */
BOOL Ex_ReadResSource(WORD lpname, LPCWSTR lpType, std::vector<CHAR>* retData)
{
    BOOL fOK = FALSE;
    /* 检查参数有效性 */
    if (lpname)
    {
        /* 查找资源 */
        HRSRC hRsrc = FindResourceW(NULL, MAKEINTRESOURCE(lpname), lpType);
        if (NULL == hRsrc)
            return (FALSE);

        /* 获取资源的大小 */
        DWORD dwSize = SizeofResource(NULL, hRsrc);
        if (dwSize == 0)
        {
            return (FALSE);
        }

        /* 加载资源 */
        HGLOBAL hGlobal = LoadResource(NULL, hRsrc);
        if (hGlobal == 0)
        {
            return (FALSE);
        }

        /* 锁定资源 */
        LPVOID pBuffer = LockResource(hGlobal);
        if (pBuffer == NULL)
        {
            return (FALSE);
        }

        (*retData).resize(dwSize);
        RtlMoveMemory((*retData).data(), pBuffer, dwSize);
        fOK = TRUE;
        /*
         * 在资源使用完毕后我们可以不需要使用 UnlockResource和FreeResource来手动地释放资源
         * 因为它们都是16位Windows遗留下来的，在Win32中，在使用完毕后系统会自动回收。
         */
        UnlockResource(hGlobal);
        FreeResource(hGlobal);
    }
    return (fOK);
}
