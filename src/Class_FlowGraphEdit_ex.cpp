#include "stdafx.h"


// 全局基类回调
ClsPROC m_pfnFlowGraphEditProc;
inline FLOWGRAPHEDIT_PRIV* _fgeditpriv(HEXOBJ hObj) {
	return (FLOWGRAPHEDIT_PRIV*)Ex_ObjGetLong(hObj, OBJECT_LONG_USERDATA);
}
// ============================================================
// 注册
// ============================================================
void _flowgraphedit_register() {
	WCHAR oldwzCls[] = L"Edit";
	EX_CLASSINFO pClsInfoEdit;
	Ex_ObjGetClassInfoEx(oldwzCls, &pClsInfoEdit);
	m_pfnFlowGraphEditProc = pClsInfoEdit.pfnClsProc;
	WCHAR newwzCls[] = L"FlowGraphEdit";
	Ex_ObjRegister(newwzCls,
		OBJECT_STYLE_VISIBLE,
		OBJECT_STYLE_EX_COMPOSITED | OBJECT_STYLE_EX_FOCUSABLE |
		OBJECT_STYLE_EX_TABSTOP | OBJECT_STYLE_EX_CUSTOMDRAW,
		DT_NOPREFIX | DT_SINGLELINE,
		0, LoadCursorW(0, MAKEINTRESOURCEW(32513)),
		CANVAS_FLAG_GDI_COMPATIBLE,
		_flowgraphedit_proc);
}
// ============================================================
// 素材管理
// ============================================================
BOOL _flowgraphedit_add(FLOWGRAPHEDIT_PRIV* pPriv, LPCWSTR name) {
	if (!name) return FALSE;
	for (INT i = 0; i < pPriv->nItemCount; i++) {
		if (lstrcmpW(pPriv->pItems[i], name) == 0) return TRUE; // 去重
	}
	if (pPriv->nItemCount >= pPriv->nItemCapacity) {
		INT newCap = pPriv->nItemCapacity == 0 ? 16 : pPriv->nItemCapacity * 2;
		LPCWSTR* pNew = (LPCWSTR*)Ex_MemAlloc(sizeof(LPCWSTR) * newCap);
		if (!pNew) return FALSE;
		memset(pNew, 0, sizeof(LPCWSTR) * newCap);
		if (pPriv->pItems && pPriv->nItemCount > 0) {
			memcpy(pNew, pPriv->pItems, sizeof(LPCWSTR) * pPriv->nItemCount);
			Ex_MemFree(pPriv->pItems);
		}
		pPriv->pItems = pNew;
		pPriv->nItemCapacity = newCap;
		if (pPriv->pFilteredIdx) Ex_MemFree(pPriv->pFilteredIdx);
		pPriv->pFilteredIdx = (INT*)Ex_MemAlloc(sizeof(INT) * newCap);
	}
	pPriv->pItems[pPriv->nItemCount++] = StrDupW(name);
	return TRUE;
}
void _flowgraphedit_clear(FLOWGRAPHEDIT_PRIV* pPriv) {
	for (INT i = 0; i < pPriv->nItemCount; i++) {
		if (pPriv->pItems[i]) Ex_MemFree((LPVOID)pPriv->pItems[i]);
	}
	pPriv->nItemCount = 0;
}
void _flowgraphedit_filter(FLOWGRAPHEDIT_PRIV* pPriv) {
	pPriv->nFilteredCount = 0;
	for (INT i = 0; i < pPriv->nItemCount; i++) {
		if (pPriv->nFilterLen == 0 || wcsstr(pPriv->pItems[i], pPriv->szFilter) != nullptr) {
			pPriv->pFilteredIdx[pPriv->nFilteredCount++] = i;
		}
	}
	if (pPriv->nPanelHover >= pPriv->nFilteredCount) pPriv->nPanelHover = pPriv->nFilteredCount - 1;
	if (pPriv->nPanelHover < 0 && pPriv->nFilteredCount > 0) pPriv->nPanelHover = 0;
}
void _flowgraphedit_showpanel(HEXOBJ hObj, FLOWGRAPHEDIT_PRIV* pPriv) {
	pPriv->bPanelActive = TRUE;
	pPriv->nPanelScroll = 0;
	_flowgraphedit_filter(pPriv);
	Ex_ObjInvalidateRect(hObj, 0);
}
void _flowgraphedit_hidepanel(HEXOBJ hObj, FLOWGRAPHEDIT_PRIV* pPriv) {
	if (pPriv && pPriv->bPanelActive) {
		pPriv->bPanelActive = FALSE;
		Ex_ObjInvalidateRect(hObj, 0);
	}
}
// ============================================================
// 插入文本
// ============================================================
void _flowgraphedit_insert(HEXOBJ hObj, obj_s* pObj, FLOWGRAPHEDIT_PRIV* pPriv, INT itemIdx) {
	if (!pObj || itemIdx < 0 || itemIdx >= pPriv->nItemCount) return;
	LPCWSTR pName = pPriv->pItems[itemIdx];
	BOOL sOK;

	DWORD oldMask = (DWORD)_edit_sendmessage(pObj, EM_GETEVENTMASK, 0, 0, &sOK);
	_edit_sendmessage(pObj, EM_SETEVENTMASK, 0, 0, &sOK);

	// 删除 "@过滤文本"
	INT delEnd = pPriv->nAtCharPos + 1 + pPriv->nFilterLen;
	_edit_sendmessage(pObj, EM_SETSEL, pPriv->nAtCharPos, delEnd, &sOK);
	_edit_sendmessage(pObj, EM_REPLACESEL, TRUE, (LPARAM)L"", &sOK);

	// 构建插入文本：@{素材名} + 零宽空格
	WCHAR szInsert[512];
	swprintf_s(szInsert, L"@{%s}\x200B", pName);
	INT nameLen = 3 + lstrlenW(pName); // @{ + name + }
	INT totalLen = lstrlenW(szInsert);

	_edit_sendmessage(pObj, EM_SETSEL, pPriv->nAtCharPos, pPriv->nAtCharPos, &sOK);
	_edit_sendmessage(pObj, EM_REPLACESEL, TRUE, (LPARAM)szInsert, &sOK);

	// 对 "@{素材名}" 部分应用蓝色+链接格式
	_edit_sendmessage(pObj, EM_SETSEL, pPriv->nAtCharPos, pPriv->nAtCharPos + nameLen, &sOK);
	CHARFORMAT2W cf;
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR | CFM_LINK | CFM_UNDERLINE;
	cf.dwEffects = (DWORD)CFE_LINK | CFE_UNDERLINE;
	cf.crTextColor = RGB(30, 120, 255);
	_edit_sendmessage(pObj, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf, &sOK);

	// 对 "零宽空格" 强制应用默认普通格式
	_edit_sendmessage(pObj, EM_SETSEL, pPriv->nAtCharPos + nameLen, pPriv->nAtCharPos + totalLen, &sOK);
	CHARFORMAT2W cfDefault;
	memset(&cfDefault, 0, sizeof(cfDefault));
	cfDefault.cbSize = sizeof(cfDefault);
	cfDefault.dwMask = CFM_COLOR | CFM_LINK | CFM_UNDERLINE;
	cfDefault.dwEffects = 0;

	_edit_sendmessage(pObj, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfDefault, &sOK);

	// 将光标移动到零宽空格之后
	_edit_sendmessage(pObj, EM_SETSEL, pPriv->nAtCharPos + totalLen, pPriv->nAtCharPos + totalLen, &sOK);
	_edit_sendmessage(pObj, EM_SCROLLCARET, 0, 0, &sOK);

	// ★★★ 终极修复：强制重置光标插入点格式，防止后续输入继承蓝色 ★★★
	CHARFORMAT2W cfReset;
	memset(&cfReset, 0, sizeof(cfReset));
	cfReset.cbSize = sizeof(cfReset);
	cfReset.dwMask = CFM_COLOR | CFM_LINK | CFM_UNDERLINE;
	cfReset.dwEffects = 0;
	_edit_sendmessage(pObj, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfReset, &sOK);

	_edit_sendmessage(pObj, EM_SETEVENTMASK, 0, oldMask, &sOK);
	_flowgraphedit_hidepanel(hObj, pPriv);
}
// ============================================================
// 获取纯文本
// ============================================================
INT _flowgraphedit_getplaintext(HEXOBJ hObj, obj_s* pObj, LPWSTR pBuffer, INT nBufSize) {
	if (!pObj) return 0;
	BOOL sOK;

	// ★ 修复1：使用 GTL_NUMCHARS 替代 GTL_DEFAULT，获取精确字符数
	GETTEXTLENGTHEX gtl = { GTL_NUMCHARS, 1200 };
	LRESULT textLen = _edit_sendmessage(pObj, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0, &sOK);
	if (textLen <= 0) {
		if (pBuffer && nBufSize > 0) pBuffer[0] = L'\0';
		return 0;
	}

	// ★ 修复2：使用 EM_GETTEXTRANGE 替代 EM_GETTEXTEX
	// EM_GETTEXTRANGE 更可靠，不会因 CFE_LINK 格式化区域导致文本截断
	INT bufChars = (INT)textLen + 256;  // 额外空间防止边界问题
	LPWSTR pText = (LPWSTR)Ex_MemAlloc(bufChars * sizeof(WCHAR));
	if (!pText) return 0;
	memset(pText, 0, bufChars * sizeof(WCHAR));

	TEXTRANGE tr;
	tr.chrg.cpMin = 0;
	tr.chrg.cpMax = -1;  // 获取全部文本
	tr.lpstrText = pText;

	LRESULT charsCopied = _edit_sendmessage(pObj, EM_GETTEXTRANGE, 0, (LPARAM)&tr, &sOK);

	// 如果 EM_GETTEXTRANGE 失败或返回0，回退到 EM_GETTEXTEX
	if (charsCopied <= 0) {
		// 回退方案：使用原有的 EM_GETTEXTEX
		GETTEXTEX gt = { 0 };
		gt.cb = (DWORD)(bufChars * sizeof(WCHAR));
		gt.flags = GT_DEFAULT;
		gt.codepage = 1200;
		_edit_sendmessage(pObj, EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pText, &sOK);
	}

	// 过滤零宽空格
	INT resultCap = bufChars;
	LPWSTR pResult = (LPWSTR)Ex_MemAlloc(resultCap * sizeof(WCHAR));
	INT resultPos = 0;

	for (INT i = 0; pText[i] != L'\0' && resultPos < resultCap - 2; i++) {
		if (pText[i] != L'\x200B') {
			pResult[resultPos++] = pText[i];
		}
	}
	pResult[resultPos] = L'\0';

	INT copyLen = __min(resultPos, nBufSize - 1);
	if (pBuffer && nBufSize > 0) {
		memcpy(pBuffer, pResult, copyLen * sizeof(WCHAR));
		pBuffer[copyLen] = L'\0';
	}
	Ex_MemFree(pResult);
	Ex_MemFree(pText);
	return resultPos;
}

// ============================================================
// 获取选中部分的纯文本 (用于 Ctrl+C 复制)
// ============================================================
INT _flowgraphedit_getselectedplaintext(HEXOBJ hObj, obj_s* pObj, LPWSTR pBuffer, INT nBufSize) {
	if (!pObj) return 0;
	BOOL sOK;
	CHARRANGE sel;
	_edit_sendmessage(pObj, EM_EXGETSEL, 0, (LPARAM)&sel, &sOK);

	if (sel.cpMin >= sel.cpMax) {
		if (pBuffer && nBufSize > 0) pBuffer[0] = L'\0';
		return 0;
	}

	INT textLen = sel.cpMax - sel.cpMin;
	INT bufChars = textLen + 256;
	LPWSTR pText = (LPWSTR)Ex_MemAlloc(bufChars * sizeof(WCHAR));
	if (!pText) return 0;
	memset(pText, 0, bufChars * sizeof(WCHAR));

	// ★ 修复：使用 EM_GETTEXTRANGE 替代 EM_GETTEXTRANGE 的旧方式
	TEXTRANGE tr;
	tr.chrg.cpMin = sel.cpMin;
	tr.chrg.cpMax = sel.cpMax;
	tr.lpstrText = pText;

	LRESULT charsCopied = _edit_sendmessage(pObj, EM_GETTEXTRANGE, 0, (LPARAM)&tr, &sOK);

	INT resultCap = bufChars;
	LPWSTR pResult = (LPWSTR)Ex_MemAlloc(resultCap * sizeof(WCHAR));
	INT resultPos = 0;

	for (INT i = 0; pText[i] != L'\0' && resultPos < resultCap - 2; i++) {
		if (pText[i] != L'\x200B') {
			pResult[resultPos++] = pText[i];
		}
	}
	pResult[resultPos] = L'\0';

	INT copyLen = __min(resultPos, nBufSize - 1);
	if (pBuffer && nBufSize > 0) {
		memcpy(pBuffer, pResult, copyLen * sizeof(WCHAR));
		pBuffer[copyLen] = L'\0';
	}
	Ex_MemFree(pResult);
	Ex_MemFree(pText);
	return resultPos;
}

// ============================================================
// 设置初始文本
// ============================================================
void _flowgraphedit_setinittext(HEXOBJ hObj, obj_s* pObj, LPCWSTR pszText) {
	if (!pObj || !pszText) return;
	BOOL sOK;

	// 1. 清空现有文本
	_edit_sendmessage(pObj, EM_SETSEL, 0, -1, &sOK);
	_edit_sendmessage(pObj, EM_REPLACESEL, TRUE, (LPARAM)L"", &sOK);

	// ★ 2. 清空文本后，强制重置默认格式，防止后续插入的普通文本继承旧的颜色/链接属性
	CHARFORMAT2W cfClear;
	memset(&cfClear, 0, sizeof(cfClear));
	cfClear.cbSize = sizeof(cfClear);
	cfClear.dwMask = CFM_COLOR | CFM_LINK | CFM_UNDERLINE;
	cfClear.dwEffects = 0;
	_edit_sendmessage(pObj, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cfClear, &sOK); // 对全部默认格式应用

	// 3. 禁用事件通知防止闪烁
	DWORD oldMask = (DWORD)_edit_sendmessage(pObj, EM_GETEVENTMASK, 0, 0, &sOK);
	_edit_sendmessage(pObj, EM_SETEVENTMASK, 0, 0, &sOK);

	INT i = 0;
	INT len = lstrlenW(pszText);
	WCHAR szNormal[1024];
	INT nNormalLen = 0;

	auto FlushNormal = [&]() {
		if (nNormalLen > 0) {
			szNormal[nNormalLen] = L'\0';
			_edit_sendmessage(pObj, EM_REPLACESEL, TRUE, (LPARAM)szNormal, &sOK);
			nNormalLen = 0;
		}
		};

	while (i < len) {
		if (pszText[i] == L'@' && i + 1 < len && pszText[i + 1] == L'{') {
			FlushNormal();
			INT j = i + 2;
			WCHAR szName[256] = { 0 };
			INT nNameLen = 0;
			while (j < len && pszText[j] != L'}' && nNameLen < 255) {
				szName[nNameLen++] = pszText[j++];
			}
			if (j < len && pszText[j] == L'}') {
				szName[nNameLen] = L'\0';
				WCHAR szInsert[512];
				swprintf_s(szInsert, L"@{%s}\x200B", szName);
				INT nameLen = 3 + nNameLen;
				INT totalLen = lstrlenW(szInsert);

				CHARRANGE cr;
				_edit_sendmessage(pObj, EM_EXGETSEL, 0, (LPARAM)&cr, &sOK);
				INT nInsertPos = cr.cpMin;
				_edit_sendmessage(pObj, EM_REPLACESEL, TRUE, (LPARAM)szInsert, &sOK);

				_edit_sendmessage(pObj, EM_SETSEL, nInsertPos, nInsertPos + nameLen, &sOK);
				CHARFORMAT2W cf;
				memset(&cf, 0, sizeof(cf)); cf.cbSize = sizeof(cf);
				cf.dwMask = CFM_COLOR | CFM_LINK | CFM_UNDERLINE;
				cf.dwEffects = (DWORD)CFE_LINK | CFE_UNDERLINE;
				cf.crTextColor = RGB(30, 120, 255);
				_edit_sendmessage(pObj, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf, &sOK);

				_edit_sendmessage(pObj, EM_SETSEL, nInsertPos + nameLen, nInsertPos + totalLen, &sOK);
				CHARFORMAT2W cfDefault;
				memset(&cfDefault, 0, sizeof(cfDefault)); cfDefault.cbSize = sizeof(cfDefault);
				cfDefault.dwMask = CFM_COLOR | CFM_LINK | CFM_UNDERLINE;
				cfDefault.dwEffects = 0;
				_edit_sendmessage(pObj, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfDefault, &sOK);

				_edit_sendmessage(pObj, EM_SETSEL, nInsertPos + totalLen, nInsertPos + totalLen, &sOK);
				i = j + 1;
			}
			else {
				if (nNormalLen >= 1023) FlushNormal();
				szNormal[nNormalLen++] = pszText[i++];
			}
		}
		else {
			if (nNormalLen >= 1023) FlushNormal();
			szNormal[nNormalLen++] = pszText[i++];
		}
	}
	FlushNormal();

	// ★ 4. 插入完毕后，将光标移动到最末尾，并再次强制重置光标格式
	CHARRANGE crEnd = { -1, -1 };
	_edit_sendmessage(pObj, EM_EXSETSEL, 0, (LPARAM)&crEnd, &sOK);

	CHARFORMAT2W cfReset;
	memset(&cfReset, 0, sizeof(cfReset));
	cfReset.cbSize = sizeof(cfReset);
	cfReset.dwMask = CFM_COLOR | CFM_LINK | CFM_UNDERLINE;
	cfReset.dwEffects = 0;
	_edit_sendmessage(pObj, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfReset, &sOK);

	_edit_sendmessage(pObj, EM_SETEVENTMASK, 0, oldMask, &sOK);
}
// ============================================================
// 绘制浮窗
// ============================================================
void _flowgraphedit_drawpanel(HEXOBJ hObj, FLOWGRAPHEDIT_PRIV* pPriv, EX_PAINTSTRUCT* ps, obj_s* pObj) {
	if (!pPriv->bPanelActive || pPriv->nFilteredCount == 0) return;
	BOOL sOK;
	POINT ptAt;
	_edit_sendmessage(pObj, EM_POSFROMCHAR, (WPARAM)&ptAt, pPriv->nAtCharPos, &sOK);
	edit_s* pEdit = (edit_s*)_obj_pOwner(pObj);
	RECT* prcText = pEdit->prctext_;
	// ★★★ 核心修复：抛弃 FLOWGRAPHEDIT_PANEL_MAX_VIS，动态计算实际显示数量（上限20） ★★★
	INT actualVisCount = __min(pPriv->nFilteredCount, 20);
	FLOAT panelW = (FLOAT)FLOWGRAPHEDIT_PANEL_WIDTH;
	FLOAT panelH = (FLOAT)(actualVisCount * FLOWGRAPHEDIT_PANEL_ITEM_H + 8);
	FLOAT px = (FLOAT)(prcText->left + ptAt.x);
	FLOAT py = (FLOAT)(prcText->top + ptAt.y + 22);
	if (px + panelW > (FLOAT)ps->uWidth - 4) px = (FLOAT)ps->uWidth - panelW - 4;
	if (px < 4) px = 4;
	if (py + panelH > (FLOAT)ps->uHeight - 4) py = (FLOAT)(prcText->top + ptAt.y) - panelH - 4;
	if (py < 4) py = 4;
	HEXBRUSH hBrushBg = _brush_create(ExARGB(38, 38, 52, 248));
	HEXBRUSH hBrushBorder = _brush_create(ExARGB(60, 140, 255, 220));
	HEXBRUSH hBrushHover = _brush_create(ExARGB(55, 130, 240, 180));
	HEXFONT  hFont = _font_createfromfamily(L"微软雅黑", 12, 0);
	_canvas_fillroundedrect(ps->hCanvas, hBrushBg, px, py, px + panelW, py + panelH, FLOWGRAPHEDIT_PANEL_RADIUS, FLOWGRAPHEDIT_PANEL_RADIUS);
	_canvas_drawroundedrect(ps->hCanvas, hBrushBorder, px, py, px + panelW, py + panelH, FLOWGRAPHEDIT_PANEL_RADIUS, FLOWGRAPHEDIT_PANEL_RADIUS, 1.5f, 0);
	INT startItem = pPriv->nPanelScroll;
	INT maxItems = __min(pPriv->nFilteredCount - startItem, actualVisCount);
	for (INT i = 0; i < maxItems; i++) {
		INT itemIdx = startItem + i;
		FLOAT itemY = py + 4 + (FLOAT)i * FLOWGRAPHEDIT_PANEL_ITEM_H;
		if (itemIdx == pPriv->nPanelHover) {
			_canvas_fillrect(ps->hCanvas, hBrushHover, px + 3, itemY, px + panelW - 3, itemY + FLOWGRAPHEDIT_PANEL_ITEM_H);
		}
		INT matIdx = pPriv->pFilteredIdx[itemIdx];
		EXARGB txtClr = (itemIdx == pPriv->nPanelHover) ? ExARGB(255, 255, 255, 255) : ExARGB(200, 205, 220, 255);
		_canvas_drawtext(ps->hCanvas, hFont, txtClr, pPriv->pItems[matIdx], -1,
			DT_LEFT | DT_VCENTER | DT_SINGLELINE,
			px + 12, itemY, px + panelW - 12, itemY + FLOWGRAPHEDIT_PANEL_ITEM_H);
	}
	_font_destroy(hFont);
	_brush_destroy(hBrushHover);
	_brush_destroy(hBrushBorder);
	_brush_destroy(hBrushBg);
}
// ============================================================
// 面板命中测试
// ============================================================
BOOL _flowgraphedit_panelhittest(HEXOBJ hObj, obj_s* pObj, FLOWGRAPHEDIT_PRIV* pPriv, INT x, INT y, INT* pItemIdx) {
	if (!pPriv->bPanelActive || pPriv->nFilteredCount == 0 || !pObj) return FALSE;
	BOOL sOK;
	POINT ptAt;
	_edit_sendmessage(pObj, EM_POSFROMCHAR, (WPARAM)&ptAt, pPriv->nAtCharPos, &sOK);
	edit_s* pEdit = (edit_s*)_obj_pOwner(pObj);
	RECT* prcText = pEdit->prctext_;
	FLOAT panelX = (FLOAT)(prcText->left + ptAt.x);
	FLOAT panelY = (FLOAT)(prcText->top + ptAt.y + 22);
	INT actualVisCount = __min(pPriv->nFilteredCount, 20);
	FLOAT panelW = (FLOAT)FLOWGRAPHEDIT_PANEL_WIDTH;
	FLOAT panelH = (FLOAT)(actualVisCount * FLOWGRAPHEDIT_PANEL_ITEM_H + 8);
	if (panelX + panelW > (FLOAT)pObj->w_right_ - pObj->w_left_ - 4) panelX = (FLOAT)(pObj->w_right_ - pObj->w_left_) - panelW - 4;
	if (panelX < 4) panelX = 4;
	if (panelY + panelH > (FLOAT)pObj->w_bottom_ - pObj->w_top_ - 4) panelY = (FLOAT)(prcText->top + ptAt.y) - panelH - 4;
	if (panelY < 4) panelY = 4;
	if (x >= (INT)panelX && x <= (INT)(panelX + panelW) && y >= (INT)panelY && y <= (INT)(panelY + panelH)) {
		INT relY = (INT)(y - panelY - 4);
		if (relY >= 0) {
			INT idx = relY / FLOWGRAPHEDIT_PANEL_ITEM_H;
			if (idx < actualVisCount && pItemIdx) *pItemIdx = pPriv->nPanelScroll + idx;
		}
		return TRUE;
	}
	return FALSE;
}
// ============================================================
// 主过程函数
// ============================================================
LRESULT CALLBACK _flowgraphedit_proc(HWND hWnd, HEXOBJ hObj, INT uMsg, WPARAM wParam, LPARAM lParam) {
	INT nError = 0;
	obj_s* pObj = nullptr;
	if (uMsg == WM_CREATE) {
		LRESULT lr = Ex_ObjCallProc(m_pfnFlowGraphEditProc, hWnd, hObj, uMsg, wParam, lParam);
		FLOWGRAPHEDIT_PRIV* pPriv = (FLOWGRAPHEDIT_PRIV*)Ex_MemAlloc(sizeof(FLOWGRAPHEDIT_PRIV));
		if (pPriv) {
			memset(pPriv, 0, sizeof(FLOWGRAPHEDIT_PRIV));
			pPriv->nPanelHover = -1;
			Ex_ObjSetLong(hObj, OBJECT_LONG_USERDATA, (LONG_PTR)pPriv);
		}
		return lr;
	}
	else if (uMsg == WM_DESTROY) {
		FLOWGRAPHEDIT_PRIV* pPriv = _fgeditpriv(hObj);
		if (pPriv) {
			_flowgraphedit_clear(pPriv);
			if (pPriv->pItems) Ex_MemFree(pPriv->pItems);
			if (pPriv->pFilteredIdx) Ex_MemFree(pPriv->pFilteredIdx);
			Ex_MemFree(pPriv);
			Ex_ObjSetLong(hObj, OBJECT_LONG_USERDATA, 0);
		}
		return Ex_ObjCallProc(m_pfnFlowGraphEditProc, hWnd, hObj, uMsg, wParam, lParam);
	}
	else if (uMsg == FLOWGRAPHEDIT_MESSAGE_ADDITEM) {
		FLOWGRAPHEDIT_PRIV* pPriv = _fgeditpriv(hObj);
		if (pPriv && lParam) {
			FLOWGRAPHEDIT_ITEM* pItem = (FLOWGRAPHEDIT_ITEM*)lParam;
			return _flowgraphedit_add(pPriv, pItem->szName);
		}
		return FALSE;
	}
	else if (uMsg == FLOWGRAPHEDIT_MESSAGE_CLEARITEMS) {
		FLOWGRAPHEDIT_PRIV* pPriv = _fgeditpriv(hObj);
		if (pPriv) _flowgraphedit_clear(pPriv);
		return 0;
	}
	else if (uMsg == FLOWGRAPHEDIT_MESSAGE_SHOWPANEL) {
		FLOWGRAPHEDIT_PRIV* pPriv = _fgeditpriv(hObj);
		if (pPriv) _flowgraphedit_showpanel(hObj, pPriv);
		return 0;
	}
	else if (uMsg == FLOWGRAPHEDIT_MESSAGE_GETPLAINTEXT) {
		if (_handle_validate(hObj, HT_OBJECT, (LPVOID*)&pObj, &nError))
			return _flowgraphedit_getplaintext(hObj, pObj, (LPWSTR)lParam, (INT)wParam);
		return 0;
	}
	else if (uMsg == FLOWGRAPHEDIT_MESSAGE_SETINITTEXT) {
		if (_handle_validate(hObj, HT_OBJECT, (LPVOID*)&pObj, &nError))
			_flowgraphedit_setinittext(hObj, pObj, (LPCWSTR)lParam);
		return 0;
	}
	else if (uMsg == WM_CHAR) {
		FLOWGRAPHEDIT_PRIV* pPriv = _fgeditpriv(hObj);
		if (pPriv && pPriv->bPanelActive) {
			WCHAR ch = (WCHAR)wParam;
			if (ch == VK_ESCAPE) {
				_flowgraphedit_hidepanel(hObj, pPriv);
				return 0;
			}
			else if (ch == VK_RETURN) {
				if (pPriv->nPanelHover >= 0 && pPriv->nPanelHover < pPriv->nFilteredCount) {
					if (_handle_validate(hObj, HT_OBJECT, (LPVOID*)&pObj, &nError))
						_flowgraphedit_insert(hObj, pObj, pPriv, pPriv->pFilteredIdx[pPriv->nPanelHover]);
				}
				return 0;
			}
			else if (ch == L'\b') {
				if (pPriv->nFilterLen > 0) {
					pPriv->nFilterLen--;
					pPriv->szFilter[pPriv->nFilterLen] = L'\0';
					_flowgraphedit_filter(pPriv);
					Ex_ObjInvalidateRect(hObj, 0);
				}
				else {
					_flowgraphedit_hidepanel(hObj, pPriv);
				}
				return Ex_ObjCallProc(m_pfnFlowGraphEditProc, hWnd, hObj, uMsg, wParam, lParam);
			}
			else if (ch >= 0x20 && pPriv->nFilterLen < 255) {
				pPriv->szFilter[pPriv->nFilterLen++] = ch;
				pPriv->szFilter[pPriv->nFilterLen] = L'\0';
				_flowgraphedit_filter(pPriv);
				Ex_ObjInvalidateRect(hObj, 0);
				return Ex_ObjCallProc(m_pfnFlowGraphEditProc, hWnd, hObj, uMsg, wParam, lParam);
			}
			return 0;
		}
		if (wParam == L'@') {
			LRESULT lr = Ex_ObjCallProc(m_pfnFlowGraphEditProc, hWnd, hObj, uMsg, wParam, lParam);
			if (_handle_validate(hObj, HT_OBJECT, (LPVOID*)&pObj, &nError)) {
				FLOWGRAPHEDIT_PRIV* pPriv2 = _fgeditpriv(hObj);
				if (pPriv2) {
					BOOL sOK;
					LRESULT selEnd = _edit_sendmessage(pObj, EM_GETSEL, 0, 0, &sOK);
					pPriv2->nAtCharPos = (INT)LOWORD(selEnd) - 1;
					pPriv2->nFilterLen = 0;
					pPriv2->szFilter[0] = L'\0';
					Ex_ObjDispatchNotify(hObj, FLOWGRAPHEDIT_EVENT_AT_TRIGGERED, 0, 0);
				}
			}
			return lr;
		}
		return Ex_ObjCallProc(m_pfnFlowGraphEditProc, hWnd, hObj, uMsg, wParam, lParam);
	}
	else if (uMsg == WM_KEYDOWN) {
		FLOWGRAPHEDIT_PRIV* pPriv = _fgeditpriv(hObj);
		// ★★★ 核心修复：拦截 Ctrl+C/V/X，阻止 RichEdit OLE 剪贴板操作 ★★★
   // 原因：RichEdit 内部通过 WM_KEYDOWN 直接处理 Ctrl+C/V/X，走 OLE 剪贴板路径，
   // 1) OLE 粘贴看到 CF_HDROP 时会将文件作为对象插入编辑框
   // 2) OLE 复制可能因剪贴板竞争而静默失败
   // 必须在此拦截，路由到自定义的纯 Win32 剪贴板处理器
		BOOL ctrlDown = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
		if (ctrlDown && !(pPriv && pPriv->bPanelActive)) {
			if (wParam == 'C') {
				Ex_ObjSendMessage(hObj, FLOWGRAPHEDIT_MESSAGE_FORCECOPY, 0, 0);
				return 0;
			}
			else if (wParam == 'X') {
				Ex_ObjSendMessage(hObj, FLOWGRAPHEDIT_MESSAGE_FORCECUT, 0, 0);
				return 0;
			}
			else if (wParam == 'V') {
				Ex_ObjSendMessage(hObj, FLOWGRAPHEDIT_MESSAGE_FORCEPASTE, 0, 0);
				return 0;
			}
		}
		if (pPriv && pPriv->bPanelActive) {
			if (wParam == VK_UP) {
				if (pPriv->nPanelHover > 0) pPriv->nPanelHover--;
				Ex_ObjInvalidateRect(hObj, 0);
				return 0;
			}
			else if (wParam == VK_DOWN) {
				if (pPriv->nPanelHover < pPriv->nFilteredCount - 1) pPriv->nPanelHover++;
				Ex_ObjInvalidateRect(hObj, 0);
				return 0;
			}
			else if (wParam == VK_RETURN) {
				if (pPriv->nPanelHover >= 0 && pPriv->nPanelHover < pPriv->nFilteredCount) {
					if (_handle_validate(hObj, HT_OBJECT, (LPVOID*)&pObj, &nError))
						_flowgraphedit_insert(hObj, pObj, pPriv, pPriv->pFilteredIdx[pPriv->nPanelHover]);
				}
				return 0;
			}
			else if (wParam == VK_ESCAPE) {
				_flowgraphedit_hidepanel(hObj, pPriv);
				return 0;
			}
		}
		return Ex_ObjCallProc(m_pfnFlowGraphEditProc, hWnd, hObj, uMsg, wParam, lParam);
	}
	else if (uMsg == WM_COPY || uMsg == WM_CUT) {
		if (_handle_validate(hObj, HT_OBJECT, (LPVOID*)&pObj, &nError)) {
			INT len = _flowgraphedit_getselectedplaintext(hObj, pObj, NULL, 0);
			if (len > 0) {
				LPWSTR pText = (LPWSTR)Ex_MemAlloc((len + 1) * sizeof(WCHAR));
				if (pText) {
					_flowgraphedit_getselectedplaintext(hObj, pObj, pText, len + 1);
			
					BOOL isClipboardOpen = FALSE;
					for (int retry = 0; retry < 30; retry++) {
						if (OpenClipboard(hWnd)) {  
							isClipboardOpen = TRUE;
							break;
						}
						Sleep(10);
					}
					if (isClipboardOpen) {
						EmptyClipboard();
						HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(WCHAR));
						if (hMem) {
							LPWSTR pMem = (LPWSTR)GlobalLock(hMem);
							memcpy(pMem, pText, (len + 1) * sizeof(WCHAR));
							GlobalUnlock(hMem);
							SetClipboardData(CF_UNICODETEXT, hMem);
						}
						CloseClipboard();
					}
					Ex_MemFree(pText);
				}
			}
			if (uMsg == WM_CUT) {
				BOOL sOK;
				_edit_sendmessage(pObj, EM_REPLACESEL, TRUE, (LPARAM)L"", &sOK);
			}
		}
		return 0;
	}
	else if (uMsg == FLOWGRAPHEDIT_MESSAGE_FORCECOPY || uMsg == FLOWGRAPHEDIT_MESSAGE_FORCECUT) {
		// ★★★ 与 WM_COPY/WM_CUT 相同的自定义剪贴板逻辑，但通过自定义消息路由 ★★★
		// 解决：Ex_ObjSendMessage(WM_COPY) 可能被 RichEdit 内部 OLE 截获导致自定义处理器不被调用
		if (_handle_validate(hObj, HT_OBJECT, (LPVOID*)&pObj, &nError)) {
			INT len = _flowgraphedit_getselectedplaintext(hObj, pObj, NULL, 0);
			if (len > 0) {
				LPWSTR pText = (LPWSTR)Ex_MemAlloc((len + 1) * sizeof(WCHAR));
				if (pText) {
					_flowgraphedit_getselectedplaintext(hObj, pObj, pText, len + 1);
				
					BOOL isClipboardOpen = FALSE;
					for (int retry = 0; retry < 30; retry++) {
						if (OpenClipboard(hWnd)) {
							isClipboardOpen = TRUE;
							break;
						}
						Sleep(10);
					}
					if (isClipboardOpen) {
						EmptyClipboard();
						HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(WCHAR));
						if (hMem) {
							LPWSTR pMem = (LPWSTR)GlobalLock(hMem);
							memcpy(pMem, pText, (len + 1) * sizeof(WCHAR));
							GlobalUnlock(hMem);
							SetClipboardData(CF_UNICODETEXT, hMem);
						}
						CloseClipboard();
					}
					Ex_MemFree(pText);
				}
			}
			if (uMsg == FLOWGRAPHEDIT_MESSAGE_FORCECUT) {
				BOOL sOK;
				_edit_sendmessage(pObj, EM_REPLACESEL, TRUE, (LPARAM)L"", &sOK);
			}
		}
		return 0;
	}
	else if (uMsg == WM_PASTE || uMsg == FLOWGRAPHEDIT_MESSAGE_FORCEPASTE) {
		// ★★★ 新增：自定义粘贴，手动读取 CF_UNICODETEXT，彻底绕过 RichEdit OLE ★★★
		// 解决：剪贴板含 CF_HDROP(DLL文件) 时 OLE 粘贴会将文件作为对象插入编辑框
		if (_handle_validate(hObj, HT_OBJECT, (LPVOID*)&pObj, &nError)) {
			BOOL isClipboardOpen = FALSE;
			for (int retry = 0; retry < 30; retry++) {
				if (OpenClipboard(hWnd)) {
					isClipboardOpen = TRUE;
					break;
					
				}
				Sleep(10);
			}
			if (isClipboardOpen) {
				HANDLE hData = GetClipboardData(CF_UNICODETEXT);
				if (hData) {
					LPCWSTR pClipText = (LPCWSTR)GlobalLock(hData);
					if (pClipText) {
						// 过滤零宽空格，防止重复粘贴带格式的脏数据
						INT len = lstrlenW(pClipText);
						LPWSTR pClean = (LPWSTR)Ex_MemAlloc((len + 1) * sizeof(WCHAR));
						INT pos = 0;
						for (INT k = 0; k < len; k++) {
							if (pClipText[k] != L'\x200B') {
								pClean[pos++] = pClipText[k];
							}
						}
						pClean[pos] = L'\0';

						// 删除当前选中的文本 (如果有)
						BOOL sOK;
						CHARRANGE cr;
						_edit_sendmessage(pObj, EM_EXGETSEL, 0, (LPARAM)&cr, &sOK);
						if (cr.cpMin != cr.cpMax) {
							_edit_sendmessage(pObj, EM_REPLACESEL, TRUE, (LPARAM)L"", &sOK);
						}

						// 插入格式化文本 (完美支持 @{素材} 高亮恢复)
						_flowgraphedit_insert_formatted(hObj, pObj, pClean);

						Ex_MemFree(pClean);
						GlobalUnlock(hData);
					}
				}
				CloseClipboard();
			}
		}
		return 0; // ★ 拦截，绝不走基类 OLE 粘贴
	}
	else if (uMsg == WM_LBUTTONDOWN) {
		FLOWGRAPHEDIT_PRIV* pPriv = _fgeditpriv(hObj);
		INT x = GET_X_LPARAM(lParam);
		INT y = GET_Y_LPARAM(lParam);
		if (pPriv && pPriv->bPanelActive) {
			INT hitIdx = -1;
			if (_handle_validate(hObj, HT_OBJECT, (LPVOID*)&pObj, &nError)) {
				if (_flowgraphedit_panelhittest(hObj, pObj, pPriv, x, y, &hitIdx)) {
					if (hitIdx >= 0 && hitIdx < pPriv->nFilteredCount)
						_flowgraphedit_insert(hObj, pObj, pPriv, pPriv->pFilteredIdx[hitIdx]);
					return 0;
				}
			}
			_flowgraphedit_hidepanel(hObj, pPriv);
		}
		return Ex_ObjCallProc(m_pfnFlowGraphEditProc, hWnd, hObj, uMsg, wParam, lParam);
	}
	else if (uMsg == WM_MOUSEMOVE) {
		FLOWGRAPHEDIT_PRIV* pPriv = _fgeditpriv(hObj);
		INT x = GET_X_LPARAM(lParam);
		INT y = GET_Y_LPARAM(lParam);
		if (pPriv && pPriv->bPanelActive) {
			INT hitIdx = -1;
			if (_handle_validate(hObj, HT_OBJECT, (LPVOID*)&pObj, &nError)) {
				if (_flowgraphedit_panelhittest(hObj, pObj, pPriv, x, y, &hitIdx)) {
					if (pPriv->nPanelHover != hitIdx) {
						pPriv->nPanelHover = hitIdx;
						Ex_ObjInvalidateRect(hObj, 0);
					}
				}
				else {
					if (pPriv->nPanelHover != -1) {
						pPriv->nPanelHover = -1;
						Ex_ObjInvalidateRect(hObj, 0);
					}
				}
			}
		}
		return Ex_ObjCallProc(m_pfnFlowGraphEditProc, hWnd, hObj, uMsg, wParam, lParam);
	}
	else if (uMsg == WM_MOUSEWHEEL) {
		FLOWGRAPHEDIT_PRIV* pPriv = _fgeditpriv(hObj);
		if (pPriv && pPriv->bPanelActive) {
			SHORT delta = GET_WHEEL_DELTA_WPARAM(wParam);
			// 获取当前面板实际可见的行数
			INT actualVisCount = __min(pPriv->nFilteredCount, 20);

			if (delta > 0) { // 鼠标滚轮向上
				if (pPriv->nPanelScroll > 0) {
					pPriv->nPanelScroll--;
					Ex_ObjInvalidateRect(hObj, 0); // 刷新面板
				}
			}
			else { // 鼠标滚轮向下
				// 确保底部不越界
				if (pPriv->nPanelScroll + actualVisCount < pPriv->nFilteredCount) {
					pPriv->nPanelScroll++;
					Ex_ObjInvalidateRect(hObj, 0); // 刷新面板
				}
			}
			return 0; // 拦截消息，不传递给基类 Edit
		}
		}
	else if (uMsg == WM_PAINT) {
		if (_handle_validate(hObj, HT_OBJECT, (LPVOID*)&pObj, &nError)) {
			EX_PAINTSTRUCT ps{ 0 };
			if (Ex_ObjBeginPaint(hObj, &ps)) {
				// 1. 绘制基类 Edit 背景、文字和光标
				if (ps.dwOwnerData) {
					// 原有绘制逻辑，保持RichEdit基础绘制
					LPVOID pITS = ((edit_s*)ps.dwOwnerData)->its_;
					INT atom;
					if ((ps.dwState & STATE_FOCUS) != 0) atom = ATOM_FOCUS;
					else if ((ps.dwState & STATE_HOVER) != 0) atom = ATOM_HOVER;
					else atom = ATOM_NORMAL;
					if ((ps.dwStyleEx & OBJECT_STYLE_EX_CUSTOMDRAW) == 0) {
						Ex_ThemeDrawControl(ps.hTheme, ps.hCanvas, 0, 0, ps.uWidth, ps.uHeight, ATOM_EDIT, atom, 255);
					}
					LPCWSTR lpBanner = ((edit_s*)ps.dwOwnerData)->pBanner_;
					if (lpBanner != 0 && (pITS == 0 || _edit_getlen(pObj) == 0)) {
						if (!((ps.dwState & STATE_FOCUS) != 0 && (ps.dwStyle & EDIT_STYLE_SHOWTIPSALWAYS) == 0)) {
							RECT* rcText = ((edit_s*)ps.dwOwnerData)->prctext_;
							INT dt = 0;
							if ((pObj->dwTextFormat_ & DT_SINGLELINE) == DT_SINGLELINE) dt = DT_VCENTER;
							_canvas_drawtext(ps.hCanvas, pObj->hFont_, ((edit_s*)ps.dwOwnerData)->crBanner_, lpBanner, -1, dt, rcText->left, rcText->top, rcText->right, rcText->bottom);
						}
					}
					if (pITS != 0) {
						RECT rcTmp{ 0 };
						IntersectRect(&rcTmp, (RECT*)&ps.rcText.left, (RECT*)&ps.rcPaint.left);
						HDC mDc = ((edit_s*)ps.dwOwnerData)->mDc_;
						wnd_s* pWnd = pObj->pWnd_;
						BOOL ismove = (pWnd->base.dwFlags_ & EWF_BSIZEMOVING) == EWF_BSIZEMOVING;
						HDC hDc = _canvas_getdc(ps.hCanvas);
						_edit_txpaint(pITS, DVASPECT_CONTENT, 0, NULL, NULL, hDc, NULL, NULL, NULL, &rcTmp, NULL, ismove ? TXTVIEW_INACTIVE : TXTVIEW_ACTIVE);
						BitBlt(hDc, rcTmp.left, rcTmp.top, rcTmp.right - rcTmp.left, rcTmp.bottom - rcTmp.top, mDc, 0, 0, SRCPAINT);
						_canvas_releasedc(ps.hCanvas);
						if (!((pObj->dwStyle_ & EDIT_STYLE_HIDDENCARET) == EDIT_STYLE_HIDDENCARET)) {
							if (!((((edit_s*)ps.dwOwnerData)->flags_ & EDIT_FLAG_BSELECTED) == EDIT_FLAG_BSELECTED)) {
								if ((((edit_s*)ps.dwOwnerData)->flags_ & EDIT_FLAG_BCARETCONTEXT) == EDIT_FLAG_BCARETCONTEXT) {
									if (!((((edit_s*)ps.dwOwnerData)->flags_ & EDIT_FLAG_BCARETSHHOWED) == EDIT_FLAG_BCARETSHHOWED)) {
										rcTmp.left = ((edit_s*)ps.dwOwnerData)->rcCaret_left_;
										rcTmp.top = ((edit_s*)ps.dwOwnerData)->rcCaret_top_;
										rcTmp.right = ((edit_s*)ps.dwOwnerData)->rcCaret_right_;
										rcTmp.bottom = ((edit_s*)ps.dwOwnerData)->rcCaret_bottom_;
										if (rcTmp.right > 0 && rcTmp.bottom > 0) {
											HEXBRUSH hCaretBrush = _brush_create(((edit_s*)ps.dwOwnerData)->crCaret_);
											_canvas_fillrect(ps.hCanvas, hCaretBrush, (FLOAT)rcTmp.left, (FLOAT)rcTmp.top, (FLOAT)rcTmp.right, (FLOAT)rcTmp.bottom);
											_brush_destroy(hCaretBrush);
										}
									}
								}
							}
						}
					}
				}
				// 2. 绘制浮窗
				FLOWGRAPHEDIT_PRIV* pPriv = _fgeditpriv(hObj);
				if (pPriv && pPriv->bPanelActive) {
					_flowgraphedit_drawpanel(hObj, pPriv, &ps, pObj);
				}
				Ex_ObjEndPaint(hObj, &ps);
			}
		}
		return 0;
	}
	else if (uMsg == WM_VSCROLL || uMsg == WM_HSCROLL || uMsg == WM_KILLFOCUS || uMsg == WM_SIZE) {
		FLOWGRAPHEDIT_PRIV* pPriv = _fgeditpriv(hObj);
		if (pPriv && pPriv->bPanelActive) _flowgraphedit_hidepanel(hObj, pPriv);
		return Ex_ObjCallProc(m_pfnFlowGraphEditProc, hWnd, hObj, uMsg, wParam, lParam);
	}
	return Ex_ObjCallProc(m_pfnFlowGraphEditProc, hWnd, hObj, uMsg, wParam, lParam);
}

void _flowgraphedit_insert_formatted(HEXOBJ hObj, obj_s* pObj, LPCWSTR pszText) {
	if (!pObj || !pszText) return;
	BOOL sOK;
	DWORD oldMask = (DWORD)_edit_sendmessage(pObj, EM_GETEVENTMASK, 0, 0, &sOK);
	_edit_sendmessage(pObj, EM_SETEVENTMASK, 0, 0, &sOK);

	INT i = 0;
	INT len = lstrlenW(pszText);
	WCHAR szNormal[1024];
	INT nNormalLen = 0;

	auto FlushNormal = [&]() {
		if (nNormalLen > 0) {
			szNormal[nNormalLen] = L'\0';
			_edit_sendmessage(pObj, EM_REPLACESEL, TRUE, (LPARAM)szNormal, &sOK);
			nNormalLen = 0;
		}
		};

	while (i < len) {
		if (pszText[i] == L'@' && i + 1 < len && pszText[i + 1] == L'{') {
			FlushNormal();
			INT j = i + 2;
			WCHAR szName[256] = { 0 };
			INT nNameLen = 0;
			while (j < len && pszText[j] != L'}' && nNameLen < 255) {
				szName[nNameLen++] = pszText[j++];
			}
			if (j < len && pszText[j] == L'}') {
				szName[nNameLen] = L'\0';
				WCHAR szInsert[512];
				swprintf_s(szInsert, L"@{%s}\x200B", szName);
				INT nameLen = 3 + nNameLen;
				INT totalLen = lstrlenW(szInsert);

				CHARRANGE cr;
				_edit_sendmessage(pObj, EM_EXGETSEL, 0, (LPARAM)&cr, &sOK);
				INT nInsertPos = cr.cpMin;

				_edit_sendmessage(pObj, EM_REPLACESEL, TRUE, (LPARAM)szInsert, &sOK);

				// 设置链接格式 (蓝色高亮)
				_edit_sendmessage(pObj, EM_SETSEL, nInsertPos, nInsertPos + nameLen, &sOK);
				CHARFORMAT2W cf;
				memset(&cf, 0, sizeof(cf)); cf.cbSize = sizeof(cf);
				cf.dwMask = CFM_COLOR | CFM_LINK | CFM_UNDERLINE;
				cf.dwEffects = (DWORD)CFE_LINK | CFE_UNDERLINE;
				cf.crTextColor = RGB(30, 120, 255);
				_edit_sendmessage(pObj, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf, &sOK);

				// 恢复默认格式
				_edit_sendmessage(pObj, EM_SETSEL, nInsertPos + nameLen, nInsertPos + totalLen, &sOK);
				CHARFORMAT2W cfDefault;
				memset(&cfDefault, 0, sizeof(cfDefault)); cfDefault.cbSize = sizeof(cfDefault);
				cfDefault.dwMask = CFM_COLOR | CFM_LINK | CFM_UNDERLINE;
				cfDefault.dwEffects = 0;
				_edit_sendmessage(pObj, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfDefault, &sOK);

				_edit_sendmessage(pObj, EM_SETSEL, nInsertPos + totalLen, nInsertPos + totalLen, &sOK);
				i = j + 1;
			}
			else {
				if (nNormalLen >= 1023) FlushNormal();
				szNormal[nNormalLen++] = pszText[i++];
			}
		}
		else {
			if (nNormalLen >= 1023) FlushNormal();
			szNormal[nNormalLen++] = pszText[i++];
		}
	}
	FlushNormal();

	// 强制重置光标格式，防止后续输入变蓝
	CHARFORMAT2W cfReset;
	memset(&cfReset, 0, sizeof(cfReset));
	cfReset.cbSize = sizeof(cfReset);
	cfReset.dwMask = CFM_COLOR | CFM_LINK | CFM_UNDERLINE;
	cfReset.dwEffects = 0;
	_edit_sendmessage(pObj, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfReset, &sOK);

	_edit_sendmessage(pObj, EM_SETEVENTMASK, 0, oldMask, &sOK);
}