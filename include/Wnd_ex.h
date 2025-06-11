﻿#pragma once
#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib, "imm32.lib")

extern "C" {
int WINAPI MessageBoxTimeoutW(IN HWND hWnd, IN LPCWSTR lpText, IN LPCWSTR lpCaption, IN UINT uType,
                              IN WORD wLanguageId, IN DWORD dwMilliseconds);
}

#define EMV_HEXDUI 0
#define EMV_PFN_PACK 1

#define EMT_DUI -1
#define EMT_OBJECT -2
#define EMT_LAYOUT_UPDATE -3
#define EMT_EASING -4

typedef LRESULT(CALLBACK* ThunkPROC)(EX_THUNK_DATA*, INT, WPARAM, LPARAM);

struct EX_THUNK_DATA
{
    HWND    hWnd;
    WNDPROC OriginalProc;
    LPVOID dwData;
    ThunkPROC ThunkProc;
};



struct wnd_s
{
    union
    {
        obj_base base;
        struct
        {
            HEXDUI                  hexdui_;
            HEXOBJ                  objChildFirst_;
            HEXOBJ                  objChildLast_;
            HEXLAYOUT               hLayout_;
            INT                     dwFlags_;
            EX_BACKGROUNDIMAGEINFO* lpBackgroundImage_;
            HEXTHEME                hTheme_;
        };   // 方便使用base里的成员
    };
    UPDATELAYEREDWINDOWINFO ulwi_;
    /*INT ulwi_cbsize_;
    LPVOID ulwi_hdcDest_;
    LPVOID ulwi_pptDst_;
    LPVOID ulwi_psize_;
    LPVOID ulwi_hdcSrc_;
    LPVOID ulwi_pptSrc_;
    LPVOID ulwi_crKey_;
    LPVOID ulwi_pblend_;
    INT ulwi_dwFlags_;
    LPVOID ulwi_prcDirty_;*/
    INT           left_;   // the same as ulwi_pptDst_x
    INT           top_;    // the same as ulwi_pptDst_y
    INT           width_;
    INT           height_;
    INT           ulwi_pptSrc_x_;
    INT           ulwi_pptSrc_y_;
    BLENDFUNCTION ulwi_pblend_;
    INT           ulwi_prcDirty_left_;
    INT           ulwi_prcDirty_top_;
    INT           ulwi_prcDirty_right_;
    INT           ulwi_prcDirty_bottom_;

    ID2D1DeviceContext*          dx_context_;
    ID2D1GdiInteropRenderTarget* dx_gdiinterop_;
    INT                          dx_counts_ = 0;
    HEXCANVAS                    canvas_bkg_;
    HEXCANVAS                    canvas_display_;

    size_t dwWinState_;
    INT    dwStyle_;
    HIMC   hImc_;
    LPVOID hCursor_;
    HWND   hWnd_;
    HWND   hWndTips_;
    LPVOID lpNid_;
    LPVOID lpIDropTarget_;

    union
    {
        LPARAM lParam_;
        wnd_s* pMenuHostWnd_;
    };

    FLOAT fBlur_;

    HEXOBJ objTrackPrev_;
    HEXOBJ objTrack_;
    HEXOBJ objCaption_;
    HEXOBJ objFocus_;
    HEXOBJ objFocusPrev_;

    HEXOBJ objMenucontext_;
    LONG   dwHitObjPos_Abs_;
    INT    dwHitCode_;
    HEXOBJ objHittest_;
    HEXOBJ objHittestPrev_;
    INT    dwTrackContext_;
    wnd_s* pMenuTrackWnd_;

    ti_s* ti_auto_;
    ti_s* ti_track_;

    mbp_s*  lpMsgParams_;
    menu_s* lpMenuParams_;

    WinMsgPROC    pfnMsgProc_;
    HEXDUI        hExDuiParent_;
    EX_HASHTABLE* hTableObjects_;
    EX_HASHTABLE* hTableEvent_;
    LPVOID        pJSObj_;
    LPVOID        pSysObj_;

    EXHANDLE lpPopupParams_;
    HWND     hWndPopup_;

    LPVOID hrgn_client_;
    LPVOID hrgn_sizebox_;
    LPVOID padding_shadow_;
    LPVOID margin_caption_;
    LPVOID size_caption_;
    INT    minWidth_;
    INT    minHeight_;

    LPVOID  hRes_;
    INT     crBkg_;
    INT     crBorder_;
    HWND    hWndShadow_;
    INT     alpha_;
    wnd_s*  pMenuPrevWnd_;
    LPVOID  hMenuPopup_;
    INT     szItemSeparator_;
    LPVOID  padding_client_;
    LPVOID  padding_text_;
    LPVOID  padding_separator_;
    HEXFONT hFont_Menu_;

    LPVOID pJSDUIObj_;
    size_t hJSObjMap_;

    INT   menu_maxwidth_;
    INT   crSD_;
    INT   Radius_;
    void* vol_wndptr_;   //-143
};

LRESULT CALLBACK _wnd_defwindowprocW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL _wnd_getfromhandle(size_t handle, HWND* hWnd = NULL, wnd_s** pWnd = NULL, obj_s** pObj = NULL,
                        BOOL* isObject = NULL, INT* nError = NULL);
LRESULT _wnd_dispatch_notify(HWND hWnd, wnd_s* pWnd, size_t hObj, INT nID, INT nCode, WPARAM wParam,
                             LPARAM lParam, obj_s* pObj);
void    _wnd_redraw_bkg(HWND hWnd, wnd_s* pWnd, LPVOID lpRect, BOOL bRedrawBkg, BOOL bUpdate);
LRESULT _wnd_defaultproc(HWND hWnd, INT uMsg, WPARAM wParam, LPARAM lParam);
BOOL    _wnd_querystyle(HWND hWnd, INT dwStyle, BOOL bExStyle);
BOOL    _wnd_delstyle(HWND hWnd, INT dwStyle, BOOL bExStyle);
BOOL    _wnd_addstyle(HWND hWnd, INT dwStyle, BOOL bExStyle);
HICON   _wnd_geticonhandle(HWND hWnd, BOOL isbigicon);
BOOL    _wnd_getscreenrect(HWND hWnd, RECT* rcMonitor = NULL, RECT* rcDesk = NULL);
std::wstring _wnd_gettitle(HWND hWnd);
INT          _wnd_classtoatom(HWND hWnd);
INT          _wnd_getfunctionkeys();
void         _wnd_recalcclient(wnd_s* pWnd, HWND hWnd, INT width, INT height);
BOOL         _wnd_wm_stylechanging(wnd_s* pWnd, HWND hWnd, WPARAM wParam, LPARAM lParam);
void         _wnd_loadtheme(wnd_s* pWnd, HWND hWnd, HEXTHEME hTheme);
void         _wnd_calc_captionrect(wnd_s* pWnd, RECT* rcCaption);
INT _wnd_create(HEXDUI hExDui, wnd_s* pWnd, HWND hWnd, INT dwStyle, HEXTHEME hTheme, LPARAM lParam,
                WinMsgPROC lpfnMsgProc);
void CALLBACK _wnd_backgroundimage_timer_inherit(HWND hWnd, UINT uMsg, UINT_PTR idEvent,
                                                 DWORD dwTime);
size_t        _wnd_dispatch_msg(HWND hWnd, wnd_s* pWnd, INT uMsg, WPARAM wParam, LPARAM lParam);
size_t _wnd_dispatch_msg_obj(HWND hWnd, mempoolmsg_s* lpData, obj_s* pObj, INT uMsg, WPARAM wParam,
                             LPARAM lParam);
LRESULT CALLBACK _wnd_tooltips_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK _wnd_shadow_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
INT              _wnd_popupclose(wnd_s* pWnd, HWND hWnd, INT wParam, obj_s* pObj);
void             _wnd_dx_unint(wnd_s* pWnd);
void             _wnd_dx_init(wnd_s* pWnd);
HEXOBJ _wnd_wm_nchittest_obj(HWND hWnd, wnd_s* pWnd, HEXOBJ objLast, INT x, INT y, INT* hitCode,
                             obj_s** pObjMouse);
INT    _wnd_wm_nchittest_sizebox(INT width, INT height, INT x, INT y);
INT    _wnd_wm_nchittest(wnd_s* pWnd, HWND hWnd, LPARAM lParam);
void   _wnd_sysbutton_create(HWND hWnd, wnd_s* pWnd, INT dwStyle);
void   _wnd_render_obj(HWND hWnd, wnd_s* pWnd, ID2D1DeviceContext* pContext, HEXCANVAS cvDisplay,
                       LPVOID pBitmapDisplay, RECT rcPaint, EXHANDLE objChildFirst, INT offsetX,
                       INT offsetY, INT pAlpha, BOOL fDX, LPVOID hBorderBrush);
BOOL   _wnd_wm_setcursor(HWND hWnd, wnd_s* pWnd, LPARAM lParam);
void CALLBACK _wnd_timer_mousetrack(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void          _wnd_wm_leavecheck(HWND hWnd, wnd_s* pWnd, HEXOBJ objCheck, HEXOBJ objHittest,
                                 obj_s* pObjHittest, LPARAM lParam, BOOL fTrack);
INT           _wnd_destroy(HWND hWnd, wnd_s* pWnd);
void          _wnd_paint_bkg(HWND hWnd, wnd_s* pWnd);
void          _wnd_render_dc(HWND hWnd, wnd_s* pWnd, LPVOID hDC, HEXCANVAS cvDisplay, RECT rcPaint,
                             BOOL fLayer);
void          _wnd_render(HWND hWnd, wnd_s* pWnd, LPVOID hDC, RECT rcPaint, BOOL fLayer, BOOL fDX);
void          _wnd_wm_size(wnd_s* pWnd, HWND hWnd, WPARAM wParam, INT width, INT height);
void          _wnd_menu_setpos(HWND hWnd, wnd_s* pWnd, tagWINDOWPOS* lParam);
void          _wnd_menu_createitems(HWND hWnd, wnd_s* pWnd);
void          _wnd_menu_init(HWND hWnd, wnd_s* pWnd);
void          _wnd_paint_shadow(wnd_s* pWnd, BOOL bUpdateRgn, BOOL bFlush);
BOOL          _wnd_wm_paint(wnd_s* pWnd, HWND hWnd);
BOOL          _wnd_wm_getminmaxinfo(wnd_s* pWnd, HWND hWnd, LPARAM lParam);
void   _wnd_wm_buttondown(HWND hWnd, wnd_s* pWnd, HEXOBJ hObj, obj_s* pObj, INT uMsg, WPARAM wParam,
                          LPARAM lParam);
void   _wnd_obj_untrack(HWND hWnd, wnd_s* pWnd, LPARAM lParam, BOOL fMsgDispatch);
void   _wnd_wm_captionchange(HWND hWnd, wnd_s* pWnd, LPARAM lParam);
void   _wnd_wm_mouse(wnd_s* pWnd, HWND hWnd, INT uMsg, WPARAM wParam, LPARAM lParam);
void   _wnd_wm_menucontext(HWND hWnd, wnd_s* pWnd, INT uMsg, WPARAM wParam, LPARAM lParam);
void   _wnd_wm_mousewheel(HWND hWnd, wnd_s* pWnd, INT uMsg, WPARAM wParam, LPARAM lParam);
void   _wnd_wm_command(HWND hWnd, wnd_s* pWnd, INT uMsg, WPARAM wParam, LPARAM lParam);
HEXOBJ _wnd_obj_setnextfocus(HWND hWnd, wnd_s* pWnd, HEXOBJ objEntry, HEXOBJ objLastFocus);
void   _wnd_obj_settabstop(HWND hWnd, wnd_s* pWnd, HEXOBJ objLastFocus);
BOOL   _wnd_wm_keyboard(wnd_s* pWnd, HWND hWnd, INT uMsg, WPARAM wParam, LPARAM lParam);
BOOL   _wnd_obj_childtabstop(HEXOBJ objEntry, HEXOBJ* objFocusable, LPVOID* pObjFocusable,
                             HEXOBJ objLast, BOOL* bFounded);
void   _wnd_wm_ime_composition(HWND hWnd, wnd_s* pWnd);
BOOL   _wnd_wm_measureitem_host(wnd_s* pWnd, WPARAM wParam, LPARAM lParam);
void   _wnd_menu_updatecurrent(wnd_s* pWnd);
LRESULT CALLBACK _wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
BOOL             _wnd_menu_mouse(HWND hWnd, wnd_s* pWnd, INT uMsg, WPARAM wParam, LONG_PTR* iItem);
void _wnd_wm_initmenupopup(HWND hWnd, wnd_s* pWnd, HMENU hMenu);
BOOL Ex_DUIGetBackgroundImage(HEXDUI hExDui, LPVOID lpBackgroundImage);
void _wnd_getclientrect(wnd_s* pWnd, RECT* lpRect, INT* nError);
