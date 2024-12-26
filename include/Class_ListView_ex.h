﻿#pragma once

// 属性_模板列表_表项数组指针
#define TEMPLATELISTVIEW_LONG_ITEMARRAY 0
// 属性_模板列表_悬浮背景色
#define TEMPLATELISTVIEW_LONG_HOVERCOLOR 1
// 属性_模板列表_选中背景色
#define TEMPLATELISTVIEW_LONG_SELECTCOLOR 2
// 属性_模板列表_边框色
#define TEMPLATELISTVIEW_LONG_BORDERCOLOR 3
// 属性_模板列表_可视起始索引
#define TEMPLATELISTVIEW_LONG_INDEX_START 4
// 属性_模板列表_可视起始索引
#define TEMPLATELISTVIEW_LONG_INDEX_END 5
// 属性_模板列表_横向偏移
#define TEMPLATELISTVIEW_LONG_OFFSETX 6
// 属性_模板列表_纵向偏移
#define TEMPLATELISTVIEW_LONG_OFFSETY 7

// 属性_模板列表_表项索引
#define TEMPLATELISTVIEW_LONG_ITEM_INDEX 0
// 属性_模板列表_表项圆角值1
#define TEMPLATELISTVIEW_LONG_ITEM_RADIUS1 1
// 属性_模板列表_表项圆角值2
#define TEMPLATELISTVIEW_LONG_ITEM_RADIUS2 2
// 属性_模板列表_表项圆角值3
#define TEMPLATELISTVIEW_LONG_ITEM_RADIUS3 3
// 属性_模板列表_表项圆角值4
#define TEMPLATELISTVIEW_LONG_ITEM_RADIUS4 4
// 属性_模板列表_表项填充色
#define TEMPLATELISTVIEW_LONG_ITEM_FILLCOLOR 5
// 属性_模板列表_表项启用圆角
#define TEMPLATELISTVIEW_LONG_ITEM_ENABLERADIUS 6
// 属性_模板列表_表项启用边框
#define TEMPLATELISTVIEW_LONG_ITEM_ENABLEBORDER 7
// 属性_模板列表_表项边框色
#define TEMPLATELISTVIEW_LONG_ITEM_BORDERCOLOR 8
// 属性_模板列表_表项边框粗细
#define TEMPLATELISTVIEW_LONG_ITEM_BORDERTHICKNESS 9


struct listview_s
{
    INT    width_item_;   // 项目宽度
    INT    height_item_;
    INT    width_split_;   // 间隔宽度
    INT    height_split_;
    INT    width_spec_;   // 特殊项目宽度
    INT    height_spec_;
    INT    width_view_;   // 项目总宽度
    INT    height_view_;
    INT    count_selects_;   // 选中项目数
    INT    count_items_;     // 项目总数
    INT    count_view_;      // 可视总数量
    INT    count_view_h_;    // 可视横向数量
    INT    count_view_v_;    // 可视纵向数量
    INT    index_select_;    // 当前选中
    INT    index_mouse_;     // 悬浮项目
    INT    index_start_;
    INT    index_end_;
    INT    nHittest_;
    INT    nOffsetX_;
    INT    nOffsetY_;
    LPVOID lpItems_;
};

void _listview_regsiter();
void _listview_updatesbvalue(HEXOBJ hObj, obj_s* pObj, listview_s* pOwner, RECT* lpRcClient);
void _listview_updateviewindex(HEXOBJ hObj, obj_s* pObj, listview_s* pOwner, BOOL bHView,
                               RECT* lpRcClient);
BOOL _listview_checkitem(listview_s* pOwner, INT iItem);
BOOL _listview_checkitem_view(listview_s* pOwner, INT iItem);
void _listview_rectfromiitem(obj_s* pObj, listview_s* pOwner, INT iItem, BOOL bHView, RECT* rcItem);
void _listview_lprectfromiitem(obj_s* pObj, listview_s* pOwner, INT iItem, BOOL bHView, RECT* lpRc);
INT  _listview_itemfrompos(obj_s* pObj, listview_s* pOwner, INT x, INT y, INT* offsetPosX,
                           INT* offsetPosY);
INT  _listview_getitemstate(LPVOID lpItems, INT iItem);
void _listview_setitemstate(LPVOID lpItems, INT iItem, INT dwState, BOOL bRemove);
BOOL _listview_queryitemstate(LPVOID lpItems, INT iItem, INT dwState);
void _listview_getscrollbarvalue(obj_s* pObj, listview_s* pOwner, BOOL bHSB, HEXOBJ* hSB, INT* oPos,
                                 INT* nLine, INT* nPage, INT* nView);
INT  _listview_checkpos(INT nPos, INT nView, INT nPage);
void _listview_redrawitem(obj_s* pObj, listview_s* pOwner, INT iItem);
void _listview_item_changestate(HWND hWnd, HEXOBJ hObj, obj_s* pObj, listview_s* pOwner,
                                LPVOID lpItems, INT iItem, INT state, BOOL bRemove, INT nEvent,
                                WPARAM wParam, LPARAM lParam);
size_t _listview_setitemcount(HWND hWnd, HEXOBJ hObj, obj_s* pObj, INT nCount, LPARAM lParam);
void   _listview_onvscrollbar(HWND hWnd, HEXOBJ hObj, obj_s* pObj, INT uMsg, WPARAM wParam,
                              LPARAM lParam);
size_t _listview_showitem(HWND hWnd, HEXOBJ hObj, obj_s* pObj, INT iItem, BOOL bCheck);
size_t _listview_reselect(HWND hWnd, HEXOBJ hObj, obj_s* pObj, listview_s* pOwner, INT iItem,
                          BOOL bShow);
void   _listview_btndown(HWND hWnd, HEXOBJ hObj, obj_s* pObj, INT uMsg, size_t wParram,
                         LPARAM lParam);
void   _listview_mouseleave(HWND hWnd, HEXOBJ hObj, obj_s* pObj);
void   _listview_mousemove(HWND hWnd, HEXOBJ hObj, obj_s* pObj, WPARAM wParam, LPARAM lParam);
void _listview_drawitem(HWND hWnd, HEXOBJ hObj, obj_s* pObj, listview_s* pOwner, EX_PAINTSTRUCT ps,
                        INT iItem, RECT rcClip, RECT rcItem);
size_t _listview_paint(HWND hWnd, HEXOBJ hObj, obj_s* pObj);
size_t _listview_size(HWND hWnd, HEXOBJ hObj, obj_s* pObj);
void   _listview_lvm_setitemstate(obj_s* pObj, listview_s* pOwner, INT iItem, INT dwState);
LRESULT CALLBACK _listview_proc(HWND hWnd, HEXOBJ hObj, INT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK _tlvi_proc(HWND hWnd, HEXOBJ hObj, INT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK _tlv_scrollbar_proc(HWND hWnd, HEXOBJ hObj, INT uMsg, WPARAM wParam, LPARAM lParam,
                                     LRESULT* lpResult);
LRESULT CALLBACK _tlv_proc(HWND hWnd, HEXOBJ hObj, INT uMsg, WPARAM wParam, LPARAM lParam);
void             _tlv_mousemove(HWND hWnd, HEXOBJ hObj, obj_s* pObj, WPARAM wParam, LPARAM lParam);
void _tlv_repos_items(HWND hWnd, HEXOBJ hObj, obj_s* pObj, EX_PAINTSTRUCT& ps, BOOL update);
void _tlv_refill(HWND hWnd, HEXOBJ hObj, obj_s* pObj, listview_s* pOwner, LONG_PTR iStart,
                 LONG_PTR iStartOld, LONG_PTR iEnd, LONG_PTR iEndOld, BOOL NeedUpdate, BOOL update);
void _tlv_array_del(array_s* hArray, int index, HEXOBJ pvData, int type);
void _tlv_mouseleave(HWND hWnd, HEXOBJ hObj, obj_s* pObj, LPARAM lParam);
LRESULT CALLBACK _tlv_childhObj_proc(HWND hWnd, HEXOBJ hObj, INT uMsg, WPARAM wParam, LPARAM lParam,
                                     LRESULT* lpResult);
INT CALLBACK     _tlv_EnumChild(size_t hObj, size_t lParam);
