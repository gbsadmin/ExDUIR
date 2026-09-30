#pragma once

// ================= FlowGraphEdit 常量 =================
#define FLOWGRAPHEDIT_PANEL_WIDTH 180
#define FLOWGRAPHEDIT_PANEL_ITEM_H 28
#define FLOWGRAPHEDIT_PANEL_MAX_VIS 8
#define FLOWGRAPHEDIT_PANEL_RADIUS 4
// ================= FlowGraphEdit 消息 =================
#define FLOWGRAPHEDIT_MESSAGE_ADDITEM      0x8020
#define FLOWGRAPHEDIT_MESSAGE_CLEARITEMS   0x8021
#define FLOWGRAPHEDIT_MESSAGE_SHOWPANEL    0x8022
#define FLOWGRAPHEDIT_MESSAGE_GETPLAINTEXT 0x8023
#define FLOWGRAPHEDIT_MESSAGE_SETINITTEXT  0x8024

#define FLOWGRAPHEDIT_MESSAGE_FORCECOPY  0x8025
#define FLOWGRAPHEDIT_MESSAGE_FORCECUT   0x8026
#define FLOWGRAPHEDIT_MESSAGE_FORCEPASTE 0x8027
// ================= FlowGraphEdit 事件 =================
#define FLOWGRAPHEDIT_EVENT_AT_TRIGGERED 10020
// ================= FlowGraphEdit 结构体 =================
#pragma pack(4)
struct FLOWGRAPHEDIT_ITEM {
	LPCWSTR szName;
};
#pragma pack()
#pragma pack(4)
struct FLOWGRAPHEDIT_PRIV {
	BOOL bPanelActive;
	INT nAtCharPos;
	WCHAR szFilter[256];
	INT nFilterLen;
	LPCWSTR* pItems;
	INT nItemCount;
	INT nItemCapacity;
	INT* pFilteredIdx;
	INT nFilteredCount;
	INT nPanelHover;
	INT nPanelScroll;
};
#pragma pack()

// 函数声明
void _flowgraphedit_register();
LRESULT CALLBACK _flowgraphedit_proc(HWND hWnd, HEXOBJ hObj, INT uMsg, WPARAM wParam, LPARAM lParam);
INT _flowgraphedit_getselectedplaintext(HEXOBJ hObj, obj_s* pObj, LPWSTR pBuffer, INT nBufSize);
INT _flowgraphedit_getplaintext(HEXOBJ hObj, obj_s* pObj, LPWSTR pBuffer, INT nBufSize);
void _flowgraphedit_insert_formatted(HEXOBJ hObj, obj_s* pObj, LPCWSTR pszText);