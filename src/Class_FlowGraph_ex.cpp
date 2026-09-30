#include "stdafx.h"
#define RYML_SINGLE_HDR_DEFINE_NOW
#include "rapidyaml/rapidyaml.hpp"

void _flowgraph_register() {
	WCHAR wzCls[] = L"FlowGraph";
	Ex_ObjRegister(wzCls, OBJECT_STYLE_VISIBLE | OBJECT_STYLE_HSCROLL | OBJECT_STYLE_VSCROLL,
		OBJECT_STYLE_EX_FOCUSABLE | OBJECT_STYLE_EX_COMPOSITED | OBJECT_STYLE_EX_DRAGDROP | OBJECT_STYLE_EX_ACCEPTFILES,
		0, 4 * sizeof(size_t), NULL, NULL, _flowgraph_proc);
}
LRESULT CALLBACK _flowgraph_proc(HWND hWnd, HEXOBJ hObj, INT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CREATE)
	{
		EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_DATA));
		memset(pData, 0, sizeof(EX_FLOWGRAPH_DATA));
		pData->zoom = 1.0f; pData->selectedNode = -1; pData->draggingNode = -1;
		pData->connectingSlot = -1; pData->connectingNode = -1; pData->hoverNode = -1;
		pData->selectedConnection = -1; pData->selectedPortNode = -1; pData->selectedPortIndex = -1;
		pData->executionDepth = 0; pData->executionPass = 0;
		pData->resizingNode = -1; pData->resizingPortIdx = -1;
		pData->isPanning = FALSE;
		pData->panStartMouseX = 0; pData->panStartMouseY = 0;
		pData->panStartScrollX = 0; pData->panStartScrollY = 0;
		pData->chainContexts = NULL;
		pData->chainContextCount = 0;
		pData->asyncPendingNode = -1;
		pData->nextAutoId = 0;
		pData->hoverTitleNode = -1;  
		pData->sidebarHoverBtn = -1;
		pData->sidebarShowAddPanel = FALSE;
		pData->sidebarAddPanelHover = -1;
		pData->sidebarShowChainPanel = FALSE;
		pData->sidebarChainPanelHover = -1;
		pData->sidebarChainPanelMode = 0;
		pData->videoTimerActive = FALSE;
		pData->videoDragNode = -1;
		pData->videoDragPortIdx = -1;
		pData->customItems = NULL;
		pData->customItemCount = 0;
		pData->sidebarShowCustomPanel = FALSE;
		pData->sidebarCustomPanelHover = -1;
		pData->cardRegistry = NULL;
		pData->cardRegistryCount = 0;
		pData->sidebarShowCancelPanel = FALSE;
		pData->sidebarCancelPanelHover = -1;
		pData->showEditPanel = FALSE;
		pData->editPanelTargetNode = -1;
		pData->editPanelBtnHover = FALSE;
		pData->comboExpandedNode = -1;
		pData->comboExpandedPortIdx = -1;
		pData->comboPanelHoverIdx = -1;
		pData->clipboardNodeId = -1;
		pData->isSelecting = FALSE;
		pData->selectedNodes = NULL;
		pData->selectedNodeCount = 0;
		pData->clipboardNodes = NULL;
		pData->clipboardNodeCount = 0;
		pData->clipboardConnections = NULL;
		pData->clipboardConnectionCount = 0;
		pData->isDraggingSelection = FALSE;
		pData->projectName[0] = L'\0'; // ★ 初始化工程名为空
		HEXOBJ panelEdit = Ex_ObjCreateEx(OBJECT_STYLE_EX_FOCUSABLE | OBJECT_STYLE_EX_COMPOSITED, L"FlowGraphEdit", 0,
			OBJECT_STYLE_VISIBLE | EDIT_STYLE_RICHTEXT | EDIT_STYLE_NEWLINE | EDIT_STYLE_HIDESELECTION | EDIT_STYLE_DISABLEMENU | OBJECT_STYLE_VSCROLL,
			0, 0, 580, 220, hObj, 0, DT_LEFT | DT_TOP, 0, 0, 0);
		Ex_ObjHandleEvent(panelEdit, FLOWGRAPHEDIT_EVENT_AT_TRIGGERED, _flowgraph_edit_at_triggered);
		Ex_ObjShow(panelEdit, FALSE);
		pData->editPanelEdit = panelEdit;
		const char* vlcArgv[] = { "--no-xlib", "--network-caching=500", "--no-video-title-show", "--disable-screensaver" };
		pData->libVlc = libvlc_new(sizeof(vlcArgv) / sizeof(*vlcArgv), vlcArgv);

		Ex_ObjSetLong(hObj, FLOWGRAPH_LONG_DATA, (LONG_PTR)pData);
		Ex_ObjSetLong(hObj, FLOWGRAPH_LONG_MOUSE_X, 0);
		Ex_ObjSetLong(hObj, FLOWGRAPH_LONG_MOUSE_Y, 0);
		Ex_ObjSetLong(hObj, FLOWGRAPH_LONG_BACKGROUNDCOLOR, ExARGB(80, 80, 80, 255));
		
	
		Ex_ObjScrollSetInfo(hObj, SCROLLBAR_TYPE_HORZ, SIF_PAGE | SIF_RANGE | SIF_POS, 0, 1, 1000, 0, TRUE);
		Ex_ObjScrollSetInfo(hObj, SCROLLBAR_TYPE_VERT, SIF_PAGE | SIF_RANGE | SIF_POS, 0, 1, 1000, 0, TRUE);
		Ex_ObjScrollShow(hObj, SCROLLBAR_TYPE_HORZ, TRUE);
		Ex_ObjScrollShow(hObj, SCROLLBAR_TYPE_VERT, TRUE);
	}
	else if (uMsg == WM_DESTROY)
	{
		EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
		if (pData)
		{
			// ===== 新增：释放链式执行上下文 =====
			for (INT i = 0; i < pData->chainContextCount; i++) {
				if (pData->chainContexts[i].nodeOrder)
				{
					Ex_MemFree(pData->chainContexts[i].nodeOrder);
				}
				
			}
			Ex_MemFree(pData->chainContexts);

			for (INT i = 0; i < pData->nodeCount; i++) {
				EX_FLOWGRAPH_NODE* node = &pData->nodes[i];
				Ex_MemFree((void*)node->title);
				for (INT j = 0; j < node->portCount; j++) {
					EX_FLOWGRAPH_PORT* port = &node->ports[j];
					Ex_MemFree((void*)port->name);
					if (port->widgetData != NULL) {
						if (port->widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT) {
							Ex_MemFree(port->widgetData);
						}
						else if (port->widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO) {
							EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)port->widgetData;
							for (INT k = 0; k < combo->count; k++) Ex_MemFree((void*)combo->options[k]);
							Ex_MemFree(combo->options);
							Ex_MemFree(combo);
						}
						else if (port->widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE) {
							_img_destroy((HEXIMAGE)port->widgetData);
							if (port->imagePath) Ex_MemFree((void*)port->imagePath);
						}
						else if (port->widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON) {
							EX_FLOWGRAPH_NODE_BUTTON_DATA* btn = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)port->widgetData;
							Ex_MemFree((void*)btn->caption);
							Ex_MemFree(btn);
						}
						else if (node->ports[j].widgetType == FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON) {
							EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* dualBtn = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)node->ports[j].widgetData;
							Ex_MemFree((void*)dualBtn->caption1);
							Ex_MemFree((void*)dualBtn->caption2);
							Ex_MemFree(dualBtn);
						}
						else if (node->ports[j].widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO) {
							EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)node->ports[j].widgetData;
							if (pVideo) {
								_flowgraph_video_cleanup(pVideo);
								DeleteCriticalSection(&pVideo->critsec);
								Ex_MemFree(pVideo);
							}
						}
						else if (port->widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT) {
							Ex_MemFree(port->widgetData);
						}
					}
				}
				Ex_MemFree(node->ports);
			}
			Ex_MemFree(pData->nodes);
			Ex_MemFree(pData->connections);

			// ===== 释放自定义条目 =====
			if (pData->customItems) {
				for (INT i = 0; i < pData->customItemCount; i++) {
					Ex_MemFree((void*)pData->customItems[i]);
				}
				Ex_MemFree(pData->customItems);
			}
			if (pData->libVlc) { libvlc_release(pData->libVlc); pData->libVlc = NULL; }
			// ===== 释放卡片类型注册表 =====
			for (INT i = 0; i < pData->cardRegistryCount; i++) {
				_flowgraph_free_card_descriptor(&pData->cardRegistry[i]);
			}
			if (pData->cardRegistry) Ex_MemFree(pData->cardRegistry);
			if (pData->selectedNodes) Ex_MemFree(pData->selectedNodes);
			// 释放剪贴板深拷贝数据
			if (pData->clipboardNodes) {
				for (INT i = 0; i < pData->clipboardNodeCount; i++) {
					_flowgraph_free_temp_node_data(&pData->clipboardNodes[i]);
				}
				Ex_MemFree(pData->clipboardNodes);
				pData->clipboardNodes = NULL;
			}
			if (pData->clipboardConnections) { 
				Ex_MemFree(pData->clipboardConnections); 
				pData->clipboardConnections = NULL;
			}
			Ex_MemFree(pData);
		}
	}
	else if (uMsg == WM_PAINT) { _flowgraph_paint(hObj); }
	else if (uMsg == WM_SIZE) { _flowgraph_updatelayout(hObj); }
	else if (uMsg == WM_MOUSEMOVE) { _flowgraph_onmousemove(hObj, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); }
	else if (uMsg == WM_LBUTTONDOWN) { _flowgraph_onlbuttondown(hObj, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); }
	else if (uMsg == WM_LBUTTONUP) { _flowgraph_onlbuttonup(hObj, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); }
	else if (uMsg == WM_RBUTTONDOWN) {
		EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
		if (!pData) return 0;

		INT scrollX = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_HORZ);
		INT scrollY = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_VERT);
		FLOAT virtualX = (GET_X_LPARAM(lParam) + scrollX) / pData->zoom;
		FLOAT virtualY = (GET_Y_LPARAM(lParam) + scrollY) / pData->zoom;

		EX_FLOWGRAPH_NODE* topmostNode = _flowgraph_find_topmost_node_at(pData, virtualX, virtualY);
		if (topmostNode) {
			// 1. 选中该节点
			pData->selectedNode = topmostNode->id;
			_flowgraph_update_edit_panel(hObj);

			// 2. 显示右键菜单
			pData->showContextMenu = TRUE;
			pData->contextMenuNodeId = topmostNode->id;
			pData->contextMenuX = (FLOAT)GET_X_LPARAM(lParam);
			pData->contextMenuY = (FLOAT)GET_Y_LPARAM(lParam);
			pData->contextMenuHover = -1;

			Ex_ObjInvalidateRect(hObj, 0);
			return 1;
		}
		else {
			// 点击空白处，隐藏菜单
			if (pData->showContextMenu) {
				pData->showContextMenu = FALSE;
				Ex_ObjInvalidateRect(hObj, 0);
			}
		}
		}
	else if (uMsg == WM_MBUTTONDOWN) {
		EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
		if (pData) {
	
			pData->isPanning = TRUE;
			pData->panStartMouseX = GET_X_LPARAM(lParam);
			pData->panStartMouseY = GET_Y_LPARAM(lParam);
			pData->panStartScrollX = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_HORZ);
			pData->panStartScrollY = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_VERT);
		}
	}
	else if (uMsg == WM_MBUTTONUP) {
		EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
		if (pData) {
			pData->isPanning = FALSE;
		}
	}
	else if (uMsg == WM_TIMER) {
		EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
		if (pData) {
			_flowgraph_update_video_timers(hObj, pData);
			Ex_ObjInvalidateRect(hObj, 0);
		}
		}
	else if (uMsg == FLOWGRAPH_MESSAGE_VIDEO_REFRESH) {
		Ex_ObjInvalidateRect(hObj, 0); // 在 UI 主线程安全刷新
		return 0;
	}
	else if (uMsg == FLOWGRAPH_MESSAGE_SET_CUSTOM_ITEMS) {
		return _flowgraph_set_custom_items(hObj, (EX_FLOWGRAPH_CUSTOM_ITEMS*)lParam);
	}
	else if (uMsg == WM_MOUSEWHEEL) { if (GetAsyncKeyState(VK_CONTROL) & 32768) { _flowgraph_onmousewheel(hObj, GET_WHEEL_DELTA_WPARAM(wParam)); return 0; } }
	else if (uMsg == WM_KEYDOWN) {
		EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
		if (!pData) return 0;

		BOOL ctrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
		HEXOBJ hFocus = Ex_ObjGetFocus(hObj);
		BOOL isEditFocused = (hFocus == pData->editPanelEdit);

		if (ctrlPressed && pData->showEditPanel && pData->editPanelEdit) {
			if (wParam == 'C' || wParam == 'X') {
				// ★ 修复：使用 EM_EXGETSEL 获取 RichEdit 真实选区，防止 EM_GETSEL 返回 0 导致按键被吞
				CHARRANGE sel = { 0 };
				Ex_ObjSendMessage(pData->editPanelEdit, EM_EXGETSEL, 0, (LPARAM)&sel);
				if (sel.cpMin != sel.cpMax) {
					if (!isEditFocused) Ex_ObjSetFocus(pData->editPanelEdit);
					Ex_ObjSendMessage(pData->editPanelEdit,
						wParam == 'C' ? FLOWGRAPHEDIT_MESSAGE_FORCECOPY : FLOWGRAPHEDIT_MESSAGE_FORCECUT,
						0, 0);
				}
				return 1; // ★ 无论是否有选区，都拦截，防止 RichEdit 默认复制带 OLE 格式
			}
			else if (wParam == 'V') {
				// ★ 修复：无条件拦截 Ctrl+V，强制走自定义纯文本粘贴，彻底杜绝 OLE 粘贴 DLL
				if (!isEditFocused) Ex_ObjSetFocus(pData->editPanelEdit);
				Ex_ObjSendMessage(pData->editPanelEdit, FLOWGRAPHEDIT_MESSAGE_FORCEPASTE, 0, 0);
				return 1;
			}
		}


		// ★ Ctrl+C/ Ctrl+X：复制/剪切选中节点
		if (ctrlPressed && (wParam == 'C' || wParam == 'X')) {
	
			// ★ 智能路由 2：画布多选复制
			if (pData->selectedNodeCount > 0) {
				// ★★★ 剪切前置检查：是否包含受保护节点（忙碌节点无法剪切） ★★★
				if (wParam == 'X') {
					BOOL hasProtected = FALSE;
					for (INT i = 0; i < pData->selectedNodeCount; i++) {
						if (_flowgraph_is_node_protected_by_busy(pData, pData->selectedNodes[i])) {
							hasProtected = TRUE; break;
						}
					}
					if (hasProtected) {
						Ex_MessageBox(hObj, L"忙碌节点所处链路无法剪切", L"警告", MB_ICONWARNING, MESSAGEBOX_FLAG_CENTEWINDOW);
						return 1; // 拦截，不执行复制和删除
					}
				}

				// 1. 清理旧剪贴板
				if (pData->clipboardNodes) {
					for (INT i = 0; i < pData->clipboardNodeCount; i++) _flowgraph_free_temp_node_data(&pData->clipboardNodes[i]);
					Ex_MemFree(pData->clipboardNodes);
				}
				if (pData->clipboardConnections) Ex_MemFree(pData->clipboardConnections);

				// 2. 深拷贝节点
				pData->clipboardNodeCount = pData->selectedNodeCount;
				pData->clipboardNodes = (EX_FLOWGRAPH_NODE*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE) * pData->clipboardNodeCount);
				FLOAT minX = 999999.0f, minY = 999999.0f, maxX = -999999.0f, maxY = -999999.0f;

				for (INT i = 0; i < pData->selectedNodeCount; i++) {
					EX_FLOWGRAPH_NODE* src = _flowgraph_findnode(pData, pData->selectedNodes[i]);
					if (!src) continue;
					pData->clipboardNodes[i] = *src;
					pData->clipboardNodes[i].title = StrDupW(src->title);
					pData->clipboardNodes[i].ports = (EX_FLOWGRAPH_PORT*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_PORT) * src->portCount);

					for (INT j = 0; j < src->portCount; j++) {
						EX_FLOWGRAPH_PORT* dst = &pData->clipboardNodes[i].ports[j];
						EX_FLOWGRAPH_PORT* sp = &src->ports[j];
						*dst = *sp;
						dst->name = StrDupW(sp->name);
						if (sp->imagePath) dst->imagePath = StrDupW(sp->imagePath);
						if (sp->widgetData) {
							dst->widgetData = _flowgraph_copy_widget_data(sp->widgetType, sp->widgetData);
							if (!dst->widgetData && sp->widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE) {
								HEXIMAGE hDst = NULL;
								if (_img_copy((HEXIMAGE)sp->widgetData, &hDst)) dst->widgetData = (LPVOID)hDst;
							}
						}
					}
					if (src->x < minX) minX = src->x;
					if (src->y < minY) minY = src->y;
					if (src->x + src->width > maxX) maxX = src->x + src->width;
					if (src->y + src->height > maxY) maxY = src->y + src->height;
				}
				pData->clipboardCenter.x = (minX + maxX) / 2.0f;
				pData->clipboardCenter.y = (minY + maxY) / 2.0f;

				// 3. 拷贝选区内的连线
				INT connCount = 0;
				for (INT i = 0; i < pData->connectionCount; i++) {
					EX_FLOWGRAPH_CONNECTION* c = &pData->connections[i];
					if (_flowgraph_is_node_selected(pData, c->fromNode) && _flowgraph_is_node_selected(pData, c->toNode)) connCount++;
				}
				pData->clipboardConnectionCount = connCount;
				if (connCount > 0) {
					pData->clipboardConnections = (EX_FLOWGRAPH_CONNECTION*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_CONNECTION) * connCount);
					INT idx = 0;
					for (INT i = 0; i < pData->connectionCount; i++) {
						EX_FLOWGRAPH_CONNECTION* c = &pData->connections[i];
						if (_flowgraph_is_node_selected(pData, c->fromNode) && _flowgraph_is_node_selected(pData, c->toNode)) {
							pData->clipboardConnections[idx++] = *c;
						}
					}
				}
				else {
					// ★★★ 修复：没有连线时必须置空，防止变成野指针导致后续释放崩溃 ★★★
					pData->clipboardConnections = NULL;
				}
				// ★★★ 核心新增：如果是 Ctrl+X，在数据 safely 存入剪贴板后，执行删除操作 ★★★
				if (wParam == 'X') {
					// 倒序删除防止索引错乱，_flowgraph_removenode 会自动清理关联的废弃连线
					for (INT i = pData->selectedNodeCount - 1; i >= 0; i--) {
						_flowgraph_removenode(hObj, pData->selectedNodes[i]);
					}
					_flowgraph_clear_selection(pData);
					Ex_ObjInvalidateRect(hObj, 0);
				}
			}
			return 1;
		}
		// ★ Ctrl+V：粘贴多节点和连线
		else if (ctrlPressed && wParam == 'V') {
			// 如果焦点在编辑框，让编辑框处理
			//if (hFocus == pData->editPanelEdit) {
			//	return Ex_ObjDefProc(hWnd, hObj, uMsg, wParam, lParam);
			//}
			if (pData->clipboardNodeCount > 0) {
				INT scrollX = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_HORZ);
				INT scrollY = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_VERT);
				INT mouseX = Ex_ObjGetLong(hObj, FLOWGRAPH_LONG_MOUSE_X);
				INT mouseY = Ex_ObjGetLong(hObj, FLOWGRAPH_LONG_MOUSE_Y);
				FLOAT virtualX = (mouseX + scrollX) / pData->zoom;
				FLOAT virtualY = (mouseY + scrollY) / pData->zoom;

				FLOAT offsetX = virtualX - pData->clipboardCenter.x;
				FLOAT offsetY = virtualY - pData->clipboardCenter.y;
				// ★★★ 修复：防止粘贴后节点坐标出现负数跑出画布顶边/左边 ★★★
				FLOAT minNewX = 999999.0f, minNewY = 999999.0f;
				for (INT i = 0; i < pData->clipboardNodeCount; i++) {
					FLOAT nx = pData->clipboardNodes[i].x + offsetX;
					FLOAT ny = pData->clipboardNodes[i].y + offsetY;
					if (nx < minNewX) minNewX = nx;
					if (ny < minNewY) minNewY = ny;
				}
				// 如果预测坐标小于0，则修正偏移量，保证所有节点 x>=0, y>=0
				if (minNewX < 0.0f) offsetX -= minNewX;
				if (minNewY < 0.0f) offsetY -= minNewY;
				// ★★★ 修复结束 ★★★
				_flowgraph_clear_selection(pData); // 清空旧选择，准备选中新节点

				INT* oldIds = (INT*)Ex_MemAlloc(sizeof(INT) * pData->clipboardNodeCount);
				INT* newIds = (INT*)Ex_MemAlloc(sizeof(INT) * pData->clipboardNodeCount);

				// 1. 粘贴节点
				for (INT i = 0; i < pData->clipboardNodeCount; i++) {
					EX_FLOWGRAPH_NODE tempNode = pData->clipboardNodes[i];
					tempNode.x += offsetX;
					tempNode.y += offsetY;
					oldIds[i] = tempNode.id;
					INT newId = _flowgraph_addnode(hObj, &tempNode); // 自动深拷贝并生成新ID
					newIds[i] = newId;
					_flowgraph_add_to_selection(pData, newId);
				}

				// 2. 粘贴连线
				for (INT i = 0; i < pData->clipboardConnectionCount; i++) {
					EX_FLOWGRAPH_CONNECTION tempConn = pData->clipboardConnections[i];
					INT newFrom = -1, newTo = -1;
					for (INT j = 0; j < pData->clipboardNodeCount; j++) {
						if (oldIds[j] == tempConn.fromNode) newFrom = newIds[j];
						if (oldIds[j] == tempConn.toNode) newTo = newIds[j];
					}
					if (newFrom != -1 && newTo != -1) {
						tempConn.fromNode = newFrom;
						tempConn.toNode = newTo;
						tempConn.id = 0; // 重置ID让系统重新生成
						tempConn.controlPoint1.x += offsetX;
						tempConn.controlPoint1.y += offsetY;
						tempConn.controlPoint2.x += offsetX;
						tempConn.controlPoint2.y += offsetY;
						_flowgraph_addconnection(hObj, &tempConn);
					}
				}
				Ex_MemFree(oldIds);
				Ex_MemFree(newIds);

				_flowgraph_updatelayout(hObj);
				Ex_ObjInvalidateRect(hObj, 0);
			}
			return 1;
		}
		if (wParam == VK_DELETE) {
			if (pData) {
				BOOL shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
				
				// ★ Shift + Delete：删除选中的多个节点
				if (shiftPressed && pData->selectedNodeCount > 0) {
					// 检查是否包含受保护节点
					BOOL hasProtected = FALSE;
					for (INT i = 0; i < pData->selectedNodeCount; i++) {
						if (_flowgraph_is_node_protected_by_busy(pData, pData->selectedNodes[i])) {
							hasProtected = TRUE; break;
						}
					}
					if (hasProtected) {
						Ex_MessageBox(hObj, L"忙碌节点所处链路无法删除", L"警告", MB_ICONWARNING, MESSAGEBOX_FLAG_CENTEWINDOW);
						return 1;
					}

					// 倒序删除防止索引错乱
					for (INT i = pData->selectedNodeCount - 1; i >= 0; i--) {
						_flowgraph_removenode(hObj, pData->selectedNodes[i]);
					}
					_flowgraph_clear_selection(pData);
					Ex_ObjInvalidateRect(hObj, 0);
					return 1;
				}
				// Delete：删除选中的连线
				else if (pData->selectedConnection != -1) {
					if (_flowgraph_is_connection_protected_by_busy(pData, pData->selectedConnection)) {
						Ex_MessageBox(hObj, L"忙碌节点所处链路无法删除", L"警告", MB_ICONWARNING, MESSAGEBOX_FLAG_CENTEWINDOW);
						return 1; // ★ 拦截
					}
					_flowgraph_removeconnection(hObj, pData->selectedConnection);
					pData->selectedConnection = -1;
					pData->selectedPortNode = -1;
					pData->selectedPortIndex = -1;
					Ex_ObjInvalidateRect(hObj, 0);
					return 1;
				}
				// Delete：删除选中端口的连线
				else if (pData->selectedPortNode != -1 && pData->selectedPortIndex != -1) {
					EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, pData->selectedPortNode);
					if (node && pData->selectedPortIndex < node->portCount && node->ports[pData->selectedPortIndex].isConnected) {
						for (INT i = pData->connectionCount - 1; i >= 0; i--) {
							EX_FLOWGRAPH_CONNECTION* conn = &pData->connections[i];
							if ((conn->fromNode == node->id && conn->fromSlot == pData->selectedPortIndex) || (conn->toNode == node->id && conn->toSlot == pData->selectedPortIndex)) {
								_flowgraph_removeconnection(hObj, conn->id); break;
							}
						}
					}
					pData->selectedPortNode = -1; pData->selectedPortIndex = -1;
					Ex_ObjInvalidateRect(hObj, 0); return 1;
				}
			}
		}
	}
	else if (uMsg == WM_VSCROLL || uMsg == WM_HSCROLL) { _flowgraph_onscrollbar(hObj, uMsg, wParam, lParam); }
	else if (uMsg == FLOWGRAPH_MESSAGE_ADD_NODE) { return _flowgraph_addnode(hObj, (EX_FLOWGRAPH_NODE*)lParam); }
	else if (uMsg == FLOWGRAPH_MESSAGE_REMOVE_NODE) { return _flowgraph_removenode(hObj, (INT)wParam); }
	else if (uMsg == FLOWGRAPH_MESSAGE_ADD_CONNECTION) { return _flowgraph_addconnection(hObj, (EX_FLOWGRAPH_CONNECTION*)lParam); }
	else if (uMsg == FLOWGRAPH_MESSAGE_REMOVE_CONNECTION) { return _flowgraph_removeconnection(hObj, (INT)wParam); }
	else if (uMsg == FLOWGRAPH_MESSAGE_SET_BACKGROUNDCOLOR) {
		Ex_ObjSetLong(hObj, FLOWGRAPH_LONG_BACKGROUNDCOLOR, lParam);
	}
	else if (uMsg == FLOWGRAPH_MESSAGE_UPDATE_NODEDATA)
	{
		EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0); if (!pData) return 0;
		INT nodeId = (INT)wParam; EX_FLOWGRAPH_PORT* pNewData = (EX_FLOWGRAPH_PORT*)lParam; if (!pNewData) return 0;
		EX_FLOWGRAPH_NODE* pNode = _flowgraph_findnode(pData, nodeId); if (!pNode) return 0;
		for (INT i = 0; i < pNode->portCount; i++) {
			if (pNode->ports[i].id == pNewData->id) {
				if (pNode->ports[i].widgetData) {
					if (pNode->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT) Ex_MemFree(pNode->ports[i].widgetData);
					else if (pNode->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT) Ex_MemFree(pNode->ports[i].widgetData);
					else if (pNode->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE)
					{
						_img_destroy((HEXIMAGE)pNode->ports[i].widgetData);
						if (pNode->ports[i].imagePath) Ex_MemFree((void*)pNode->ports[i].imagePath);
					}
					else if (pNode->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO) {
						EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)pNode->ports[i].widgetData;
						for (INT k = 0; k < combo->count; k++) Ex_MemFree((void*)combo->options[k]);
						Ex_MemFree(combo->options); Ex_MemFree(combo);
					}
					else if (pNode->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON) {
						EX_FLOWGRAPH_NODE_BUTTON_DATA* btn = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)pNode->ports[i].widgetData;
						Ex_MemFree((void*)btn->caption); Ex_MemFree(btn);
					}
					else if (pNode->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON) {
						EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* dualBtn = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)pNode->ports[i].widgetData;
						Ex_MemFree((void*)dualBtn->caption1);
						Ex_MemFree((void*)dualBtn->caption2);
						Ex_MemFree(dualBtn);
					}
				}
				pNode->ports[i].widgetType = pNewData->widgetType;
				if (pNewData->widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT)
					pNode->ports[i].widgetData = (LPVOID)StrDupW((LPCWSTR)pNewData->widgetData);
				else if (pNewData->widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT)
					pNode->ports[i].widgetData = (LPVOID)StrDupW((LPCWSTR)pNewData->widgetData);
				else if (pNewData->widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE)
				{
					pNode->ports[i].widgetData = pNewData->widgetData; // 转移所有权
					if (pNewData->imagePath) pNode->ports[i].imagePath = StrDupW(pNewData->imagePath);
					else pNode->ports[i].imagePath = NULL;
				}
				else
					pNode->ports[i].widgetData = pNewData->widgetData;
				// ★★★ 修复：执行中的节点不应重置自身状态和级联失效下游 ★★★
				// 原因：执行期间调用 UPDATE_NODEDATA 是正常的数据输出/显示更新行为，
				// 不应视为"用户手动修改数据"而触发状态重置。
				// 只有非执行状态（IDLE/COMPLETED/FAILED）下的数据修改才需要级联失效。
				if (pNode->executionStatus != FLOWGRAPH_EXEC_STATUS_RUNNING) {
					pNode->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
					_flowgraph_invalidate_downstream(pData, pNode->id);
				}
				// ★★★ 新增：如果是图片类型，自动调整组件大小
				_flowgraph_adjust_image_size(&pNode->ports[i]);
				_flowgraph_calcnodesize(hObj, pNode); _flowgraph_updatelayout(hObj); Ex_ObjInvalidateRect(hObj, 0);
			}
		}
		return 0;
	}
	else if (uMsg == FLOWGRAPH_MESSAGE_EXECUTE_NODE) {
		_flowgraph_executenode(hObj, (INT)wParam, (INT)lParam);
		Ex_ObjInvalidateRect(hObj, 0);
		return 1;
	}
	else if (uMsg == FLOWGRAPH_MESSAGE_EXECUTE_CHAIN) {
		_flowgraph_execute_chain(hObj, (INT)wParam, (INT)lParam);
		Ex_ObjInvalidateRect(hObj, 0);
		return 1;
	}
	else if (uMsg == FLOWGRAPH_MESSAGE_EXECUTE_ALL) {
		_flowgraph_execute_all(hObj, (INT)wParam);
		Ex_ObjInvalidateRect(hObj, 0);
		return 1;
	}
	else if (uMsg == FLOWGRAPH_MESSAGE_FIND_NODE)
	{
		EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0); if (!pData) return -1;
		EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, lParam);
		if (node)
		{
			return (size_t)node;
		}
		return 0;
	}
	else if (uMsg == FLOWGRAPH_MESSAGE_EXPORT_YAML) {
		return _flowgraph_export_to_yaml(hObj, (LPCWSTR)lParam);
	}
	else if (uMsg == FLOWGRAPH_MESSAGE_IMPORT_YAML) {
		return _flowgraph_import_from_yaml(hObj, (LPCWSTR)lParam);
	}
	else if (uMsg == FLOWGRAPH_MESSAGE_MARK_ASYNC_NODE) {
		EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
		if (pData) pData->asyncPendingNode = (INT)wParam;
		return 1;
		}
	else if (uMsg == FLOWGRAPH_MESSAGE_NODE_EXECUTION_COMPLETE) {
		return _flowgraph_node_execution_complete(hObj, (EX_FLOWGRAPH_ASYNC_RESULT*)lParam);
	}
	else if (uMsg == FLOWGRAPH_MESSAGE_REGISTER_CARD_TYPE) {
		return _flowgraph_register_card_type(hObj, (EX_FLOWGRAPH_CARD_DESCRIPTOR*)lParam);
	}
	else if (uMsg == FLOWGRAPH_MESSAGE_CREATE_CUSTOM_NODE) {
		return _flowgraph_create_custom_node(hObj, (EX_FLOWGRAPH_CUSTOM_NODE_CREATE*)lParam);
	}
	else if (uMsg == FLOWGRAPH_MESSAGE_UNREGISTER_CARD_TYPE) {
		return _flowgraph_unregister_card_type(hObj, (INT)wParam);
	}
	else if (uMsg == FLOWGRAPH_MESSAGE_ADD_DYNAMIC_PORT2) { return _flowgraph_add_dynamic_port2(hObj, (INT)wParam); }
	else if (uMsg == FLOWGRAPH_MESSAGE_REMOVE_DYNAMIC_PORT2) { return _flowgraph_remove_dynamic_port2(hObj, (INT)wParam); }
	else if (uMsg == WM_DROPFILES)
	{
		EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
		if (!pData) return 0;

		HDROP hDrop = (HDROP)wParam;
		UINT fileNumber = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
		if (fileNumber == 0) {
			DragFinish(hDrop);
			return 1;
		}

		// ★★★ 核心修复：使用 GetMessagePos 获取 Drop 瞬间的真实屏幕坐标 ★★★
		DWORD dwPos = GetMessagePos();
		POINT ptScreen;
		ptScreen.x = GET_X_LPARAM(dwPos);
		ptScreen.y = GET_Y_LPARAM(dwPos);

		// 1. 将屏幕坐标转换为主窗口(hWnd)的客户区坐标
		ScreenToClient(hWnd, &ptScreen);

		// 2. 获取当前控件(hObj)在父窗口中的相对位置，计算出控件内的物理像素坐标
		RECT rcObj;
		Ex_ObjGetRect(hObj, &rcObj);
		INT localX = ptScreen.x - rcObj.left;
		INT localY = ptScreen.y - rcObj.top;

		// 兜底：如果计算出的坐标越界，说明拖到了控件外，直接放弃
		RECT rcClient;
		Ex_ObjGetClientRect(hObj, &rcClient);
		if (localX < 0 || localX >(rcClient.right - rcClient.left) ||
			localY < 0 || localY >(rcClient.bottom - rcClient.top)) {
			DragFinish(hDrop);
			return 1;
		}

		INT scrollX = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_HORZ);
		INT scrollY = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_VERT);
		FLOAT virtualX = (localX + scrollX) / pData->zoom;
		FLOAT virtualY = (localY + scrollY) / pData->zoom;

		// 2. 查找鼠标下方的节点
		EX_FLOWGRAPH_NODE* topmostNode = _flowgraph_find_topmost_node_at(pData, virtualX, virtualY);
	
		BOOL handled = FALSE;
		// 3. 仅当拖拽到“本地图片卡片”上时处理
		if (topmostNode && topmostNode->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_IMAGE) {
			for (UINT index = 0; index < fileNumber; index++) {
				UINT fileNameLength = DragQueryFileW(hDrop, index, NULL, 0);
				if (fileNameLength > 0) {
					std::wstring fileName;
					fileName.resize(fileNameLength);
					DragQueryFileW(hDrop, index, (LPWSTR)fileName.data(), fileNameLength * 2);
					// 4. 检查文件后缀是否为支持的图片格式
					size_t dotPos = fileName.find_last_of(L".");
					if (dotPos != std::wstring::npos) {
						std::wstring ext = fileName.substr(dotPos + 1);
						for (auto& c : ext) c = towlower(c); // 转小写
						if (ext == L"png" || ext == L"jpg" || ext == L"jpeg" || ext == L"bmp" || ext == L"dds") {
							// 5. 遍历端口，找到 IMAGE 类型的端口并加载
							for (INT i = 0; i < topmostNode->portCount; i++) {
								EX_FLOWGRAPH_PORT& port = topmostNode->ports[i];
								if (port.dataType == FLOWGRAPH_DATATYPE_IMAGE) {
									// 释放旧数据
									if (port.widgetData) {
										_img_destroy((HEXIMAGE)port.widgetData);
										port.widgetData = NULL;
									}
									if (port.imagePath) {
										Ex_MemFree((void*)port.imagePath);
										port.imagePath = NULL;
									}

									// ★★★ 加载新图片 ★★★
									HEXIMAGE hImg = NULL;
									hImg = _flowgraph_load_image_no_lock(fileName.c_str());
									if (hImg) {
										port.widgetData = (LPVOID)hImg;
										port.imagePath = StrDupW(fileName.c_str());
										// 自适应预览框大小
										_flowgraph_adjust_image_size(&port);

										// 重置状态并级联失效下游
										topmostNode->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
										_flowgraph_invalidate_downstream(pData, topmostNode->id);

										_flowgraph_calcnodesize(hObj, topmostNode);
										_flowgraph_updatelayout(hObj);
										Ex_ObjInvalidateRect(hObj, 0);
										handled = TRUE;
									}
									break; // 只更新第一个找到的图片端口
								}
							}
							if (handled) break; // 只处理第一张有效的图片
						}
					}
				}
			}
		}

		DragFinish(hDrop); // 释放拖拽句柄
		if (handled) return 1; // 拦截消息，表示已成功处理
	}
	return Ex_ObjDefProc(hWnd, hObj, uMsg, wParam, lParam);
}

// ==================== 绘制 ====================
void _flowgraph_paint(HEXOBJ hObj)
{
	EX_PAINTSTRUCT ps;
	if (Ex_ObjBeginPaint(hObj, &ps))
	{
		EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0); if (!pData) { Ex_ObjEndPaint(hObj, &ps); return; }
		INT scrollX = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_HORZ); INT scrollY = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_VERT);
		_canvas_clear(ps.hCanvas, Ex_ObjGetLong(hObj, FLOWGRAPH_LONG_BACKGROUNDCOLOR));
		FLOAT gridSpacing = 30.0f * pData->zoom; if (gridSpacing < 5.0f) gridSpacing = 5.0f;
		HEXBRUSH hBrushGrid = _brush_create(ExARGB(29, 29, 29, 255));
		for (FLOAT x = fmodf(-scrollX, gridSpacing); x < ps.rcPaint.right; x += gridSpacing) _canvas_drawline(ps.hCanvas, hBrushGrid, x, 0, x, (FLOAT)ps.rcPaint.bottom, 1, 2);
		for (FLOAT y = fmodf(-scrollY, gridSpacing); y < ps.rcPaint.bottom; y += gridSpacing) _canvas_drawline(ps.hCanvas, hBrushGrid, 0, y, (FLOAT)ps.rcPaint.right, y, 1, 2);
		_brush_destroy(hBrushGrid);
		
		// 绘制未选中连线
		for (INT i = 0; i < pData->connectionCount; i++) {
			EX_FLOWGRAPH_CONNECTION* conn = &pData->connections[i];
			if (pData->selectedConnection == conn->id) continue;
			EX_FLOWGRAPH_NODE* fromNode = _flowgraph_findnode(pData, conn->fromNode);
			EX_FLOWGRAPH_NODE* toNode = _flowgraph_findnode(pData, conn->toNode);
			if (!fromNode || !toNode) continue;
			POINTF fromPtCenter = _flowgraph_get_port_center(fromNode, conn->fromSlot);
			POINTF toPtCenter = _flowgraph_get_port_center(toNode, conn->toSlot);
			POINTF fromPt, toPt;
			fromPt.x = fromPtCenter.x * pData->zoom - scrollX; fromPt.y = fromPtCenter.y * pData->zoom - scrollY;
			toPt.x = toPtCenter.x * pData->zoom - scrollX; toPt.y = toPtCenter.y * pData->zoom - scrollY;
			EXARGB lineColor = _flowgraph_get_port_color(fromNode->ports[conn->fromSlot].dataType);
			HEXBRUSH hBrushLine = _brush_create(lineColor);
			POINTF curvePoints[4] = { {fromPt.x, fromPt.y}, {conn->controlPoint1.x * pData->zoom - scrollX, conn->controlPoint1.y * pData->zoom - scrollY}, {conn->controlPoint2.x * pData->zoom - scrollX, conn->controlPoint2.y * pData->zoom - scrollY}, {toPt.x, toPt.y} };
			_canvas_drawcurve(ps.hCanvas, hBrushLine, curvePoints, 4, 0.5f, 2.0f, 0);
			_brush_destroy(hBrushLine);
		}
		// 绘制未选中节点
		for (INT i = 0; i < pData->nodeCount; i++) {
			if (!_flowgraph_is_node_selected(pData, pData->nodes[i].id)) {
				_flowgraph_drawnode(ps.hCanvas, pData, &pData->nodes[i], FALSE, pData->zoom, scrollX, scrollY);
			}
		}
		// 绘制选中节点
		for (INT i = 0; i < pData->nodeCount; i++) {
			if (_flowgraph_is_node_selected(pData, pData->nodes[i].id)) {
				_flowgraph_drawnode(ps.hCanvas, pData, &pData->nodes[i], TRUE, pData->zoom, scrollX, scrollY);
			}
		}
		// 绘制选中连线
		for (INT i = 0; i < pData->connectionCount; i++) {
			EX_FLOWGRAPH_CONNECTION* conn = &pData->connections[i];
			if (pData->selectedConnection != conn->id) continue;
			EX_FLOWGRAPH_NODE* fromNode = _flowgraph_findnode(pData, conn->fromNode);
			EX_FLOWGRAPH_NODE* toNode = _flowgraph_findnode(pData, conn->toNode);
			if (!fromNode || !toNode) continue;
			POINTF fromPtCenter = _flowgraph_get_port_center(fromNode, conn->fromSlot);
			POINTF toPtCenter = _flowgraph_get_port_center(toNode, conn->toSlot);
			POINTF fromPt, toPt;
			fromPt.x = fromPtCenter.x * pData->zoom - scrollX; fromPt.y = fromPtCenter.y * pData->zoom - scrollY;
			toPt.x = toPtCenter.x * pData->zoom - scrollX; toPt.y = toPtCenter.y * pData->zoom - scrollY;
			HEXBRUSH hBrushLine = _brush_create(ExARGB(255, 165, 0, 255));
			POINTF curvePoints[4] = { {fromPt.x, fromPt.y}, {conn->controlPoint1.x * pData->zoom - scrollX, conn->controlPoint1.y * pData->zoom - scrollY}, {conn->controlPoint2.x * pData->zoom - scrollX, conn->controlPoint2.y * pData->zoom - scrollY}, {toPt.x, toPt.y} };
			_canvas_drawcurve(ps.hCanvas, hBrushLine, curvePoints, 4, 1.0f, 3.0f, 0);
			HEXBRUSH hBrushCtrl = _brush_create(ExARGB(255, 255, 255, 255));
			POINTF cp1 = { conn->controlPoint1.x * pData->zoom - scrollX, conn->controlPoint1.y * pData->zoom - scrollY };
			POINTF cp2 = { conn->controlPoint2.x * pData->zoom - scrollX, conn->controlPoint2.y * pData->zoom - scrollY };
			_canvas_fillellipse(ps.hCanvas, hBrushCtrl, cp1.x, cp1.y, 4.0f * pData->zoom, 4.0f * pData->zoom);
			_canvas_fillellipse(ps.hCanvas, hBrushCtrl, cp2.x, cp2.y, 4.0f * pData->zoom, 4.0f * pData->zoom);
			_brush_destroy(hBrushCtrl); _brush_destroy(hBrushLine);
		}
		// 绘制拖拽临时连线
		if (pData->connectingSlot != -1 && pData->connectingNode != -1) {
			EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, pData->connectingNode); if (node) {
				POINTF fromPt, toPt; FLOAT mouseX = (FLOAT)Ex_ObjGetLong(hObj, FLOWGRAPH_LONG_MOUSE_X); FLOAT mouseY = (FLOAT)Ex_ObjGetLong(hObj, FLOWGRAPH_LONG_MOUSE_Y);
				EX_FLOWGRAPH_PORT& port = node->ports[pData->connectingSlot];
				if (pData->connectingSlotType == FLOWGRAPH_SLOTTYPE_OUTPUT) {
					fromPt.x = (node->x + (port.portRect.left + port.portRect.right) / 2.0f) * pData->zoom - scrollX; fromPt.y = (node->y + (port.portRect.top + port.portRect.bottom) / 2.0f) * pData->zoom - scrollY;
					toPt.x = mouseX; toPt.y = mouseY;
				}
				else { toPt.x = (node->x + (port.portRect.left + port.portRect.right) / 2.0f) * pData->zoom - scrollX; toPt.y = (node->y + (port.portRect.top + port.portRect.bottom) / 2.0f) * pData->zoom - scrollY; fromPt.x = mouseX; fromPt.y = mouseY; }
				HEXBRUSH hBrushTemp = _brush_create(_flowgraph_get_port_color(port.dataType));
				_canvas_drawline(ps.hCanvas, hBrushTemp, fromPt.x, fromPt.y, toPt.x, toPt.y, 2.0f, D2D1_DASH_STYLE_DASH);
				_brush_destroy(hBrushTemp);
			}
		}

		// ★ 绘制节点标题悬停提示框
		if (pData->hoverTitleNode != -1) {
			EX_FLOWGRAPH_NODE* hoverNode = _flowgraph_findnode(pData, pData->hoverTitleNode);
			if (hoverNode) {
				FLOAT mouseX = (FLOAT)Ex_ObjGetLong(hObj, FLOWGRAPH_LONG_MOUSE_X);
				FLOAT mouseY = (FLOAT)Ex_ObjGetLong(hObj, FLOWGRAPH_LONG_MOUSE_Y);
				_flowgraph_draw_node_tooltip(ps.hCanvas, pData, hoverNode, mouseX, mouseY, (FLOAT)ps.uWidth, (FLOAT)ps.uHeight);
			}
		}
		// ★ 绘制侧边栏（最上层，覆盖在画布内容之上）
		_flowgraph_draw_sidebar(hObj, ps.hCanvas, pData, (FLOAT)ps.uWidth, (FLOAT)ps.uHeight);
		// ★ 绘制组合框下拉面板 (浮动层)
		if (pData->comboExpandedNode != -1) {
			EX_FLOWGRAPH_NODE* expNode = _flowgraph_findnode(pData, pData->comboExpandedNode);
			if (expNode && pData->comboExpandedPortIdx < expNode->portCount) {
				EX_FLOWGRAPH_PORT& expPort = expNode->ports[pData->comboExpandedPortIdx];
				if (expPort.widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO && expPort.widgetData) {
					EX_FLOWGRAPH_NODE_COMBO_DATA* comboData = (EX_FLOWGRAPH_NODE_COMBO_DATA*)expPort.widgetData;

					FLOAT wx = (expNode->x + expPort.widgetRect.left) * pData->zoom - scrollX;
					FLOAT wy = (expNode->y + expPort.widgetRect.bottom) * pData->zoom - scrollY;
					FLOAT ww = (expPort.widgetRect.right - expPort.widgetRect.left) * pData->zoom;
					FLOAT itemH = 28.0f * pData->zoom;
					FLOAT panelH = comboData->count * itemH;
					FLOAT panelX = wx;
					FLOAT panelY = wy;

					// 阴影
					HEXBRUSH hBrushShadow = _brush_create(ExARGB(0, 0, 0, 80));
					_canvas_fillrect(ps.hCanvas, hBrushShadow, panelX + 4, panelY + 4, panelX + ww + 4, panelY + panelH + 4);
					_brush_destroy(hBrushShadow);

					// 背景
					HEXBRUSH hBrushBg = _brush_create(ExARGB(38, 38, 42, 255));
					_canvas_fillrect(ps.hCanvas, hBrushBg, panelX, panelY, panelX + ww, panelY + panelH);
					_brush_destroy(hBrushBg);

					// 边框
					HEXBRUSH hBrushPBorder = _brush_create(ExARGB(70, 70, 80, 255));
					_canvas_drawrect(ps.hCanvas, hBrushPBorder, panelX, panelY, panelX + ww, panelY + panelH, 1.0f, 0);
					_brush_destroy(hBrushPBorder);

					HEXFONT hFont = _font_createfromfamily(L"Arial", 10 * pData->zoom, 0);

					for (INT k = 0; k < comboData->count; k++) {
						FLOAT iy = panelY + k * itemH;
						BOOL isHover = (pData->comboPanelHoverIdx == k);
						BOOL isSelected = (comboData->current == k);

						if (isHover) {
							HEXBRUSH hBrushHover = _brush_create(ExARGB(60, 60, 75, 255));
							_canvas_fillrect(ps.hCanvas, hBrushHover, panelX + 1, iy + 1, panelX + ww - 1, iy + itemH - 1);
							_brush_destroy(hBrushHover);
						}

						EXARGB textColor = isSelected ? ExARGB(100, 180, 255, 255) : ExARGB(217, 217, 217, 255);
						_canvas_drawtext(ps.hCanvas, hFont, textColor, comboData->options[k], -1,
							DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS,
							panelX + 24.0f * pData->zoom, iy, panelX + ww - 8.0f * pData->zoom, iy + itemH);

						if (isSelected) {
							// 左侧高亮指示条
							HEXBRUSH hBrushSel = _brush_create(ExARGB(100, 180, 255, 255));
							_canvas_fillrect(ps.hCanvas, hBrushSel, panelX + 4, iy + 6, panelX + 7, iy + itemH - 6);
							_brush_destroy(hBrushSel);
						}
					}
					_font_destroy(hFont);
				}
			}
		}

		// ★ 绘制框选虚线矩形
		if (pData->isSelecting) {
			FLOAT x1 = __min(pData->selectStartPos.x, pData->selectEndPos.x) * pData->zoom - scrollX;
			FLOAT y1 = __min(pData->selectStartPos.y, pData->selectEndPos.y) * pData->zoom - scrollY;
			FLOAT x2 = __max(pData->selectStartPos.x, pData->selectEndPos.x) * pData->zoom - scrollX;
			FLOAT y2 = __max(pData->selectStartPos.y, pData->selectEndPos.y) * pData->zoom - scrollY;

			// 半透明填充
			HEXBRUSH hBrushFill = _brush_create(ExARGB(100, 150, 255, 40));
			_canvas_fillrect(ps.hCanvas, hBrushFill, x1, y1, x2, y2);
			_brush_destroy(hBrushFill);

			// 虚线边框 (利用原有的 D2D1_DASH_STYLE_DASH)
			HEXBRUSH hBrushLine = _brush_create(ExARGB(100, 150, 255, 255));
			_canvas_drawline(ps.hCanvas, hBrushLine, x1, y1, x2, y1, 1.5f, D2D1_DASH_STYLE_DASH);
			_canvas_drawline(ps.hCanvas, hBrushLine, x2, y1, x2, y2, 1.5f, D2D1_DASH_STYLE_DASH);
			_canvas_drawline(ps.hCanvas, hBrushLine, x2, y2, x1, y2, 1.5f, D2D1_DASH_STYLE_DASH);
			_canvas_drawline(ps.hCanvas, hBrushLine, x1, y2, x1, y1, 1.5f, D2D1_DASH_STYLE_DASH);
			_brush_destroy(hBrushLine);
		}

		// ★ 绘制浮动编辑面板(最顶层)
		if (pData->showEditPanel) {
			_flowgraph_draw_edit_panel(ps.hCanvas, pData, hObj, (FLOAT)ps.uWidth, (FLOAT)ps.uHeight);
		}

		// ★ 绘制右键菜单面板 (最顶层)
		if (pData->showContextMenu) {
			_flowgraph_draw_context_menu(hObj, ps.hCanvas, pData);
		}

		// ★★★ 核心新增：绘制顶部工程名透明面板 ★★★
		if (pData->projectName[0] != L'\0') {
			HEXFONT hFontProj = _font_createfromfamily(L"微软雅黑", 14, FONT_STYLE_BOLD);
			FLOAT textW = 0, textH = 0;
			// 计算文本实际宽度
			_canvas_calctextsize(ps.hCanvas, hFontProj, pData->projectName, -1, DT_SINGLELINE | DT_VCENTER, 9999.0f, 9999.0f, &textW, &textH);

			FLOAT panelW = textW + 40.0f; // 左右各留20px内边距
			FLOAT panelH = 36.0f;
			// 顶部居中显示
			FLOAT panelX = ((FLOAT)ps.uWidth - panelW) / 2.0f;
			// 防重叠保护：如果窗口太窄，避免与左侧侧边栏(60px)重叠
			if (panelX < FLOWGRAPH_SIDEBAR_WIDTH + 10.0f) {
				panelX = FLOWGRAPH_SIDEBAR_WIDTH + 10.0f;
			}
			FLOAT panelY = 15.0f;

			// 1. 绘制半透明黑底 (类似OSD悬浮窗)
			HEXBRUSH hBrushBg = _brush_create(ExARGB(20, 20, 25, 180));
			_canvas_fillroundedrect(ps.hCanvas, hBrushBg, panelX, panelY, panelX + panelW, panelY + panelH, 6.0f, 6.0f);
			_brush_destroy(hBrushBg);

			// 2. 绘制细微边框增加精致感
			HEXBRUSH hBrushBorder = _brush_create(ExARGB(80, 80, 100, 150));
			_canvas_drawroundedrect(ps.hCanvas, hBrushBorder, panelX, panelY, panelX + panelW, panelY + panelH, 6.0f, 6.0f, 1.0f, 0);
			_brush_destroy(hBrushBorder);

			// 3. 绘制白色工程名文字
			_canvas_drawtext(ps.hCanvas, hFontProj, ExARGB(235, 235, 245, 255), pData->projectName, -1,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS,
				panelX, panelY, panelX + panelW, panelY + panelH);

			_font_destroy(hFontProj);
		}
		Ex_ObjEndPaint(hObj, &ps);
	}
}

// ==================== 绘制添加面板 ====================
void _flowgraph_draw_add_panel(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, FLOAT canvasWidth, FLOAT canvasHeight)
{
	FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
	FLOAT panelY = FLOWGRAPH_SIDEBAR_BTN_Y0;
	FLOAT panelW = FLOWGRAPH_ADD_PANEL_WIDTH;

	INT visibleCount = pData->cardRegistryCount;
	// ★ 动态计算面板高度（无卡片时显示空提示高度）
	FLOAT panelH = FLOWGRAPH_ADD_PANEL_HEADER + (visibleCount > 0 ? visibleCount * FLOWGRAPH_ADD_PANEL_ITEM_H + 4 : 36);

	// 阴影
	HEXBRUSH hBrushShadow = _brush_create(ExARGB(0, 0, 0, 50));
	_canvas_fillrect(hCanvas, hBrushShadow, panelX + 3, panelY + 3, panelX + panelW + 3, panelY + panelH + 3);
	_brush_destroy(hBrushShadow);
	// 背景
	HEXBRUSH hBrushPanelBg = _brush_create(ExARGB(28, 28, 36, 248));
	_canvas_fillrect(hCanvas, hBrushPanelBg, panelX, panelY, panelX + panelW, panelY + panelH);
	_brush_destroy(hBrushPanelBg);
	// 边框
	HEXBRUSH hBrushBorder = _brush_create(ExARGB(65, 65, 80, 255));
	_canvas_drawrect(hCanvas, hBrushBorder, panelX, panelY, panelX + panelW, panelY + panelH, 1.0f, 0);
	_brush_destroy(hBrushBorder);

	// 面板标题
	HEXFONT hFontTitle = _font_createfromfamily(L"微软雅黑", 10, 0);
	_canvas_drawtext(hCanvas, hFontTitle, ExARGB(120, 120, 135, 255), L"添加节点", -1,
		DT_LEFT | DT_VCENTER, panelX + 10, panelY + 2, panelX + panelW - 10, panelY + FLOWGRAPH_ADD_PANEL_HEADER);
	_font_destroy(hFontTitle);

	// ★ 无卡片类型时的空状态提示
	if (visibleCount == 0) {
		HEXFONT hFontEmpty = _font_createfromfamily(L"微软雅黑", 11, 0);
		_canvas_drawtext(hCanvas, hFontEmpty, ExARGB(90, 90, 105, 255), L"暂无卡片类型", -1,
			DT_CENTER | DT_VCENTER, panelX + 4, panelY + FLOWGRAPH_ADD_PANEL_HEADER, panelX + panelW - 4, panelY + panelH - 4);
		_font_destroy(hFontEmpty);
		return;
	}

	// ★ 遍历注册的卡片类型绘制条目
	FLOAT itemStartY = panelY + FLOWGRAPH_ADD_PANEL_HEADER;
	FLOAT itemH = FLOWGRAPH_ADD_PANEL_ITEM_H;
	for (INT i = 0; i < visibleCount; i++) {
		EX_FLOWGRAPH_CARD_DESCRIPTOR* desc = &pData->cardRegistry[i];
		FLOAT iy = itemStartY + i * itemH;
		BOOL isHover = (pData->sidebarAddPanelHover == i);

		if (isHover) {
			HEXBRUSH hBrushItemHover = _brush_create(ExARGB(50, 50, 65, 200));
			_canvas_fillrect(hCanvas, hBrushItemHover, panelX + 1, iy + 1, panelX + panelW - 1, iy + itemH - 1);
			_brush_destroy(hBrushItemHover);
		}

		// 使用卡片注册时的 tagColor 作为左侧色条
		HEXBRUSH hBrushColor = _brush_create(desc->tagColor);
		_canvas_fillrect(hCanvas, hBrushColor, panelX + 8, iy + (itemH - 16) / 2, panelX + 12, iy + (itemH + 16) / 2);
		_brush_destroy(hBrushColor);

		HEXFONT hFont = _font_createfromfamily(L"微软雅黑", 12, 0);
		EXARGB textColor = isHover ? ExARGB(240, 240, 245, 255) : ExARGB(195, 195, 210, 255);
		// 使用卡片注册时的 typeName 作为显示文本
		_canvas_drawtext(hCanvas, hFont, textColor, desc->typeName, -1,
			DT_LEFT | DT_VCENTER, panelX + 20, iy, panelX + panelW - 12, iy + itemH);
		_font_destroy(hFont);
	}
}

// ==================== 绘制侧边栏 ====================
void _flowgraph_draw_sidebar(HEXOBJ hObj, HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, FLOAT canvasWidth, FLOAT canvasHeight)
{
	FLOAT sw = FLOWGRAPH_SIDEBAR_WIDTH;
	HEXBRUSH hBrushBg = _brush_create(ExARGB(18, 18, 22, 245));
	_canvas_fillrect(hCanvas, hBrushBg, 0, 0, sw, canvasHeight);
	_brush_destroy(hBrushBg);
	HEXBRUSH hBrushSep = _brush_create(ExARGB(50, 50, 60, 255));
	_canvas_drawline(hCanvas, hBrushSep, sw - 0.5f, 0, sw - 0.5f, canvasHeight, 1, 0);
	_brush_destroy(hBrushSep);

	FLOAT btnYs[FLOWGRAPH_SIDEBAR_BTN_COUNT] = {
		(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y0, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y1, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y2,
		(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y3, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y4, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y5,
		(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y6, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y7
	};
	LPCWSTR labels[FLOWGRAPH_SIDEBAR_BTN_COUNT] = { L"添加", L"链路", L"链路", L"取消", L"导出", L"导入", L"自定义", L"帮助" };
	EXARGB activeColors[FLOWGRAPH_SIDEBAR_BTN_COUNT] = {
		ExARGB(100, 150, 255, 255),   // 0 添加
		ExARGB(0, 200, 200, 255),     // 1 链路从头
		ExARGB(200, 150, 0, 255),     // 2 链路继续
		ExARGB(255, 80, 80, 255),     // 3 取消 (红色)
		ExARGB(0, 180, 200, 255),     // 4 导出
		ExARGB(160, 100, 220, 255),   // 5 导入
		ExARGB(220, 160, 60, 255),    // 6 自定义
		ExARGB(100, 200, 255, 255)    // 7 帮助
	};

	for (INT i = 0; i < FLOWGRAPH_SIDEBAR_BTN_COUNT; i++) {
		FLOAT by = btnYs[i];
		BOOL isHover = (pData->sidebarHoverBtn == i);
		BOOL isAddActive = (i == 0 && pData->sidebarShowAddPanel);
		// ★ 索引更新：1=从头，2=继续
		BOOL isChainActive = (pData->sidebarShowChainPanel &&
			((i == 1 && pData->sidebarChainPanelMode == 0) ||
				(i == 2 && pData->sidebarChainPanelMode == 1)));
		BOOL isCustomActive = (i == 6 && pData->sidebarShowCustomPanel); // ★ 原8改6
		BOOL isCancelActive = (i == 3 && pData->sidebarShowCancelPanel); // ★ 原5改3

		if (isHover || isAddActive || isChainActive || isCustomActive || isCancelActive) {
			HEXBRUSH hBrushHover = _brush_create((isAddActive || isChainActive || isCustomActive || isCancelActive) ? ExARGB(40, 40, 55, 220) : ExARGB(45, 45, 58, 200));
			_canvas_fillrect(hCanvas, hBrushHover, 1, by, sw - 1, by + FLOWGRAPH_SIDEBAR_BTN_H);
			_brush_destroy(hBrushHover);
			HEXBRUSH hBrushActive = _brush_create(activeColors[i]);
			_canvas_fillrect(hCanvas, hBrushActive, 0, by + 6, 3, by + FLOWGRAPH_SIDEBAR_BTN_H - 6);
			_brush_destroy(hBrushActive);
		}

		EXARGB iconColor = (isHover || isAddActive || isChainActive || isCustomActive || isCancelActive) ? ExARGB(230, 230, 235, 255) : ExARGB(140, 140, 155, 255);
		HEXBRUSH hBrushIcon = _brush_create(iconColor);
		FLOAT cx = sw / 2.0f;
		FLOAT cy = by + 24.0f;

		if (i == 0) { // 添加 (+)
			_canvas_drawline(hCanvas, hBrushIcon, cx - 8, cy, cx + 8, cy, 2.0f, 0);
			_canvas_drawline(hCanvas, hBrushIcon, cx, cy - 8, cx, cy + 8, 2.0f, 0);
		}
		else if (i == 1) { // 链路从头 (|▶) (原3)
			_canvas_drawline(hCanvas, hBrushIcon, cx - 7, cy - 8, cx - 7, cy + 8, 2.0f, 0);
			POINTF pts[3] = { {cx - 3, cy - 7}, {cx - 3, cy + 7}, {cx + 9, cy} };
			HEXPATH hPath; _path_create(PATH_FLAG_DISABLESCALE, &hPath); _path_open(hPath);
			_path_beginfigure2(hPath, pts[0].x, pts[0].y);
			_path_addline(hPath, pts[0].x, pts[0].y, pts[1].x, pts[1].y);
			_path_addline(hPath, pts[1].x, pts[1].y, pts[2].x, pts[2].y);
			_path_endfigure(hPath, TRUE); _path_close(hPath);
			_canvas_fillpath(hCanvas, hPath, hBrushIcon); _path_destroy(hPath);
		}
		else if (i == 2) { // 链路继续 (‖▶) (原4)
			_canvas_drawline(hCanvas, hBrushIcon, cx - 9, cy - 8, cx - 9, cy + 8, 2.0f, 0);
			_canvas_drawline(hCanvas, hBrushIcon, cx - 5, cy - 8, cx - 5, cy + 8, 2.0f, 0);
			POINTF pts[3] = { {cx - 1, cy - 7}, {cx - 1, cy + 7}, {cx + 9, cy} };
			HEXPATH hPath; _path_create(PATH_FLAG_DISABLESCALE, &hPath); _path_open(hPath);
			_path_beginfigure2(hPath, pts[0].x, pts[0].y);
			_path_addline(hPath, pts[0].x, pts[0].y, pts[1].x, pts[1].y);
			_path_addline(hPath, pts[1].x, pts[1].y, pts[2].x, pts[2].y);
			_path_endfigure(hPath, TRUE); _path_close(hPath);
			_canvas_fillpath(hCanvas, hPath, hBrushIcon); _path_destroy(hPath);
		}
		else if (i == 3) { // ★ 取消 (■) (原5)
			_canvas_fillrect(hCanvas, hBrushIcon, cx - 6, cy - 6, cx + 6, cy + 6);
		}
		else if (i == 4) { // 导出 (↑) (原6)
			_canvas_drawline(hCanvas, hBrushIcon, cx, cy - 9, cx, cy + 1, 2.0f, 0);
			_canvas_drawline(hCanvas, hBrushIcon, cx - 5, cy - 4, cx, cy - 9, 2.0f, 0);
			_canvas_drawline(hCanvas, hBrushIcon, cx + 5, cy - 4, cx, cy - 9, 2.0f, 0);
			_canvas_drawline(hCanvas, hBrushIcon, cx - 7, cy + 4, cx + 7, cy + 4, 2.0f, 0);
		}
		else if (i == 5) { // 导入 (↓) (原7)
			_canvas_drawline(hCanvas, hBrushIcon, cx - 7, cy - 4, cx + 7, cy - 4, 2.0f, 0);
			_canvas_drawline(hCanvas, hBrushIcon, cx, cy - 1, cx, cy + 9, 2.0f, 0);
			_canvas_drawline(hCanvas, hBrushIcon, cx - 5, cy + 4, cx, cy + 9, 2.0f, 0);
			_canvas_drawline(hCanvas, hBrushIcon, cx + 5, cy + 4, cx, cy + 9, 2.0f, 0);
		}
		else if (i == 6) { // 自定义 (田) (原8)
			FLOAT boxSize = 5.0f; FLOAT gap = 3.0f;
			FLOAT startX = cx - boxSize - gap / 2.0f; FLOAT startY = cy - boxSize - gap / 2.0f;
			_canvas_fillrect(hCanvas, hBrushIcon, startX, startY, startX + boxSize, startY + boxSize);
			_canvas_fillrect(hCanvas, hBrushIcon, startX + boxSize + gap, startY, startX + boxSize * 2 + gap, startY + boxSize);
			_canvas_fillrect(hCanvas, hBrushIcon, startX, startY + boxSize + gap, startX + boxSize, startY + boxSize * 2 + gap);
			_canvas_fillrect(hCanvas, hBrushIcon, startX + boxSize + gap, startY + boxSize + gap, startX + boxSize * 2 + gap, startY + boxSize * 2 + gap);
		}
		else if (i == 7) { // 帮助 (?) (原9)
			_canvas_drawellipse(hCanvas, hBrushIcon, cx, cy, 12.0f, 12.0f, 1.5f, 0);
			HEXFONT hFontIcon = _font_createfromfamily(L"Arial", 14, FONT_STYLE_BOLD);
			_canvas_drawtext(hCanvas, hFontIcon, iconColor, L"?", -1, DT_CENTER | DT_VCENTER, cx - 8, cy - 8, cx + 8, cy + 8);
			_font_destroy(hFontIcon);
		}
		_brush_destroy(hBrushIcon);

		HEXFONT hFont = _font_createfromfamily(L"微软雅黑", 10, 0);
		EXARGB textColor = (isHover || isAddActive || isChainActive || isCustomActive || isCancelActive) ? ExARGB(220, 220, 225, 255) : ExARGB(120, 120, 135, 255);

		if (i == 1) { // 链路从头
			HEXFONT hFontSmall = _font_createfromfamily(L"微软雅黑", 9, 0);
			EXARGB tc2 = (isHover || isChainActive) ? ExARGB(220, 220, 225, 255) : ExARGB(110, 110, 125, 255);
			_canvas_drawtext(hCanvas, hFontSmall, tc2, L"链路", -1, DT_CENTER | DT_TOP, 0, by + 36, sw, by + 54);
			_canvas_drawtext(hCanvas, hFontSmall, tc2, L"从头", -1, DT_CENTER | DT_TOP, 0, by + 54, sw, by + 70);
			_font_destroy(hFontSmall);
		}
		else if (i == 2) { // 链路继续
			HEXFONT hFontSmall = _font_createfromfamily(L"微软雅黑", 9, 0);
			EXARGB tc2 = (isHover || isChainActive) ? ExARGB(220, 220, 225, 255) : ExARGB(110, 110, 125, 255);
			_canvas_drawtext(hCanvas, hFontSmall, tc2, L"链路", -1, DT_CENTER | DT_TOP, 0, by + 36, sw, by + 54);
			_canvas_drawtext(hCanvas, hFontSmall, tc2, L"继续", -1, DT_CENTER | DT_TOP, 0, by + 54, sw, by + 70);
			_font_destroy(hFontSmall);
		}
		else if (i == 6) { // 自定义
			HEXFONT hFontSmall = _font_createfromfamily(L"微软雅黑", 9, 0);
			EXARGB tc2 = (isHover || isCustomActive) ? ExARGB(220, 220, 225, 255) : ExARGB(110, 110, 125, 255);
			_canvas_drawtext(hCanvas, hFontSmall, tc2, L"自定", -1, DT_CENTER | DT_TOP, 0, by + 36, sw, by + 54);
			_canvas_drawtext(hCanvas, hFontSmall, tc2, L"义", -1, DT_CENTER | DT_TOP, 0, by + 54, sw, by + 70);
			_font_destroy(hFontSmall);
		}
		else {
			_canvas_drawtext(hCanvas, hFont, textColor, labels[i], -1, DT_CENTER | DT_TOP, 0, by + 42, sw, by + FLOWGRAPH_SIDEBAR_BTN_H);
		}
		_font_destroy(hFont);
	}

	// ★ 更新分隔线坐标
	HEXBRUSH hBrushBtnSep = _brush_create(ExARGB(42, 42, 52, 200));
	_canvas_drawline(hCanvas, hBrushBtnSep, 10, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y1 - 10, sw - 10, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y1 - 10, 1, 0); // 添加与链路
	_canvas_drawline(hCanvas, hBrushBtnSep, 12, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y2 - 6, sw - 12, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y2 - 6, 1, 0);  // 链路内部
	_canvas_drawline(hCanvas, hBrushBtnSep, 10, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y3 - 10, sw - 10, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y3 - 10, 1, 0); // 链路与取消
	_canvas_drawline(hCanvas, hBrushBtnSep, 10, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y4 - 10, sw - 10, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y4 - 10, 1, 0); // 取消与导出
	_canvas_drawline(hCanvas, hBrushBtnSep, 12, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y5 - 6, sw - 12, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y5 - 6, 1, 0);  // 导出内部
	_canvas_drawline(hCanvas, hBrushBtnSep, 10, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y6 - 10, sw - 10, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y6 - 10, 1, 0); // 导入与自定义
	_canvas_drawline(hCanvas, hBrushBtnSep, 10, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y7 - 10, sw - 10, (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y7 - 10, 1, 0); // 自定义与帮助
	_brush_destroy(hBrushBtnSep);

	if (pData->sidebarShowAddPanel) _flowgraph_draw_add_panel(hCanvas, pData, canvasWidth, canvasHeight);
	if (pData->sidebarShowChainPanel) _flowgraph_draw_chain_panel(hCanvas, pData, canvasWidth, canvasHeight);
	if (pData->sidebarHoverBtn == 7) _flowgraph_draw_help_tooltip(hObj, hCanvas, canvasWidth, canvasHeight); // ★ 原9改7
	if (pData->sidebarShowCancelPanel) _flowgraph_draw_cancel_panel(hCanvas, pData, canvasWidth, canvasHeight);
	if (pData->sidebarShowCustomPanel) _flowgraph_draw_custom_panel(hCanvas, pData, canvasWidth, canvasHeight);
}

void _flowgraph_draw_help_tooltip(HEXOBJ hObj, HEXCANVAS hCanvas, FLOAT canvasWidth, FLOAT canvasHeight)
{
	LPCWSTR helpText =
		L"操作帮助:\n"
		L"【快捷键操作】\n"
		L"Ctrl+C        ：复制选中的节点(自动包含选区内连线)\n"
		L"Ctrl+X        ：剪切选中的节点(自动包含选区内连线)\n"
		L"Ctrl+V        ：粘贴节点到鼠标位置(智能防越界)\n"
		L"Delete        ：删除选中的连接线\n"
		L"Shift+Delete  ：删除选中的多个节点(忙碌受保护)\n"
		L"【鼠标与画布控制】\n"
		L"空白处左键拖拽：框选多个节点\n"
		L"多选后左键拖拽：整体移动选中的节点组\n"
		L"鼠标中键拖动  ：平移画布\n"
		L"Ctrl+鼠标滚轮 ：缩放画布\n"
		L"鼠标右键节点  ：打开上下文菜单(执行/取消/清空)\n"
		L"拖拽图片到节点：快速加载本地图片(仅限本地图卡片)\n"
		L"【节点编辑与交互】\n"
		L"左键拖动端口  ：手动创建连接线\n"
		L"双击编辑框    ：原位快速编辑文本内容\n"
		L"选中特定节点  ：弹出右下侧浮动编辑面板\n"
		L"左键拖动白点  ：调整连线贝塞尔曲率\n"
		L"拖动组件右下角：调整编辑/图片/视频框大小\n"
		L"点击组合框箭头：展开下拉选项\n"
		L"点击视频控件  ：播放/暂停及拖拽进度条\n"
		L"点击双按钮(+/-)：动态增减参考图/音频端口\n"
		L"【节点状态与提示】\n"
		L"鼠标悬停标题  ：查看详细执行信息、拓扑及输入图片预览\n"
		L"  (支持未执行时透传预览上游本地图，每行3张带端口标识)\n"
		L"右上角圆点标识状态(执行中带蓝色外发光)：\n"
		L"  ⚪ 灰色 - 未执行  🔵 蓝色 - 执行中\n"
		L"  🟡 黄色 - 排队中  🟢 绿色 - 成功  🔴 红色 - 失败\n"
		L"【侧边栏功能】\n"
		L"[+] 添加      ：选择类型，新建节点\n"
		L"[|▶]链路从头  ：从头执行选定的单链路\n"
		L"[‖▶]链路继续  ：继续执行选定的单链路\n"
		L"[■] 取消      ：强制取消执行/排队的节点\n"
		L"[↑] 导出      ：保存YAML工程(自动打包素材到同名文件夹)\n"
		L"[↓] 导入      ：从YAML工程恢复(自动从同名文件夹加载素材)\n"
		L"[田] 自定义   ：显示自定义功能菜单\n"
		L"[?] 帮助      ：显示本操作面板\n"
		L"【核心机制与重要提示】\n"
		L"1.数据缓存：断开连线时输入数据会保留不丢失\n"
		L"2.级联失效：修改数据/删除连线会使下游重置为未执行\n"
		L"3.忙碌保护：执行中的节点及链路无法被删除或剪切\n";

	FLOAT mouseX = (FLOAT)Ex_ObjGetLong(hObj, FLOWGRAPH_LONG_MOUSE_X);
	FLOAT mouseY = (FLOAT)Ex_ObjGetLong(hObj, FLOWGRAPH_LONG_MOUSE_Y);

	HEXFONT hFont = _font_createfromfamily(L"微软雅黑", 12, 0);
	if (!hFont) return;
	FLOAT textW = 0, textH = 0;
	_canvas_calctextsize(hCanvas, hFont, helpText, -1, DT_LEFT | DT_TOP | DT_WORDBREAK, 500, 9999.0f, &textW, &textH);

	FLOAT tipX = mouseX + 15;
	FLOAT tipY = mouseY - textH / 2;

	if (tipX + textW + 16 > canvasWidth) tipX = mouseX - textW - 20;
	if (tipX < 5) tipX = 5;
	if (tipY + textH + 16 > canvasHeight) tipY = canvasHeight - textH - 16;
	if (tipY < 5) tipY = 5;

	// 背景
	HEXBRUSH hBrushBg = _brush_create(ExARGB(30, 30, 35, 235));
	_canvas_fillrect(hCanvas, hBrushBg, tipX - 8, tipY - 8, tipX + textW + 8, tipY + textH + 8);
	_brush_destroy(hBrushBg);

	// 左侧蓝色条
	HEXBRUSH hBrushBlue = _brush_create(ExARGB(100, 200, 255, 255));
	_canvas_fillrect(hCanvas, hBrushBlue, tipX - 8, tipY - 8, tipX - 4, tipY + textH + 8);
	_brush_destroy(hBrushBlue);

	// 边框
	HEXBRUSH hBrushBorder = _brush_create(ExARGB(80, 80, 100, 200));
	_canvas_drawrect(hCanvas, hBrushBorder, tipX - 8, tipY - 8, tipX + textW + 8, tipY + textH + 8, 1.0f, 0);
	_brush_destroy(hBrushBorder);

	// 文本
	_canvas_drawtext(hCanvas, hFont, ExARGB(220, 220, 220, 255), helpText, -1,
		DT_LEFT | DT_TOP | DT_WORDBREAK,
		tipX, tipY, tipX + textW, tipY + textH);

	_font_destroy(hFont);
}

POINTF _flowgraph_get_port_center(EX_FLOWGRAPH_NODE* node, INT slotIndex) {
	EX_FLOWGRAPH_PORT& port = node->ports[slotIndex];
	POINTF pt;
	if (port.portRect.left != 0 || port.portRect.top != 0 || port.portRect.right != 0 || port.portRect.bottom != 0) {
		pt.x = node->x + (port.portRect.left + port.portRect.right) / 2.0f;
		pt.y = node->y + (port.portRect.top + port.portRect.bottom) / 2.0f;
	}
	else if (port.widgetRect.left != 0 || port.widgetRect.top != 0 || port.widgetRect.right != 0 || port.widgetRect.bottom != 0) {
		if (port.portType == FLOWGRAPH_PORTTYPE_INPUT) {
			pt.x = node->x + port.widgetRect.left; pt.y = node->y + (port.widgetRect.top + port.widgetRect.bottom) / 2.0f;
		}
		else if (port.portType == FLOWGRAPH_PORTTYPE_OUTPUT) {
			pt.x = node->x + port.widgetRect.right; pt.y = node->y + (port.widgetRect.top + port.widgetRect.bottom) / 2.0f;
		}
		else { pt.x = node->x + (port.widgetRect.left + port.widgetRect.right) / 2.0f; pt.y = node->y + (port.widgetRect.top + port.widgetRect.bottom) / 2.0f; }
	}
	else { pt.x = node->x; pt.y = node->y; }
	return pt;
}
// ==================== 绘制节点 ====================
void _flowgraph_drawnode(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, EX_FLOWGRAPH_NODE* node, BOOL selected, FLOAT zoom, INT scrollX, INT scrollY)
{
	FLOAT nodeX = node->x * zoom - scrollX; FLOAT nodeY = node->y * zoom - scrollY;
	FLOAT nodeWidth = node->width * zoom; FLOAT nodeHeight = node->height * zoom;

	HEXBRUSH hBrushNode = _brush_create(ExARGB(23, 23, 23, 255));
	HEXBRUSH hBrushBorder = _brush_create(selected ? ExARGB(255, 255, 255, 255) : ExRGB2ARGB(0, 255));
	_canvas_fillroundedrect(hCanvas, hBrushNode, nodeX, nodeY, nodeX + nodeWidth, nodeY + nodeHeight, 5.0f * zoom, 5.0f * zoom);
	_canvas_drawroundedrect(hCanvas, hBrushBorder, nodeX, nodeY, nodeX + nodeWidth, nodeY + nodeHeight, 5.0f * zoom, 5.0f * zoom, 2.0f * zoom, 0);

	if (node->executionStatus == FLOWGRAPH_EXEC_STATUS_RUNNING) {
		HEXBRUSH hBrushGlow = _brush_create(ExARGB(0, 120, 255, 120));
		_canvas_drawroundedrect(hCanvas, hBrushGlow, nodeX - 2, nodeY - 2, nodeX + nodeWidth + 2, nodeY + nodeHeight + 2, 6.0f * zoom, 6.0f * zoom, 2.0f * zoom, 0);
		_brush_destroy(hBrushGlow);
	}

	EXARGB tagColor = ExARGB(100, 100, 100, 255);
	EX_FLOWGRAPH_CARD_DESCRIPTOR* desc = _flowgraph_find_card_descriptor(pData, node->cardType);
	if (desc) tagColor = desc->tagColor;
	HEXBRUSH hBrushTag = _brush_create(tagColor);
	FLOAT titleHeight = 30.0f * zoom;
	EX_RECTF rcTitle = { nodeX + 2.0f * zoom, nodeY + 2.0f * zoom, nodeX + nodeWidth - 2.0f * zoom, nodeY + titleHeight - 2.0f * zoom };
	_canvas_fillrect(hCanvas, hBrushTag, rcTitle.left, rcTitle.top, rcTitle.left + 4.0f * zoom, rcTitle.bottom);
	_brush_destroy(hBrushTag);

	EXARGB statusDotColor = _flowgraph_get_status_color(node->executionStatus);
	HEXBRUSH hBrushStatusDot = _brush_create(statusDotColor);
	FLOAT dotRadius = 4.0f * zoom;
	FLOAT dotX = rcTitle.right - 10.0f * zoom;
	FLOAT dotY = (rcTitle.top + rcTitle.bottom) / 2.0f;
	_canvas_fillellipse(hCanvas, hBrushStatusDot, dotX, dotY, dotRadius, dotRadius);
	_brush_destroy(hBrushStatusDot);

	WCHAR titleText[256];
	swprintf_s(titleText, 256, L"%s(Id:%d)", node->title, node->id);
	HEXFONT hFontTitle = _font_createfromfamily(L"Arial", 12 * zoom, FONT_STYLE_BOLD);
	_canvas_drawtext(hCanvas, hFontTitle, ExARGB(217, 217, 217, 255), titleText, -1, DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS, rcTitle.left + 4.0f * zoom, rcTitle.top, rcTitle.right - 16.0f * zoom, rcTitle.bottom);
	_font_destroy(hFontTitle);

	// ==================== 1. 绘制内部 Widget ====================
	for (INT i = 0; i < node->portCount; i++) {
		EX_FLOWGRAPH_PORT& port = node->ports[i];
		BOOL showWidget = (port.portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) || (port.portType == FLOWGRAPH_PORTTYPE_INPUT && !port.isConnected);
		if (port.widgetType != 0 && showWidget) {
			FLOAT wx = nodeX + port.widgetRect.left * zoom; FLOAT wy = nodeY + port.widgetRect.top * zoom;
			FLOAT ww = (port.widgetRect.right - port.widgetRect.left) * zoom; FLOAT wh = (port.widgetRect.bottom - port.widgetRect.top) * zoom;

			// (此处保留原有的 Widget 绘制逻辑：EDIT, IMAGE, COMBO, BUTTON, DUAL_BUTTON, TEXT, VIDEO)
			if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT) {
				HEXBRUSH hBrushEdit = _brush_create(ExARGB(23, 23, 23, 255));
				_canvas_fillrect(hCanvas, hBrushEdit, wx, wy, wx + ww, wy + wh);
				_canvas_drawrect(hCanvas, hBrushBorder, wx, wy, wx + ww, wy + wh, 1.0f * zoom, 0);
				if (port.widgetData) {
					HEXFONT hFont = _font_createfromfamily(L"Arial", 14 * zoom, 0);
					_canvas_drawtext(hCanvas, hFont, ExARGB(217, 217, 217, 255), (LPCWSTR)port.widgetData, -1, DT_LEFT | DT_TOP | DT_WORDBREAK, wx + 5.0f * zoom, wy + 5.0f * zoom, wx + ww - 5.0f * zoom, wy + wh - 5.0f * zoom);
					_font_destroy(hFont);
				}
				_brush_destroy(hBrushEdit);
			}
			else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE) {
				if (port.widgetData) {
					HEXIMAGE hImage = (HEXIMAGE)port.widgetData; INT imgW, imgH;
					_img_getsize(hImage, &imgW, &imgH);
					_canvas_drawimagerectrect(hCanvas, hImage, wx, wy, wx + ww, wy + wh, 0, 0, imgW, imgH, 255);
				}
				else {
					HEXBRUSH hBrushImg = _brush_create(ExARGB(38, 38, 38, 255));
					_canvas_fillrect(hCanvas, hBrushImg, wx, wy, wx + ww, wy + wh); _brush_destroy(hBrushImg);
				}
			}
			else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO) {
				HEXBRUSH hBrushCombo = _brush_create(ExARGB(34, 34, 34, 255));
				_canvas_fillrect(hCanvas, hBrushCombo, wx, wy, wx + ww, wy + wh);

				BOOL isExpanded = (pData->comboExpandedNode == node->id && pData->comboExpandedPortIdx == i);
				// 展开时使用蓝色高亮边框
				HEXBRUSH hBrushComboBorder = _brush_create(isExpanded ? ExARGB(100, 150, 255, 255) : ExRGB2ARGB(0, 255));
				_canvas_drawrect(hCanvas, hBrushComboBorder, wx, wy, wx + ww, wy + wh, 1.0f * zoom, 0);
				_brush_destroy(hBrushComboBorder);

				EX_FLOWGRAPH_NODE_COMBO_DATA* comboData = (EX_FLOWGRAPH_NODE_COMBO_DATA*)port.widgetData;

				// 右侧下拉按钮区域
				FLOAT btnWidth = 24.0f * zoom;
				FLOAT btnX = wx + ww - btnWidth;

				// 绘制下拉按钮背景
				HEXBRUSH hBrushBtnBg = _brush_create(ExARGB(45, 45, 50, 255));
				_canvas_fillrect(hCanvas, hBrushBtnBg, btnX, wy, wx + ww, wy + wh);
				_brush_destroy(hBrushBtnBg);

				// 绘制分隔线
				_canvas_drawline(hCanvas, hBrushBorder, btnX, wy, btnX, wy + wh, 1.0f * zoom, 0);

				// 绘制向下的小三角形
				FLOAT triSize = 5.0f * zoom;
				FLOAT triCx = btnX + btnWidth / 2.0f;
				FLOAT triCy = wy + wh / 2.0f;
				POINTF triPts[3] = {
					{triCx - triSize, triCy - triSize / 2.0f},
					{triCx + triSize, triCy - triSize / 2.0f},
					{triCx, triCy + triSize / 2.0f}
				};
				HEXPATH hPathTri; _path_create(PATH_FLAG_DISABLESCALE, &hPathTri); _path_open(hPathTri);
				_path_beginfigure2(hPathTri, triPts[0].x, triPts[0].y);
				_path_addline(hPathTri, triPts[0].x, triPts[0].y, triPts[1].x, triPts[1].y);
				_path_addline(hPathTri, triPts[1].x, triPts[1].y, triPts[2].x, triPts[2].y);
				_path_addline(hPathTri, triPts[2].x, triPts[2].y, triPts[0].x, triPts[0].y);
				_path_endfigure(hPathTri, TRUE); _path_close(hPathTri);
				_canvas_fillpath(hCanvas, hPathTri, hBrushBorder);
				_path_destroy(hPathTri);

				// 绘制当前选中文本
				if (comboData && comboData->count > 0) {
					HEXFONT hFont = _font_createfromfamily(L"Arial", 10 * zoom, 0);
					_canvas_drawtext(hCanvas, hFont, ExARGB(217, 217, 217, 255), comboData->options[comboData->current], -1,
						DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS,
						wx + 8.0f * zoom, wy, btnX - 5.0f * zoom, wy + wh);
					_font_destroy(hFont);
				}
				_brush_destroy(hBrushCombo);
			}
			else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON) {
				EX_FLOWGRAPH_NODE_BUTTON_DATA* btnData = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)port.widgetData;
				HEXBRUSH hBrushBtn = _brush_create(ExARGB(50, 50, 60, 255));
				_canvas_fillroundedrect(hCanvas, hBrushBtn, wx, wy, wx + ww, wy + wh, 4.0f * zoom, 4.0f * zoom);
				_canvas_drawroundedrect(hCanvas, hBrushBorder, wx, wy, wx + ww, wy + wh, 4.0f * zoom, 4.0f * zoom, 1.0f * zoom, 0);
				if (btnData && btnData->caption) {
					HEXFONT hFont = _font_createfromfamily(L"Arial", 12 * zoom, 0);
					_canvas_drawtext(hCanvas, hFont, ExARGB(217, 217, 217, 255), btnData->caption, -1, DT_CENTER | DT_VCENTER, wx, wy, wx + ww, wy + wh);
					_font_destroy(hFont);
				}
				_brush_destroy(hBrushBtn);
			}
			else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON) {
				EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* dualBtnData = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)port.widgetData;
				FLOAT gap = 6.0f * zoom; FLOAT halfW = (ww - gap) / 2.0f;
				HEXBRUSH hBrushDualBtn = _brush_create(ExARGB(50, 50, 60, 255));
				_canvas_fillroundedrect(hCanvas, hBrushDualBtn, wx, wy, wx + halfW, wy + wh, 4.0f * zoom, 4.0f * zoom);
				_canvas_drawroundedrect(hCanvas, hBrushBorder, wx, wy, wx + halfW, wy + wh, 4.0f * zoom, 4.0f * zoom, 1.0f * zoom, 0);
				if (dualBtnData && dualBtnData->caption1) {
					HEXFONT hFontDual = _font_createfromfamily(L"Arial", 11 * zoom, 0);
					_canvas_drawtext(hCanvas, hFontDual, ExARGB(217, 217, 217, 255), dualBtnData->caption1, -1, DT_CENTER | DT_VCENTER, wx + 2, wy, wx + halfW - 2, wy + wh);
					_font_destroy(hFontDual);
				}
				FLOAT rightX = wx + halfW + gap;
				_canvas_fillroundedrect(hCanvas, hBrushDualBtn, rightX, wy, rightX + halfW, wy + wh, 4.0f * zoom, 4.0f * zoom);
				_canvas_drawroundedrect(hCanvas, hBrushBorder, rightX, wy, rightX + halfW, wy + wh, 4.0f * zoom, 4.0f * zoom, 1.0f * zoom, 0);
				if (dualBtnData && dualBtnData->caption2) {
					HEXFONT hFontDual = _font_createfromfamily(L"Arial", 11 * zoom, 0);
					_canvas_drawtext(hCanvas, hFontDual, ExARGB(217, 217, 217, 255), dualBtnData->caption2, -1, DT_CENTER | DT_VCENTER, rightX + 2, wy, rightX + halfW - 2, wy + wh);
					_font_destroy(hFontDual);
				}
				_brush_destroy(hBrushDualBtn);
			}
			else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT) {
				HEXBRUSH hBrushTextBg = _brush_create(ExARGB(30, 30, 35, 255));
				_canvas_fillrect(hCanvas, hBrushTextBg, wx, wy, wx + ww, wy + wh);
				_canvas_drawrect(hCanvas, hBrushBorder, wx, wy, wx + ww, wy + wh, 1.0f * zoom, 0);
				if (port.widgetData) {
					HEXFONT hFont = _font_createfromfamily(L"Arial", 12 * zoom, 0);
					_canvas_drawtext(hCanvas, hFont, ExARGB(217, 217, 217, 255), (LPCWSTR)port.widgetData, -1, DT_LEFT | DT_TOP | DT_WORDBREAK, wx + 5.0f * zoom, wy + 5.0f * zoom, wx + ww - 5.0f * zoom, wy + wh - 5.0f * zoom);
					_font_destroy(hFont);
				}
				_brush_destroy(hBrushTextBg);
			}
			else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO) {
				_flowgraph_draw_video_widget(hCanvas, node, &port, zoom, scrollX, scrollY);
			}

			// 右下角调整大小手柄
			if (port.widgetType != FLOWGRAPH_NODEDATA_TYPE_BUTTON && port.widgetType != FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON) {
				HEXBRUSH hBrushHandle = _brush_create(ExARGB(150, 150, 150, 255));
				POINTF p1 = { wx + ww, wy + wh - 6 * zoom }, p2 = { wx + ww, wy + wh }, p3 = { wx + ww - 6 * zoom, wy + wh };
				HEXPATH hPath; _path_create(PATH_FLAG_DISABLESCALE, &hPath); _path_open(hPath); _path_beginfigure2(hPath, p1.x, p1.y);
				_path_addline(hPath, p1.x, p1.y, p2.x, p2.y); _path_addline(hPath, p2.x, p2.y, p3.x, p3.y);
				_path_endfigure(hPath, TRUE); _path_close(hPath); _canvas_fillpath(hCanvas, hPath, hBrushHandle); _path_destroy(hPath); _brush_destroy(hBrushHandle);
			}
		}
	}

	// ==================== 2. ★ 绘制外部端口 (INPUT 和 OUTPUT) ====================
	for (INT i = 0; i < node->portCount; i++) {
		EX_FLOWGRAPH_PORT& port = node->ports[i];
		if (port.portType == FLOWGRAPH_PORTTYPE_INPUT || port.portType == FLOWGRAPH_PORTTYPE_OUTPUT) {
			// ★ 视觉圆心外扩：INPUT在 x=-15, OUTPUT在 x=width+15
			FLOAT visualCx = (port.portType == FLOWGRAPH_PORTTYPE_INPUT) ?
				(nodeX - 15.0f * zoom) :
				(nodeX + nodeWidth + 15.0f * zoom);
			FLOAT cy = nodeY + (port.portRect.top + port.portRect.bottom) / 2.0f * zoom;

			EXARGB portColor = _flowgraph_get_port_color(port.dataType);
			BOOL isHover = (pData->hoverNode == node->id && pData->hoverSlot == i &&
				pData->hoverSlotType == (port.portType == FLOWGRAPH_PORTTYPE_OUTPUT ? FLOWGRAPH_SLOTTYPE_OUTPUT : FLOWGRAPH_SLOTTYPE_INPUT));
			BOOL isSelectedPort = (pData->selectedPortNode == node->id && pData->selectedPortIndex == i);

			// ★ 绘制圆圈 (画大一点：原 8/6 改为 11/8)
			FLOAT radius = (isHover || isSelectedPort) ? 11.0f * zoom : 8.0f * zoom;
			HEXBRUSH hBrushPort = _brush_create(isSelectedPort ? ExARGB(255, 255, 100, 255) : (isHover ? ExARGB(255, 255, 255, 255) : portColor));

			_canvas_fillellipse(hCanvas, hBrushPort, visualCx, cy, radius, radius);
			_canvas_drawellipse(hCanvas, hBrushBorder, visualCx, cy, radius, radius, 1.5f * zoom, 0);
			_brush_destroy(hBrushPort);

			// ★ 悬停时显示端口名称
			if (isHover) {
				HEXFONT hFontSlot = _font_createfromfamily(L"微软雅黑", 11 * zoom, 0);
				EXARGB textColor = ExARGB(240, 240, 240, 255);
				if (port.portType == FLOWGRAPH_PORTTYPE_INPUT) {
					// ★ 修复：向左扩展宽度至 150px，并强制单行显示，防止文字换行
					_canvas_drawtext(hCanvas, hFontSlot, textColor, port.name, -1,
						DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS,
						visualCx - 160.0f * zoom, cy - 10.0f * zoom,
						visualCx - 12.0f * zoom, cy + 10.0f * zoom);
				}
				else {
					// ★ 优化：同样增加单行和省略号标志，防止极端长文本换行
					_canvas_drawtext(hCanvas, hFontSlot, textColor, port.name, -1,
						DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS,
						visualCx + 12.0f * zoom, cy - 10.0f * zoom,
						visualCx + 160.0f * zoom + 150.0f * zoom, cy + 10.0f * zoom);
				}
				_font_destroy(hFontSlot);
			}
		}
	}
	_brush_destroy(hBrushNode); _brush_destroy(hBrushBorder);
}
EXARGB _flowgraph_get_port_color(INT dataType) {
	switch (dataType) {
	case FLOWGRAPH_DATATYPE_IMAGE:  return ExARGB(180, 80, 80, 255);
	case FLOWGRAPH_DATATYPE_STRING: return ExARGB(80, 80, 180, 255);
	case FLOWGRAPH_DATATYPE_COMBO:  return ExARGB(150, 50, 150, 255);
	case FLOWGRAPH_DATATYPE_VIDEO:  return ExARGB(180, 130, 50, 255);
	case FLOWGRAPH_DATATYPE_AUDIO:  return ExARGB(255, 215, 0, 255);
	default: return ExARGB(200, 200, 200, 255);
	}
}
void _flowgraph_draw_triangle_arrow(HEXCANVAS hCanvas, HEXBRUSH hBrush, FLOAT x, FLOAT y, FLOAT size, BOOL left) {
	POINTF points[3];
	if (left) { points[0] = { x + size, y }; points[1] = { x, y + size / 2.0f }; points[2] = { x + size, y + size }; }
	else { points[0] = { x, y }; points[1] = { x + size, y + size / 2.0f }; points[2] = { x, y + size }; }
	HEXPATH hPath; _path_create(PATH_FLAG_DISABLESCALE, &hPath); _path_open(hPath); _path_beginfigure2(hPath, points[0].x, points[0].y);
	_path_addline(hPath, points[0].x, points[0].y, points[1].x, points[1].y);
	_path_addline(hPath, points[1].x, points[1].y, points[2].x, points[2].y);
	_path_addline(hPath, points[2].x, points[2].y, points[0].x, points[0].y);
	_path_endfigure(hPath, TRUE); _path_close(hPath);
	_canvas_fillpath(hCanvas, hPath, hBrush); _path_destroy(hPath);
}
// ==================== 布局 ====================
void _flowgraph_updatelayout(HEXOBJ hObj)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData) return;
	RECT rcClient; Ex_ObjGetClientRect(hObj, &rcClient);
	INT clientWidth = rcClient.right - rcClient.left;
	INT clientHeight = rcClient.bottom - rcClient.top;
	FLOAT maxX = 0.0f, maxY = 0.0f;
	for (INT i = 0; i < pData->nodeCount; i++) {
		EX_FLOWGRAPH_NODE* node = &pData->nodes[i];
		FLOAT nodeRight = node->x + node->width; FLOAT nodeBottom = node->y + node->height;
		if (nodeRight > maxX) maxX = nodeRight; if (nodeBottom > maxY) maxY = nodeBottom;
	}
	FLOAT scaledContentW = maxX * pData->zoom; FLOAT scaledContentH = maxY * pData->zoom;
	INT hScrollMax = 1;
	if (scaledContentW > clientWidth) { hScrollMax = (INT)scaledContentW + 50; pData->panOffset.x = __min(pData->panOffset.x, hScrollMax); }
	else { pData->panOffset.x = 0; Ex_ObjScrollSetPos(hObj, SCROLLBAR_TYPE_HORZ, 0, TRUE); }
	Ex_ObjScrollSetRange(hObj, SCROLLBAR_TYPE_HORZ, 0, hScrollMax, TRUE);
	INT vScrollMax = 1;
	if (scaledContentH > clientHeight) { vScrollMax = (INT)scaledContentH + 50; pData->panOffset.y = __min(pData->panOffset.y, vScrollMax); }
	else { pData->panOffset.y = 0; Ex_ObjScrollSetPos(hObj, SCROLLBAR_TYPE_VERT, 0, TRUE); }
	Ex_ObjScrollSetRange(hObj, SCROLLBAR_TYPE_VERT, 0, vScrollMax, TRUE);
	// ★ 更新浮动编辑面板位置
	if (pData->showEditPanel) {
		_flowgraph_update_edit_panel(hObj);
	}
}
// ==================== 鼠标移动 ====================
void _flowgraph_onmousemove(HEXOBJ hObj, INT x, INT y)
{
	Ex_ObjSetLong(hObj, FLOWGRAPH_LONG_MOUSE_X, x); 
	Ex_ObjSetLong(hObj, FLOWGRAPH_LONG_MOUSE_Y, y);
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0); if (!pData) return;
	if (pData->showContextMenu) {
		FLOAT menuW = FLOWGRAPH_CONTEXT_MENU_WIDTH;
		FLOAT menuH = FLOWGRAPH_CONTEXT_MENU_HEADER * 2 + FLOWGRAPH_CONTEXT_MENU_ITEM_H * 4;
		FLOAT mx = pData->contextMenuX;
		FLOAT my = pData->contextMenuY;

		RECT rcClient;
		Ex_ObjGetClientRect(hObj, &rcClient);
		FLOAT clientW = (FLOAT)(rcClient.right - rcClient.left);
		FLOAT clientH = (FLOAT)(rcClient.bottom - rcClient.top);
		if (mx + menuW > clientW) mx = clientW - menuW;
		if (my + menuH > clientH) my = clientH - menuH;
		if (mx < 0) mx = 0;
		if (my < 0) my = 0;

		INT prevHover = pData->contextMenuHover;
		pData->contextMenuHover = -1;

		if (x >= mx && x <= mx + menuW && y >= my && y <= my + menuH) {
			INT idx = (INT)((y - my - FLOWGRAPH_CONTEXT_MENU_HEADER) / FLOWGRAPH_CONTEXT_MENU_ITEM_H);
			if (idx >= 0 && idx < 4) {
				pData->contextMenuHover = idx;
			}
		}
		if (prevHover != pData->contextMenuHover) Ex_ObjInvalidateRect(hObj, 0);
		return; // 拦截，不处理画布悬停
	}
	// ===== 中键拖拽画布 =====
	if (pData->isPanning) {
		if (pData->comboExpandedNode != -1) {
			pData->comboExpandedNode = -1; pData->comboExpandedPortIdx = -1; pData->comboPanelHoverIdx = -1;
		}
		INT dx = x - pData->panStartMouseX;
		INT dy = y - pData->panStartMouseY;
		INT newScrollX = pData->panStartScrollX - dx;
		INT newScrollY = pData->panStartScrollY - dy;
		INT minH, maxH, minV, maxV;
		Ex_ObjScrollGetRange(hObj, SCROLLBAR_TYPE_HORZ, &minH, &maxH);
		Ex_ObjScrollGetRange(hObj, SCROLLBAR_TYPE_VERT, &minV, &maxV);
		newScrollX = __max(minH, __min(maxH, newScrollX));
		newScrollY = __max(minV, __min(maxV, newScrollY));
		pData->panOffset.x = newScrollX; pData->panOffset.y = newScrollY;
		Ex_ObjScrollSetPos(hObj, SCROLLBAR_TYPE_HORZ, newScrollX, TRUE);
		Ex_ObjScrollSetPos(hObj, SCROLLBAR_TYPE_VERT, newScrollY, TRUE);
		Ex_ObjInvalidateRect(hObj, 0);
		return;
	}

	INT scrollX = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_HORZ); INT scrollY = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_VERT);
	FLOAT virtualX = (x + scrollX) / pData->zoom; FLOAT virtualY = (y + scrollY) / pData->zoom;
	if (pData->isSelecting) {
		pData->selectEndPos = { virtualX, virtualY };
		_flowgraph_update_selection_rect(pData); // 实时计算框选内的节点
		Ex_ObjInvalidateRect(hObj, 0);
		return;
	}
	if (pData->resizingNode != -1) {
		EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, pData->resizingNode);
		if (node) {
			auto& port = node->ports[pData->resizingPortIdx];
			FLOAT dx = virtualX - pData->dragStartPos.x; FLOAT dy = virtualY - pData->dragStartPos.y;
			FLOAT newW = port.widgetWidth + dx; FLOAT newH = port.widgetHeight + dy;
			if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT) { newW = __max(300.0f, newW); /* 高度自动计算 */ }
			else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE) { newW = __max(300.0f, newW); newH = __max(180.0f, newH); }
			else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO) { newW = __max(300.0f, newW); newH = 30; }
			else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT) { newW = __max(300.0f, newW); /* 高度自动计算 */ }
			else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO) { newW = __max(300.0f, newW); newH = __max(180.0f, newH); }
			port.widgetWidth = newW;
			// ★ EDIT和TEXT高度由内容自适应，不跟随鼠标拖拽改变
			if (port.widgetType != FLOWGRAPH_NODEDATA_TYPE_TEXT && port.widgetType != FLOWGRAPH_NODEDATA_TYPE_EDIT) port.widgetHeight = newH;
			pData->dragStartPos = { virtualX, virtualY };
			_flowgraph_calcnodesize(hObj, node); _flowgraph_updatelayout(hObj); Ex_ObjInvalidateRect(hObj, 0);
		}
		return;
	}
	if (pData->videoDragNode != -1) {
		EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, pData->videoDragNode);
		if (node && pData->videoDragPortIdx < node->portCount) {
			EX_FLOWGRAPH_PORT& port = node->ports[pData->videoDragPortIdx];
			if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO && port.widgetData) {
				EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)port.widgetData;
				FLOAT widgetLeft = node->x + port.widgetRect.left;
				FLOAT widgetWidth = port.widgetRect.right - port.widgetRect.left;
				FLOAT progressH = FLOWGRAPH_VIDEO_PROGRESS_H;
				FLOAT ctrlY_local = node->y + port.widgetRect.bottom - FLOWGRAPH_VIDEO_CTRL_HEIGHT;
				// 仅在进度条区域内响应
				if (virtualY >= ctrlY_local && virtualY <= ctrlY_local + progressH + 4 && widgetWidth > 0) {
					FLOAT pos = (virtualX - widgetLeft) / widgetWidth;
					pos = __max(0.0f, __min(1.0f, pos));
					if (pVideo->nDuration > 0) pVideo->nCurrentTime = (INT64)(pos * pVideo->nDuration);
				}
				Ex_ObjInvalidateRect(hObj, 0);
			}
		}
		return;
	}
	else if (pData->draggingControlPoint && pData->selectedConnection != -1) {
		EX_FLOWGRAPH_CONNECTION* conn = _flowgraph_findconnection(pData, pData->selectedConnection);
		if (conn) {
			FLOAT deltaX = virtualX - pData->dragStartPos.x, deltaY = virtualY - pData->dragStartPos.y;
			if (pData->draggingWhichPoint == 1) { conn->controlPoint1.x += deltaX; conn->controlPoint1.y += deltaY; }
			else if (pData->draggingWhichPoint == 2) { conn->controlPoint2.x += deltaX; conn->controlPoint2.y += deltaY; }
			pData->dragStartPos.x = virtualX; pData->dragStartPos.y = virtualY; Ex_ObjInvalidateRect(hObj, 0);
		}
	}
	
	else if (pData->draggingNode != -1) {
		if (pData->comboExpandedNode != -1) {
			pData->comboExpandedNode = -1; pData->comboExpandedPortIdx = -1; pData->comboPanelHoverIdx = -1;
		}

		FLOAT dx = virtualX - pData->dragStartPos.x;
		FLOAT dy = virtualY - pData->dragStartPos.y;

		if (dx != 0.0f || dy != 0.0f) {
			// ★ 核心：多选拖拽逻辑
			if (pData->isDraggingSelection && pData->selectedNodeCount > 0) {
				for (INT i = 0; i < pData->selectedNodeCount; i++) {
					EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, pData->selectedNodes[i]);
					if (node) {
						node->x += dx;
						node->y += dy;
						node->x = __max(0.0f, node->x);
						node->y = __max(0.0f, node->y);

						// 更新与该节点相连的连线控制点，保持曲线跟随
						for (INT c = 0; c < pData->connectionCount; c++) {
							EX_FLOWGRAPH_CONNECTION* conn = &pData->connections[c];
							if (conn->fromNode == node->id) {
								conn->controlPoint1.x += dx;
								conn->controlPoint1.y += dy;
							}
							if (conn->toNode == node->id) {
								conn->controlPoint2.x += dx;
								conn->controlPoint2.y += dy;
							}
						}
						Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_NODE_MOVED, node->id, 0);
					}
				}
			}
			// 兼容旧逻辑 (如果 selectedNodeCount 为 0 但 draggingNode 有值)
			else {
				EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, pData->draggingNode);
				if (node) {
					node->x += dx; node->y += dy;
					node->x = __max(0.0f, node->x); node->y = __max(0.0f, node->y);
					for (INT i = 0; i < pData->connectionCount; i++) {
						EX_FLOWGRAPH_CONNECTION* conn = &pData->connections[i];
						if (conn->fromNode == node->id) { conn->controlPoint1.x += dx; conn->controlPoint1.y += dy; }
						if (conn->toNode == node->id) { conn->controlPoint2.x += dx; conn->controlPoint2.y += dy; }
					}
					Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_NODE_MOVED, node->id, 0);
				}
			}

			pData->dragStartPos.x = virtualX;
			pData->dragStartPos.y = virtualY;
			_flowgraph_updatelayout(hObj);
			Ex_ObjInvalidateRect(hObj, 0);
		}
	}
	else if (pData->connectingSlot != -1) { Ex_ObjInvalidateRect(hObj, 0); }
	else {
		// ===== 侧边栏区域检测 =====
		INT prevSidebarHoverBtn = pData->sidebarHoverBtn;
		INT prevAddPanelHover = pData->sidebarAddPanelHover;
		BOOL prevShowAddPanel = pData->sidebarShowAddPanel;
		INT prevChainPanelHover = pData->sidebarChainPanelHover;
		BOOL prevShowChainPanel = pData->sidebarShowChainPanel;
		BOOL prevShowCustomPanel = pData->sidebarShowCustomPanel;
		INT prevCustomPanelHover = pData->sidebarCustomPanelHover;
		BOOL prevShowCancelPanel = pData->sidebarShowCancelPanel;
		INT prevCancelPanelHover = pData->sidebarCancelPanelHover;

		pData->sidebarHoverBtn = -1;
		pData->sidebarAddPanelHover = -1;
		pData->sidebarChainPanelHover = -1;
		pData->sidebarCustomPanelHover = -1;
		pData->sidebarCancelPanelHover = -1;
		FLOAT btnYs[FLOWGRAPH_SIDEBAR_BTN_COUNT] = {
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y0,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y1,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y2,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y3,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y4,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y5,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y6,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y7
				};

		// 检查侧边栏按钮
		if (x >= 0 && x < FLOWGRAPH_SIDEBAR_WIDTH) {
			for (INT i = 0; i < FLOWGRAPH_SIDEBAR_BTN_COUNT; i++) {
				if (y >= btnYs[i] && y < btnYs[i] + FLOWGRAPH_SIDEBAR_BTN_H) {
					pData->sidebarHoverBtn = i;
					break;
				}
			}
			if (pData->sidebarHoverBtn == 0) {// 添加
				pData->sidebarShowAddPanel = TRUE;
				pData->sidebarShowChainPanel = FALSE;
				pData->sidebarShowCustomPanel = FALSE;
				pData->sidebarShowCancelPanel = FALSE;
			}
			else if (pData->sidebarHoverBtn == 1) {// ★ 链路从头 (原3)
				pData->sidebarShowChainPanel = TRUE;
				pData->sidebarChainPanelMode = 0;
				pData->sidebarShowAddPanel = FALSE;
				pData->sidebarShowCustomPanel = FALSE;
				pData->sidebarShowCancelPanel = FALSE;
			}
			else if (pData->sidebarHoverBtn == 2) {// ★ 链路继续 (原4)
				pData->sidebarShowChainPanel = TRUE;
				pData->sidebarChainPanelMode = 1;
				pData->sidebarShowAddPanel = FALSE;
				pData->sidebarShowCustomPanel = FALSE;
				pData->sidebarShowCancelPanel = FALSE;
			}
			else if (pData->sidebarHoverBtn == 3) { // ★ 取消 (原5)
				pData->sidebarShowCancelPanel = TRUE;
				pData->sidebarShowAddPanel = FALSE; 
				pData->sidebarShowChainPanel = FALSE; 
				pData->sidebarShowCustomPanel = FALSE;
			}
			else if (pData->sidebarHoverBtn == 6) {// ★ 自定义 (原8)
				pData->sidebarShowCustomPanel = TRUE;
				pData->sidebarShowAddPanel = FALSE;
				pData->sidebarShowChainPanel = FALSE;
				pData->sidebarShowCancelPanel = FALSE;
			}
			else {
				pData->sidebarShowAddPanel = FALSE;
				pData->sidebarShowChainPanel = FALSE;
				pData->sidebarShowCustomPanel = FALSE;
				pData->sidebarShowCancelPanel = FALSE;
			}
		}
		// 检查添加面板区域
		else if (prevShowAddPanel) {
			FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
			FLOAT panelY = FLOWGRAPH_SIDEBAR_BTN_Y0;
			FLOAT panelW = FLOWGRAPH_ADD_PANEL_WIDTH;
			FLOAT panelH = FLOWGRAPH_ADD_PANEL_HEADER + (pData->cardRegistryCount > 0 ? pData->cardRegistryCount * FLOWGRAPH_ADD_PANEL_ITEM_H + 4 : 36);

			if (x >= panelX && x < panelX + panelW && y >= panelY && y < panelY + panelH) {
				pData->sidebarShowAddPanel = TRUE;
				if (pData->cardRegistryCount > 0) {
					FLOAT itemStartY = panelY + FLOWGRAPH_ADD_PANEL_HEADER;
					FLOAT itemH = FLOWGRAPH_ADD_PANEL_ITEM_H;
					INT idx = (INT)((y - itemStartY) / itemH);
					if (idx >= 0 && idx < pData->cardRegistryCount) pData->sidebarAddPanelHover = idx;
					else pData->sidebarAddPanelHover = -1;
				}
			}
			else {
				pData->sidebarShowAddPanel = FALSE;
			}
			pData->sidebarShowChainPanel = FALSE;
			pData->sidebarShowCustomPanel = FALSE;
			pData->sidebarShowCancelPanel = FALSE;
		}
		// ★ 检查链路节点选择面板区域
		else if (prevShowChainPanel && pData->nodeCount > 0) {
			INT visibleCount = __min(pData->nodeCount, FLOWGRAPH_CHAIN_PANEL_MAX_VISIBLE);
			FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
			FLOAT panelY = (FLOAT)(pData->sidebarChainPanelMode == 0 ? FLOWGRAPH_SIDEBAR_BTN_Y1 : FLOWGRAPH_SIDEBAR_BTN_Y2);
			FLOAT panelW = FLOWGRAPH_CHAIN_PANEL_WIDTH;
			FLOAT panelH = FLOWGRAPH_CHAIN_PANEL_HEADER + visibleCount * FLOWGRAPH_CHAIN_PANEL_ITEM_H + 4;

			if (x >= panelX && x < panelX + panelW && y >= panelY && y < panelY + panelH) {
				pData->sidebarShowChainPanel = TRUE;
				FLOAT itemStartY = panelY + FLOWGRAPH_CHAIN_PANEL_HEADER;
				FLOAT itemH = FLOWGRAPH_CHAIN_PANEL_ITEM_H;
				INT idx = (INT)((y - itemStartY) / itemH);
				if (idx >= 0 && idx < visibleCount) pData->sidebarChainPanelHover = idx;
			}
			else {
				pData->sidebarShowChainPanel = FALSE;
			}
			pData->sidebarShowAddPanel = FALSE;
			pData->sidebarShowCustomPanel = FALSE;
			pData->sidebarShowCancelPanel = FALSE;
		}
		else if (prevShowCancelPanel && pData->nodeCount > 0) {
			INT busyCount = 0;
			for (INT i = 0; i < pData->nodeCount; i++) {
				if (pData->nodes[i].executionStatus == FLOWGRAPH_EXEC_STATUS_RUNNING || pData->nodes[i].executionStatus == FLOWGRAPH_EXEC_STATUS_PENDING) busyCount++;
			}
			INT visibleCount = __min(busyCount, FLOWGRAPH_CANCEL_PANEL_MAX_VISIBLE);
			FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
			FLOAT panelY = (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y3; // ★ Y5
			FLOAT panelW = FLOWGRAPH_CANCEL_PANEL_WIDTH;
			FLOAT panelH = FLOWGRAPH_CANCEL_PANEL_HEADER + visibleCount * FLOWGRAPH_CANCEL_PANEL_ITEM_H + 4;
			if (x >= panelX && x < panelX + panelW && y >= panelY && y < panelY + panelH) {
				pData->sidebarShowCancelPanel = TRUE;
				FLOAT itemStartY = panelY + FLOWGRAPH_CANCEL_PANEL_HEADER;
				FLOAT itemH = FLOWGRAPH_CANCEL_PANEL_ITEM_H;
				INT idx = (INT)((y - itemStartY) / itemH);
				if (idx >= 0 && idx < visibleCount) pData->sidebarCancelPanelHover = idx;
			}
			else {
				pData->sidebarShowCancelPanel = FALSE;
			}
			pData->sidebarShowAddPanel = FALSE; 
			pData->sidebarShowChainPanel = FALSE; 
			pData->sidebarShowCustomPanel = FALSE;
		}
		// ★ 检查自定义面板区域
		else if (prevShowCustomPanel && pData->customItemCount > 0) {
			INT visibleCount = __min(pData->customItemCount, FLOWGRAPH_CUSTOM_PANEL_MAX_VISIBLE);
			FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
			FLOAT panelY = (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y6;
			FLOAT panelW = FLOWGRAPH_CUSTOM_PANEL_WIDTH;
			FLOAT panelH = FLOWGRAPH_CUSTOM_PANEL_HEADER + visibleCount * FLOWGRAPH_CUSTOM_PANEL_ITEM_H + 4;

			if (x >= panelX && x < panelX + panelW && y >= panelY && y < panelY + panelH) {
				pData->sidebarShowCustomPanel = TRUE;
				FLOAT itemStartY = panelY + FLOWGRAPH_CUSTOM_PANEL_HEADER;
				FLOAT itemH = FLOWGRAPH_CUSTOM_PANEL_ITEM_H;
				INT idx = (INT)((y - itemStartY) / itemH);
				if (idx >= 0 && idx < visibleCount) pData->sidebarCustomPanelHover = idx;
			}
			else {
				pData->sidebarShowCustomPanel = FALSE;
			}
			pData->sidebarShowAddPanel = FALSE;
			pData->sidebarShowChainPanel = FALSE;
			pData->sidebarShowCancelPanel = FALSE;
		}
		// ★ 无条目时也显示空面板，需检测关闭
		else if (prevShowCustomPanel && pData->customItemCount == 0) {
			FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
			FLOAT panelY = (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y6;
			FLOAT panelW = FLOWGRAPH_CUSTOM_PANEL_WIDTH;
			FLOAT panelH = FLOWGRAPH_CUSTOM_PANEL_HEADER + 36;

			if (x >= panelX && x < panelX + panelW && y >= panelY && y < panelY + panelH) {
				pData->sidebarShowCustomPanel = TRUE;
			}
			else {
				pData->sidebarShowCustomPanel = FALSE;
			}
			pData->sidebarShowAddPanel = FALSE;
			pData->sidebarShowChainPanel = FALSE;
			pData->sidebarShowCancelPanel = FALSE;
		}
		else {
			pData->sidebarShowAddPanel = FALSE;
			pData->sidebarShowChainPanel = FALSE;
			pData->sidebarShowCustomPanel = FALSE; // ✅ 补充：原代码此处遗漏了CustomPanel
			pData->sidebarShowCancelPanel = FALSE;
		}
		// ===== 浮动编辑面板按钮悬停 =====
		if (pData->showEditPanel) {
			FLOAT px, py, pw, ph;
			_flowgraph_get_edit_panel_rect(hObj, &px, &py, &pw, &ph);
			FLOAT btnX = px + pw - 120, btnY = py + ph - 45, btnW = 110, btnH = 35;
			BOOL wasHover = pData->editPanelBtnHover;
			pData->editPanelBtnHover = (x >= btnX && x <= btnX + btnW && y >= btnY && y <= btnY + btnH);
			if (wasHover != pData->editPanelBtnHover) Ex_ObjInvalidateRect(hObj, 0);
		}
		// 在侧边栏或面板区域，跳过画布悬停检测
		if (pData->sidebarHoverBtn != -1 || pData->sidebarShowAddPanel || pData->sidebarShowChainPanel || pData->sidebarShowCustomPanel || pData->sidebarShowCancelPanel) {
			pData->hoverNode = -1; pData->hoverSlot = -1; pData->hoverSlotType = -1;
			pData->hoverTitleNode = -1;
			if (prevSidebarHoverBtn != pData->sidebarHoverBtn || prevAddPanelHover != pData->sidebarAddPanelHover ||
				prevShowAddPanel != pData->sidebarShowAddPanel || prevChainPanelHover != pData->sidebarChainPanelHover ||
				prevShowChainPanel != pData->sidebarShowChainPanel ||
				prevCustomPanelHover != pData->sidebarCustomPanelHover ||       // ★ 新增
				prevShowCustomPanel != pData->sidebarShowCustomPanel ||
				prevShowCancelPanel != pData->sidebarShowCancelPanel ||
				prevCancelPanelHover != pData->sidebarCancelPanelHover)
				Ex_ObjInvalidateRect(hObj, 0);
			return;
		}

		// 面板刚关闭，刷新
		if ((prevShowAddPanel && !pData->sidebarShowAddPanel) || (prevShowChainPanel && !pData->sidebarShowChainPanel) || (prevShowCustomPanel && !pData->sidebarShowCustomPanel))
			Ex_ObjInvalidateRect(hObj, 0);
		
		// ★ 组合框下拉面板悬停检测
		if (pData->comboExpandedNode != -1) {
			EX_FLOWGRAPH_NODE* expNode = _flowgraph_findnode(pData, pData->comboExpandedNode);
			if (expNode && pData->comboExpandedPortIdx < expNode->portCount) {
				EX_FLOWGRAPH_PORT& expPort = expNode->ports[pData->comboExpandedPortIdx];
				if (expPort.widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO && expPort.widgetData) {
					EX_FLOWGRAPH_NODE_COMBO_DATA* comboData = (EX_FLOWGRAPH_NODE_COMBO_DATA*)expPort.widgetData;
					FLOAT wx = expNode->x + expPort.widgetRect.left;
					FLOAT wy = expNode->y + expPort.widgetRect.bottom;
					FLOAT ww = expPort.widgetRect.right - expPort.widgetRect.left;
					FLOAT itemH = 28.0f; // 虚拟坐标不乘zoom
					FLOAT panelH = comboData->count * itemH;

					if (virtualX >= wx && virtualX <= wx + ww && virtualY >= wy && virtualY <= wy + panelH) {
						INT idx = (INT)((virtualY - wy) / itemH);
						if (idx >= 0 && idx < comboData->count) {
							if (pData->comboPanelHoverIdx != idx) {
								pData->comboPanelHoverIdx = idx;
								Ex_ObjInvalidateRect(hObj, 0);
							}
						}
					}
					else {
						if (pData->comboPanelHoverIdx != -1) {
							pData->comboPanelHoverIdx = -1;
							Ex_ObjInvalidateRect(hObj, 0);
						}
					}
				}
			}
		}

		// ===== 原有画布悬停检测 =====
		INT prevHoverNode = pData->hoverNode, prevHoverSlot = pData->hoverSlot, prevHoverSlotType = pData->hoverSlotType;
		INT prevHoverTitleNode = pData->hoverTitleNode;
		pData->hoverNode = -1; pData->hoverSlot = -1; pData->hoverSlotType = -1;
		pData->hoverTitleNode = -1;

		// ★ 核心修复：由于端口圆点被放大并外扩到了节点外部（如 x=-30 到 0），
		// _flowgraph_find_topmost_node_at 的节点矩形包围盒无法命中外部端口，
		// 导致鼠标一移到圆点上 topmostNode 就返回 NULL，文字瞬间消失。
		// 因此，必须独立遍历所有节点（按 Z-order 从顶到底）来检测端口悬停。

		BOOL portHovered = FALSE;

		// 1. 优先检测选中节点（它始终绘制在最顶层）
		if (pData->selectedNode != -1) {
			EX_FLOWGRAPH_NODE* selNode = _flowgraph_findnode(pData, pData->selectedNode);
			if (selNode) {
				for (INT j = 0; j < selNode->portCount; j++) {
					if (selNode->ports[j].portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) continue;
					RECT rc = selNode->ports[j].portRect;
					if (virtualX >= selNode->x + rc.left && virtualX <= selNode->x + rc.right &&
						virtualY >= selNode->y + rc.top && virtualY <= selNode->y + rc.bottom) {
						pData->hoverNode = selNode->id;
						pData->hoverSlot = j;
						pData->hoverSlotType = selNode->ports[j].portType == FLOWGRAPH_PORTTYPE_OUTPUT ? FLOWGRAPH_SLOTTYPE_OUTPUT : FLOWGRAPH_SLOTTYPE_INPUT;
						portHovered = TRUE;
						break;
					}
				}
			}
		}

		// 2. 如果选中节点未命中，倒序遍历其他节点（数组末尾的节点 Z-order 更高）
		if (!portHovered) {
			for (INT i = pData->nodeCount - 1; i >= 0; i--) {
				EX_FLOWGRAPH_NODE* node = &pData->nodes[i];
				if (node->id == pData->selectedNode) continue; // 跳过已检测的选中节点

				for (INT j = 0; j < node->portCount; j++) {
					if (node->ports[j].portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) continue;
					RECT rc = node->ports[j].portRect;
					if (virtualX >= node->x + rc.left && virtualX <= node->x + rc.right &&
						virtualY >= node->y + rc.top && virtualY <= node->y + rc.bottom) {
						pData->hoverNode = node->id;
						pData->hoverSlot = j;
						pData->hoverSlotType = node->ports[j].portType == FLOWGRAPH_PORTTYPE_OUTPUT ? FLOWGRAPH_SLOTTYPE_OUTPUT : FLOWGRAPH_SLOTTYPE_INPUT;
						portHovered = TRUE;
						break;
					}
				}
				if (portHovered) break;
			}
		}

		// 3. 如果端口没有悬停，再检测标题区域悬停（标题在节点内部，可以使用原有逻辑）
		if (!portHovered) {
			for (INT i = pData->nodeCount - 1; i >= 0; i--) {
				EX_FLOWGRAPH_NODE* node = &pData->nodes[i];
				FLOAT titleHeight = 30.0f;
				if (virtualX >= node->x && virtualX <= node->x + node->width &&
					virtualY >= node->y && virtualY <= node->y + titleHeight) {
					pData->hoverTitleNode = node->id;
					break;
				}
			}
		}

		if (prevHoverNode != pData->hoverNode || prevHoverSlot != pData->hoverSlot || prevHoverSlotType != pData->hoverSlotType ||
			prevHoverTitleNode != pData->hoverTitleNode)
			Ex_ObjInvalidateRect(hObj, 0);
	}
}
// ==================== 鼠标左键按下 ====================
void _flowgraph_onlbuttondown(HEXOBJ hObj, INT x, INT y)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0); if (!pData || pData->draggingControlPoint) return;
	INT scrollX = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_HORZ); INT scrollY = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_VERT);
	if (pData->showContextMenu) {
		FLOAT menuW = FLOWGRAPH_CONTEXT_MENU_WIDTH;
		FLOAT menuH = FLOWGRAPH_CONTEXT_MENU_HEADER * 2 + FLOWGRAPH_CONTEXT_MENU_ITEM_H * 4;
		FLOAT mx = pData->contextMenuX;
		FLOAT my = pData->contextMenuY;

		RECT rcClient;
		Ex_ObjGetClientRect(hObj, &rcClient);
		FLOAT clientW = (FLOAT)(rcClient.right - rcClient.left);
		FLOAT clientH = (FLOAT)(rcClient.bottom - rcClient.top);
		if (mx + menuW > clientW) mx = clientW - menuW;
		if (my + menuH > clientH) my = clientH - menuH;
		if (mx < 0) mx = 0;
		if (my < 0) my = 0;

		if (x >= mx && x <= mx + menuW && y >= my && y <= my + menuH) {
			INT idx = (INT)((y - my - FLOWGRAPH_CONTEXT_MENU_HEADER) / FLOWGRAPH_CONTEXT_MENU_ITEM_H);
			if (idx >= 0 && idx < 4) {
				EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, pData->contextMenuNodeId);
				BOOL disabled = FALSE;
				if (node && idx == 2) { // 如果节点不在运行，禁用“取消执行”
					if (node->executionStatus != FLOWGRAPH_EXEC_STATUS_RUNNING && node->executionStatus != FLOWGRAPH_EXEC_STATUS_PENDING) {
						disabled = TRUE;
					}
				}
				// ★ 新增：清空数据的禁用判断（与绘制逻辑保持一致，防止点击灰显项）
				if (node && idx == 3) {
					if (node->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_TEXT ||
						node->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_IMAGE ||
						node->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_AUDIO) disabled = TRUE;
				}

				if (!disabled && node) {
					if (idx == 0) {
						// ★★★ 新增：从头执行前，先清空该节点及其下游链路的输出与缓存数据 ★★★
						// 作用：清除旧的生成图/视频/文本，并级联失效下游节点，确保UI无旧数据残留
						_flowgraph_clear_node_outputs(hObj, node);

						Ex_ObjSendMessage(hObj, FLOWGRAPH_MESSAGE_EXECUTE_CHAIN, FLOWGRAPH_EXECUTE_MODE_FRESH, node->id);
					}
					else if (idx == 1) {
						Ex_ObjSendMessage(hObj, FLOWGRAPH_MESSAGE_EXECUTE_CHAIN, FLOWGRAPH_EXECUTE_MODE_CONTINUE, node->id);
					}
					else if (idx == 2) {
						_flowgraph_cancel_node(hObj, node->id);
					}
					else if (idx == 3) {
						// ★ 执行清空数据
						_flowgraph_clear_node_outputs(hObj, node);
					}
				}
			}
		}
		// 无论点击菜单内还是菜单外，都关闭菜单
		pData->showContextMenu = FALSE;
		pData->contextMenuHover = -1;
		Ex_ObjInvalidateRect(hObj, 0);
		return; // 拦截点击，不触发画布其他逻辑
	}
	// ===== 浮动编辑面板区域检测 =====
	if (pData->showEditPanel) {
		FLOAT px, py, pw, ph;
		_flowgraph_get_edit_panel_rect(hObj, &px, &py, &pw, &ph);
		if (x >= px && x <= px + pw && y >= py && y <= py + ph) {
			// 检测提交按钮点击
			FLOAT btnX = px + pw - 120, btnY = py + ph - 45, btnW = 110, btnH = 35;
			if (x >= btnX && x <= btnX + btnW && y >= btnY && y <= btnY + btnH) {
				_flowgraph_editpanel_submit(hObj);
				return; // 拦截按钮点击
			}

			// ★ 修复：点击编辑框区域时，手动赋予焦点，确保光标定位和文本选择正常工作
			FLOAT editX = px + 10, editY = py + 32, editW = pw - 20, editH = ph - 82;
			if (x >= editX && x <= editX + editW && y >= editY && y <= editY + editH) {
				Ex_ObjSetFocus(pData->editPanelEdit);
				return; // 不触发画布操作，让 Ex_ObjDefProc 将鼠标消息传递给编辑框
			}

			// 点击在面板的其他空白区域（如标题栏），拦截以防止穿透到下方节点
			return;
		}
	}

	FLOAT virtualX = (x + scrollX) / pData->zoom; FLOAT virtualY = (y + scrollY) / pData->zoom;
	// ★ 组合框下拉面板点击检测与外部关闭
	if (pData->comboExpandedNode != -1) {
		EX_FLOWGRAPH_NODE* expNode = _flowgraph_findnode(pData, pData->comboExpandedNode);
		if (expNode && pData->comboExpandedPortIdx < expNode->portCount) {
			EX_FLOWGRAPH_PORT& expPort = expNode->ports[pData->comboExpandedPortIdx];
			if (expPort.widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO && expPort.widgetData) {
				EX_FLOWGRAPH_NODE_COMBO_DATA* comboData = (EX_FLOWGRAPH_NODE_COMBO_DATA*)expPort.widgetData;
				FLOAT wx = expNode->x + expPort.widgetRect.left;
				FLOAT wy = expNode->y + expPort.widgetRect.bottom;
				FLOAT ww = expPort.widgetRect.right - expPort.widgetRect.left;
				FLOAT itemH = 28.0f;
				FLOAT panelH = comboData->count * itemH;

				if (virtualX >= wx && virtualX <= wx + ww && virtualY >= wy && virtualY <= wy + panelH) {
					INT idx = (INT)((virtualY - wy) / itemH);
					if (idx >= 0 && idx < comboData->count && comboData->current != idx) {
						comboData->current = idx;
						Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_NODEDATA_COMBO_CHANGED, expNode->id, (LPARAM)&expPort);
						expNode->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
						_flowgraph_invalidate_downstream(pData, expNode->id);
					}
				}
			}
		}
		// 无论点击何处，收起面板并拦截本次点击
		pData->comboExpandedNode = -1;
		pData->comboExpandedPortIdx = -1;
		pData->comboPanelHoverIdx = -1;
		Ex_ObjInvalidateRect(hObj, 0);
		return;
	}
	// ★ 计算点击位置最顶层节点(解决重叠节点穿透点击问题)
	EX_FLOWGRAPH_NODE* topmostNode = _flowgraph_find_topmost_node_at(pData, virtualX, virtualY);
	// ===== 侧边栏点击检测 =====
	if (x >= 0 && x < FLOWGRAPH_SIDEBAR_WIDTH) {
		FLOAT btnYs[FLOWGRAPH_SIDEBAR_BTN_COUNT] = {
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y0,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y1,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y2,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y3,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y4,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y5,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y6,
			(FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y7
		};
		for (INT i = 0; i < FLOWGRAPH_SIDEBAR_BTN_COUNT; i++) {
			if (y >= btnYs[i] && y < btnYs[i] + FLOWGRAPH_SIDEBAR_BTN_H) {
				if (i == 4) {
					// 导出YAML
					WCHAR szFile[MAX_PATH] = { 0 };
					lstrcpyW(szFile, L"flowgraph_export.yaml");
					OPENFILENAMEW ofn = { 0 };
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = GetAncestor(Ex_ObjGetHWND(hObj), GA_ROOT);
					ofn.lpstrFile = szFile;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = L"YAML文件\0*.yaml;*.yml\0所有文件\0*.*\0";
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
					ofn.lpstrDefExt = L"yaml";
					if (GetSaveFileNameW(&ofn)) {
						Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_EXPORT_YAML, 0, (LPARAM)szFile);
						_flowgraph_export_to_yaml(hObj, szFile);
					}
				}
				else if (i == 5) {
					// 导入YAML
					WCHAR szFile[MAX_PATH] = { 0 };
					OPENFILENAMEW ofn = { 0 };
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = GetAncestor(Ex_ObjGetHWND(hObj), GA_ROOT);
					ofn.lpstrFile = szFile;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = L"YAML文件\0*.yaml;*.yml\0所有文件\0*.*\0";
					ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
					if (GetOpenFileNameW(&ofn)) {
						Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_IMPORT_YAML, 0, (LPARAM)szFile);
						_flowgraph_import_from_yaml(hObj, szFile);
					}
				}
				// i==0(添加), i==1(链路从头), i==2(链路继续) 由悬停处理面板显示
				return;
			}
		}
		return;
	}

	// ===== 添加面板点击检测 =====
	if (pData->sidebarShowAddPanel) {
		FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
		FLOAT panelY = FLOWGRAPH_SIDEBAR_BTN_Y0;
		FLOAT panelW = FLOWGRAPH_ADD_PANEL_WIDTH;
		FLOAT panelH = FLOWGRAPH_ADD_PANEL_HEADER + (pData->cardRegistryCount > 0 ? pData->cardRegistryCount * FLOWGRAPH_ADD_PANEL_ITEM_H + 4 : 36);

		if (x >= panelX && x < panelX + panelW && y >= panelY && y < panelY + panelH) {
			if (pData->cardRegistryCount > 0) {
				FLOAT itemStartY = panelY + FLOWGRAPH_ADD_PANEL_HEADER;
				FLOAT itemH = FLOWGRAPH_ADD_PANEL_ITEM_H;
				INT idx = (INT)((y - itemStartY) / itemH);
				if (idx >= 0 && idx < pData->cardRegistryCount) {
					// ★ 核心修改：从注册表中动态获取真实的 cardType，而非简单的 idx + 1
					INT cardType = pData->cardRegistry[idx].cardType;
					Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_SIDEBAR_ADD_CARD, cardType, 0);
				}
			}
			pData->sidebarShowAddPanel = FALSE;
			pData->sidebarAddPanelHover = -1;
			Ex_ObjInvalidateRect(hObj, 0);
			return;
		}
		pData->sidebarShowAddPanel = FALSE;
		pData->sidebarAddPanelHover = -1;
		Ex_ObjInvalidateRect(hObj, 0);
	}

	// ★ ===== 链路节点选择面板点击检测 =====
	if (pData->sidebarShowChainPanel && pData->nodeCount > 0) {
		INT visibleCount = __min(pData->nodeCount, FLOWGRAPH_CHAIN_PANEL_MAX_VISIBLE);
		FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
		FLOAT panelY = (FLOAT)(pData->sidebarChainPanelMode == 0 ? FLOWGRAPH_SIDEBAR_BTN_Y1 : FLOWGRAPH_SIDEBAR_BTN_Y2);
		FLOAT panelW = FLOWGRAPH_CHAIN_PANEL_WIDTH;
		FLOAT panelH = FLOWGRAPH_CHAIN_PANEL_HEADER + visibleCount * FLOWGRAPH_CHAIN_PANEL_ITEM_H + 4;

		if (x >= panelX && x < panelX + panelW && y >= panelY && y < panelY + panelH) {
			FLOAT itemStartY = panelY + FLOWGRAPH_CHAIN_PANEL_HEADER;
			FLOAT itemH = FLOWGRAPH_CHAIN_PANEL_ITEM_H;
			INT idx = (INT)((y - itemStartY) / itemH);
			if (idx >= 0 && idx < visibleCount) {
				INT nodeId = pData->nodes[idx].id;
				INT mode = (pData->sidebarChainPanelMode == 0) ? FLOWGRAPH_EXECUTE_MODE_FRESH : FLOWGRAPH_EXECUTE_MODE_CONTINUE;
				Ex_ObjSendMessage(hObj, FLOWGRAPH_MESSAGE_EXECUTE_CHAIN, mode, nodeId);
			}
			pData->sidebarShowChainPanel = FALSE;
			pData->sidebarChainPanelHover = -1;
			Ex_ObjInvalidateRect(hObj, 0);
			return;
		}
		pData->sidebarShowChainPanel = FALSE;
		pData->sidebarChainPanelHover = -1;
		Ex_ObjInvalidateRect(hObj, 0);
	}

	if (pData->sidebarShowCancelPanel) {
		INT busyNodes[1024]; INT busyCount = 0;
		for (INT i = 0; i < pData->nodeCount && busyCount < 1024; i++) {
			if (pData->nodes[i].executionStatus == FLOWGRAPH_EXEC_STATUS_RUNNING || pData->nodes[i].executionStatus == FLOWGRAPH_EXEC_STATUS_PENDING) {
				busyNodes[busyCount++] = i;
			}
		}
		INT visibleCount = __min(busyCount, FLOWGRAPH_CANCEL_PANEL_MAX_VISIBLE);
		FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
		FLOAT panelY = (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y3; // ★ Y3
		FLOAT panelW = FLOWGRAPH_CANCEL_PANEL_WIDTH;
		FLOAT panelH = FLOWGRAPH_CANCEL_PANEL_HEADER + visibleCount * FLOWGRAPH_CANCEL_PANEL_ITEM_H + 4;

		if (x >= panelX && x < panelX + panelW && y >= panelY && y < panelY + panelH) {
			FLOAT itemStartY = panelY + FLOWGRAPH_CANCEL_PANEL_HEADER;
			FLOAT itemH = FLOWGRAPH_CANCEL_PANEL_ITEM_H;
			INT idx = (INT)((y - itemStartY) / itemH);
			if (idx >= 0 && idx < visibleCount) {
				INT targetNodeId = pData->nodes[busyNodes[idx]].id;
				_flowgraph_cancel_node(hObj, targetNodeId); // ★ 执行取消
			}
			pData->sidebarShowCancelPanel = FALSE;
			pData->sidebarCancelPanelHover = -1;
			Ex_ObjInvalidateRect(hObj, 0);
			return;
		}
		pData->sidebarShowCancelPanel = FALSE;
		pData->sidebarCancelPanelHover = -1;
		Ex_ObjInvalidateRect(hObj, 0);
	}
	// ★ ===== 自定义面板点击检测 =====
	if (pData->sidebarShowCustomPanel) {
		INT visibleCount = __min(pData->customItemCount, FLOWGRAPH_CUSTOM_PANEL_MAX_VISIBLE);
		FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
		FLOAT panelY = (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y6;
		FLOAT panelW = FLOWGRAPH_CUSTOM_PANEL_WIDTH;
		FLOAT panelH = FLOWGRAPH_CUSTOM_PANEL_HEADER + (pData->customItemCount > 0 ? visibleCount * FLOWGRAPH_CUSTOM_PANEL_ITEM_H + 4 : 36);

		if (x >= panelX && x < panelX + panelW && y >= panelY && y < panelY + panelH) {
			if (pData->customItemCount > 0) {
				FLOAT itemStartY = panelY + FLOWGRAPH_CUSTOM_PANEL_HEADER;
				FLOAT itemH = FLOWGRAPH_CUSTOM_PANEL_ITEM_H;
				INT idx = (INT)((y - itemStartY) / itemH);
				if (idx >= 0 && idx < visibleCount) {
					// ★ 派发自定义条目点击事件
					Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_CUSTOM_ITEM_CLICKED, idx, (LPARAM)pData->customItems[idx]);
				}
			}
			pData->sidebarShowCustomPanel = FALSE;
			pData->sidebarCustomPanelHover = -1;
			Ex_ObjInvalidateRect(hObj, 0);
			return;
		}
		pData->sidebarShowCustomPanel = FALSE;
		pData->sidebarCustomPanelHover = -1;
		Ex_ObjInvalidateRect(hObj, 0);
	}
	// ===== 视频控件点击检测 =====
	{
		for (INT i = pData->nodeCount - 1; i >= 0; i--) {
			EX_FLOWGRAPH_NODE* node = &pData->nodes[i];
			if (topmostNode && node != topmostNode) continue;
			for (INT j = 0; j < node->portCount; j++) {
				EX_FLOWGRAPH_PORT& port = node->ports[j];
				if (port.widgetType != FLOWGRAPH_NODEDATA_TYPE_VIDEO || !port.widgetData) continue;
				if (virtualX < node->x + port.widgetRect.left || virtualX > node->x + port.widgetRect.right ||
					virtualY < node->y + port.widgetRect.top || virtualY > node->y + port.widgetRect.bottom)
					continue;

				EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)port.widgetData;
				FLOAT ctrlY_local = node->y + port.widgetRect.bottom - FLOWGRAPH_VIDEO_CTRL_HEIGHT;
				FLOAT progressH = FLOWGRAPH_VIDEO_PROGRESS_H;
				FLOAT widgetLeft = node->x + port.widgetRect.left;
				FLOAT widgetWidth = port.widgetRect.right - port.widgetRect.left;

				// 进度条点击/拖拽
				if (virtualY >= ctrlY_local && virtualY < ctrlY_local + progressH + 4 && pVideo->mediaPlayer) {
					pData->videoDragNode = node->id;
					pData->videoDragPortIdx = j;
					FLOAT pos = (virtualX - widgetLeft) / widgetWidth;
					pos = __max(0.0f, __min(1.0f, pos));
					if (pVideo->nDuration > 0) {
						pVideo->nCurrentTime = (INT64)(pos * pVideo->nDuration);
						libvlc_media_player_set_time(pVideo->mediaPlayer, pVideo->nCurrentTime);
					}
					Ex_ObjInvalidateRect(hObj, 0);
					return;
				}
				// 播放/暂停按钮
				FLOAT btnY = ctrlY_local + progressH + (FLOWGRAPH_VIDEO_CTRL_HEIGHT - progressH) / 2.0f;
				FLOAT playX = widgetLeft + 14.0f;
				if (virtualY >= btnY - 8 && virtualY <= btnY + 8 && virtualX >= playX - 10 && virtualX <= playX + 10 && pVideo->mediaPlayer) {
					if (pVideo->bIsPlaying) {
						libvlc_media_player_set_pause(pVideo->mediaPlayer, 1);
						pVideo->bIsPaused = TRUE; pVideo->bIsPlaying = FALSE;
					}
					else if (pVideo->bIsPaused) {
						libvlc_media_player_set_pause(pVideo->mediaPlayer, 0);
						pVideo->bIsPaused = FALSE; pVideo->bIsPlaying = TRUE;
						_flowgraph_update_video_timers(hObj, pData);
					}
					else if (pVideo->bIsLoaded) {
						libvlc_media_player_play(pVideo->mediaPlayer);
						pVideo->bIsPlaying = TRUE;
						_flowgraph_update_video_timers(hObj, pData);
					}
					Ex_ObjInvalidateRect(hObj, 0);
					return;
				}

				break; // 命中视频区域，不再检测其他
			}
		}
	}
	pData->selectedNode = -1; 
	pData->selectedPortNode = -1; 
	pData->selectedPortIndex = -1;
	// ★ 更新浮动编辑面板
	_flowgraph_update_edit_panel(hObj);
	BOOL shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
	if (shiftPressed) {
		for (INT i = pData->nodeCount - 1; i >= 0; i--) {
			EX_FLOWGRAPH_NODE* node = &pData->nodes[i];
			if (topmostNode && node != topmostNode) continue;
			for (INT j = 0; j < node->portCount; j++) {
				if (node->ports[j].portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) continue;
				RECT rc = node->ports[j].portRect;
				if (virtualX >= node->x + rc.left && virtualX <= node->x + rc.right && virtualY >= node->y + rc.top && virtualY <= node->y + rc.bottom) {
					pData->selectedPortNode = node->id; pData->selectedPortIndex = j; pData->selectedConnection = -1;
					pData->connectingSlot = j; pData->connectingNode = node->id;
					pData->connectingSlotType = node->ports[j].portType == FLOWGRAPH_PORTTYPE_OUTPUT ? FLOWGRAPH_SLOTTYPE_OUTPUT : FLOWGRAPH_SLOTTYPE_INPUT;
					Ex_ObjInvalidateRect(hObj, 0); return;
				}
			}
		}
	}
	// 1. 组件调整大小手柄
	for (INT i = pData->nodeCount - 1; i >= 0; i--) {
		EX_FLOWGRAPH_NODE* node = &pData->nodes[i];
		if (topmostNode && node != topmostNode) continue;
		for (INT j = 0; j < node->portCount; j++) {
			auto& port = node->ports[j];
			BOOL showWidget = (port.portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) || (port.portType == FLOWGRAPH_PORTTYPE_INPUT && !port.isConnected);
			if (port.widgetType != 0 && showWidget && port.widgetType != FLOWGRAPH_NODEDATA_TYPE_BUTTON) {
				FLOAT right = node->x + port.widgetRect.right; FLOAT bottom = node->y + port.widgetRect.bottom;
				if (virtualX >= right - 8 && virtualX <= right + 2 && virtualY >= bottom - 10 && virtualY <= bottom + 2) {
					pData->resizingNode = node->id; pData->resizingPortIdx = j;
					pData->dragStartPos = { virtualX, virtualY }; Ex_ObjInvalidateRect(hObj, 0); return;
				}
			}
		}
	}
	// 2. 控制点拖拽
	for (INT i = 0; i < pData->connectionCount; i++) {
		EX_FLOWGRAPH_CONNECTION* conn = &pData->connections[i];
		FLOAT distCtrl1 = sqrtf(powf(virtualX - conn->controlPoint1.x, 2) + powf(virtualY - conn->controlPoint1.y, 2));
		FLOAT distCtrl2 = sqrtf(powf(virtualX - conn->controlPoint2.x, 2) + powf(virtualY - conn->controlPoint2.y, 2));
		if (distCtrl1 <= 10.0f || distCtrl2 <= 10.0f) {
			pData->selectedConnection = conn->id; pData->draggingControlPoint = TRUE;
			pData->dragStartPos = { virtualX, virtualY }; pData->draggingWhichPoint = distCtrl1 <= 10.0f ? 1 : 2;
			Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_CONNECTION_SELECTED, conn->id, 0); Ex_ObjInvalidateRect(hObj, 0); return;
		}
	}
	// 3. 连线选中
	for (INT i = 0; i < pData->connectionCount; i++) {
		EX_FLOWGRAPH_CONNECTION* conn = &pData->connections[i];
		EX_FLOWGRAPH_NODE* fromNode = _flowgraph_findnode(pData, conn->fromNode); EX_FLOWGRAPH_NODE* toNode = _flowgraph_findnode(pData, conn->toNode);
		if (!fromNode || !toNode) continue;
		POINTF v1 = { fromNode->x + (fromNode->ports[conn->fromSlot].portRect.left + fromNode->ports[conn->fromSlot].portRect.right) / 2.0f, fromNode->y + (fromNode->ports[conn->fromSlot].portRect.top + fromNode->ports[conn->fromSlot].portRect.bottom) / 2.0f };
		POINTF v2 = conn->controlPoint1; POINTF v3 = conn->controlPoint2;
		POINTF v4 = { toNode->x + (toNode->ports[conn->toSlot].portRect.left + toNode->ports[conn->toSlot].portRect.right) / 2.0f, toNode->y + (toNode->ports[conn->toSlot].portRect.top + toNode->ports[conn->toSlot].portRect.bottom) / 2.0f };
		POINTF p = { virtualX, virtualY };
		if (_flowgraph_dist_to_segment(p, v1, v2) < 5.0f || _flowgraph_dist_to_segment(p, v2, v3) < 5.0f || _flowgraph_dist_to_segment(p, v3, v4) < 5.0f) {
			pData->selectedConnection = conn->id; Ex_ObjInvalidateRect(hObj, 0); return;
		}
	}
	// 4. 端口圆点
	for (INT i = pData->nodeCount - 1; i >= 0; i--) {
		EX_FLOWGRAPH_NODE* node = &pData->nodes[i];
		if (topmostNode && node != topmostNode) continue;
		for (INT j = 0; j < node->portCount; j++) {
			if (node->ports[j].portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) continue;
			RECT rc = node->ports[j].portRect;
			if (virtualX >= node->x + rc.left && virtualX <= node->x + rc.right && virtualY >= node->y + rc.top && virtualY <= node->y + rc.bottom) {
				pData->selectedPortNode = node->id; pData->selectedPortIndex = j; pData->selectedConnection = -1;
				pData->connectingSlot = j; pData->connectingNode = node->id;
				pData->connectingSlotType = node->ports[j].portType == FLOWGRAPH_PORTTYPE_OUTPUT ? FLOWGRAPH_SLOTTYPE_OUTPUT : FLOWGRAPH_SLOTTYPE_INPUT;
				Ex_ObjInvalidateRect(hObj, 0); return;
			}
		}
	}
	// 5. 按钮点击 + 组合框交互
	for (INT i = pData->nodeCount - 1; i >= 0; i--) {
		EX_FLOWGRAPH_NODE* node = &pData->nodes[i];
		if (topmostNode && node != topmostNode) continue;
		for (INT j = 0; j < node->portCount; j++) {
			EX_FLOWGRAPH_PORT& port = node->ports[j];
			BOOL showWidget = (port.portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) || (port.portType == FLOWGRAPH_PORTTYPE_INPUT && !port.isConnected);
			if (!showWidget || port.widgetType == 0) continue;
			if (virtualX < node->x + port.widgetRect.left || virtualX > node->x + port.widgetRect.right ||
				virtualY < node->y + port.widgetRect.top || virtualY > node->y + port.widgetRect.bottom)
				continue;
			// 双按钮点击
			if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON) {
				FLOAT gap = 6.0f;
				FLOAT widgetW = (FLOAT)(port.widgetRect.right - port.widgetRect.left);
				FLOAT halfW = (widgetW - gap) / 2.0f;
				FLOAT localX = virtualX - node->x - port.widgetRect.left;
				INT whichButton = 0;
				if (localX >= 0 && localX <= halfW) whichButton = FLOWGRAPH_DUAL_BUTTON_LEFT;
				else if (localX > halfW + gap && localX <= widgetW) whichButton = FLOWGRAPH_DUAL_BUTTON_RIGHT;

				// 第一组：参考图控制
				if (whichButton != 0 && port.widgetId == FLOWGRAPH_WIDGET_ID_REF_CONTROL) {
					if (whichButton == FLOWGRAPH_DUAL_BUTTON_LEFT) _flowgraph_add_dynamic_port(hObj, node->id);
					else _flowgraph_remove_dynamic_port(hObj, node->id);
					Ex_ObjInvalidateRect(hObj, 0); return;
				}
				// ★ 第二组：参考音频控制
				else if (whichButton != 0 && port.widgetId == FLOWGRAPH_WIDGET_ID_REF_AUDIO_CONTROL) {
					if (whichButton == FLOWGRAPH_DUAL_BUTTON_LEFT) _flowgraph_add_dynamic_port2(hObj, node->id);
					else _flowgraph_remove_dynamic_port2(hObj, node->id);
					Ex_ObjInvalidateRect(hObj, 0); return;
				}
				Ex_ObjInvalidateRect(hObj, 0); return;
			}
			// 按钮点击
			if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON) {
				EX_FLOWGRAPH_BUTTON_CLICK_INFO info = { node->id, node->cardType, j, port.id };
				Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_BUTTON_CLICKED, node->id, (LPARAM)&info);
				Ex_ObjInvalidateRect(hObj, 0); return;
			}
			// 组合框交互 (仅响应右侧下拉按钮)
			if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO && port.widgetData != NULL) {
				FLOAT btnWidth = 24.0f; // 虚拟坐标宽度
				FLOAT btnX = node->x + port.widgetRect.right - btnWidth;

				if (virtualX >= btnX && virtualX <= node->x + port.widgetRect.right &&
					virtualY >= node->y + port.widgetRect.top && virtualY <= node->y + port.widgetRect.bottom) {

					// 切换展开状态
					if (pData->comboExpandedNode == node->id && pData->comboExpandedPortIdx == j) {
						pData->comboExpandedNode = -1;
						pData->comboExpandedPortIdx = -1;
					}
					else {
						pData->comboExpandedNode = node->id;
						pData->comboExpandedPortIdx = j;
						pData->comboPanelHoverIdx = -1;
					}
					Ex_ObjInvalidateRect(hObj, 0);
					return;
				}
			}
		}
	}
	// 6. 节点拖拽 (支持多选组拖拽)
	if (topmostNode) {
		BOOL isNodeAlreadySelected = _flowgraph_is_node_selected(pData, topmostNode->id);
		BOOL ctrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
		BOOL shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

		if (isNodeAlreadySelected) {
			// ★ 情况 A：点击了已经选中的节点，准备拖拽整个选择组
			pData->isDraggingSelection = TRUE;
		}
		else {
			// ★ 情况 B：点击了未选中的节点
			if (!ctrlPressed && !shiftPressed) {
				// 没按修饰键，清空多选，改为单选该节点
				_flowgraph_clear_selection(pData);
			}
			// 将当前节点加入选择组
			_flowgraph_add_to_selection(pData, topmostNode->id);
			pData->isDraggingSelection = TRUE;
		}

		// 记录拖拽状态
		pData->draggingNode = topmostNode->id; // 记录主节点，兼容后续逻辑
		pData->dragStartPos = { virtualX, virtualY };
		pData->selectedConnection = -1;

		_flowgraph_update_edit_panel(hObj);
		Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_NODE_CLICKED, topmostNode->id, 0);
		Ex_ObjInvalidateRect(hObj, 0);
		return;
	}
	// ========================================================================================
	// ★ 核心新增：所有HitTest均失败，确认为点击空白区域，启动多选框选逻辑
	// ========================================================================================
	BOOL ctrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

	// 如果没有按住 Shift 或 Ctrl，则清空之前的多选状态
	if (!shiftPressed && !ctrlPressed) {
		_flowgraph_clear_selection(pData);
	}

	// 启动框选状态
	pData->isSelecting = TRUE;
	pData->selectStartPos = { virtualX, virtualY };
	pData->selectEndPos = { virtualX, virtualY };

	Ex_ObjInvalidateRect(hObj, 0);
}
FLOAT _flowgraph_dist_to_segment(POINTF p, POINTF v, POINTF w) {
	FLOAT l2 = (w.x - v.x) * (w.x - v.x) + (w.y - v.y) * (w.y - v.y);
	if (l2 == 0.0f) return sqrtf((p.x - v.x) * (p.x - v.x) + (p.y - v.y) * (p.y - v.y));
	float t = ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2;
	t = __max(0, __min(1, t));
	POINTF projection = { v.x + t * (w.x - v.x), v.y + t * (w.y - v.y) };
	return sqrtf((p.x - projection.x) * (p.x - projection.x) + (p.y - projection.y) * (p.y - projection.y));
}
// ==================== 鼠标左键释放 ====================
void _flowgraph_onlbuttonup(HEXOBJ hObj, INT x, INT y)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0); if (!pData) return;
	if (pData->videoDragNode != -1) {
		EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, pData->videoDragNode);
		if (node && pData->videoDragPortIdx < node->portCount) {
			EX_FLOWGRAPH_PORT& port = node->ports[pData->videoDragPortIdx];
			if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO && port.widgetData) {
				EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)port.widgetData;
				if (pVideo->mediaPlayer && pVideo->nDuration > 0) {
					libvlc_media_player_set_time(pVideo->mediaPlayer, pVideo->nCurrentTime);
				}
			}
		}
		pData->videoDragNode = -1;
		pData->videoDragPortIdx = -1;
		Ex_ObjInvalidateRect(hObj, 0);
		return;
	}
	if (pData->isSelecting) {
		pData->isSelecting = FALSE;
		_flowgraph_update_selection_rect(pData);
		Ex_ObjInvalidateRect(hObj, 0);
		return;
	}
	if (pData->resizingNode != -1) { pData->resizingNode = -1; pData->resizingPortIdx = -1; Ex_ObjInvalidateRect(hObj, 0); return; }
	if (pData->draggingControlPoint) {
		pData->draggingControlPoint = FALSE; pData->draggingWhichPoint = 0;
		if (pData->selectedConnection != -1) Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_CONNECTION_MOVED, pData->selectedConnection, 0);
	}
	if (pData->connectingSlot != -1 && pData->connectingNode != -1) {
		INT scrollX = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_HORZ); INT scrollY = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_VERT);
		FLOAT virtualX = (x + scrollX) / pData->zoom; FLOAT virtualY = (y + scrollY) / pData->zoom;
		for (INT i = 0; i < pData->nodeCount; i++) {
			EX_FLOWGRAPH_NODE* node = &pData->nodes[i]; if (node->id == pData->connectingNode) continue;
			for (INT j = 0; j < node->portCount; j++) {
				if (node->ports[j].portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) continue;
				RECT rc = node->ports[j].portRect;
				if (virtualX >= node->x + rc.left && virtualX <= node->x + rc.right && virtualY >= node->y + rc.top && virtualY <= node->y + rc.bottom) {
					EX_FLOWGRAPH_NODE* srcNode = _flowgraph_findnode(pData, pData->connectingNode);
					EX_FLOWGRAPH_PORT* srcPort = &srcNode->ports[pData->connectingSlot];
					EX_FLOWGRAPH_PORT* destPort = &node->ports[j];
					if (srcPort->portType != destPort->portType && (srcPort->dataType == destPort->dataType || destPort->dataType == FLOWGRAPH_DATATYPE_ANY || srcPort->dataType == FLOWGRAPH_DATATYPE_ANY)) {
						EX_FLOWGRAPH_CONNECTION conn; conn.id = (INT)GetTickCount64();
						if (srcPort->portType == FLOWGRAPH_PORTTYPE_OUTPUT) { conn.fromNode = srcNode->id; conn.fromSlot = pData->connectingSlot; conn.toNode = node->id; conn.toSlot = j; }
						else { conn.fromNode = node->id; conn.fromSlot = j; conn.toNode = srcNode->id; conn.toSlot = pData->connectingSlot; }
						_flowgraph_addconnection(hObj, &conn);
					}
					break;
				}
			}
		}
		pData->connectingSlot = -1; pData->connectingNode = -1;
	}
	pData->isDraggingSelection = FALSE;
	pData->draggingNode = -1; 
	Ex_ObjInvalidateRect(hObj, 0);
}

void _flowgraph_onmousewheel(HEXOBJ hObj, SHORT delta)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0); if (!pData) return;
	if (pData->comboExpandedNode != -1) {
		pData->comboExpandedNode = -1;
		pData->comboExpandedPortIdx = -1;
		pData->comboPanelHoverIdx = -1;
	}
	FLOAT newZoom = pData->zoom * (delta > 0 ? 1.1f : 0.9f);
	if (newZoom < 0.5f) newZoom = 0.5f; if (newZoom > 2.0f) newZoom = 2.0f;
	pData->zoom = newZoom; _flowgraph_updatelayout(hObj); Ex_ObjInvalidateRect(hObj, 0);
}
void _flowgraph_onscrollbar(HEXOBJ hObj, INT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bHScoll = uMsg == WM_HSCROLL;
	INT nMin, nMax; Ex_ObjScrollGetRange(hObj, bHScoll ? SCROLLBAR_TYPE_HORZ : SCROLLBAR_TYPE_VERT, &nMin, &nMax);
	if (nMax <= 1) return;
	INT nCode = LOWORD(wParam);
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (pData->comboExpandedNode != -1) {
		pData->comboExpandedNode = -1;
		pData->comboExpandedPortIdx = -1;
		pData->comboPanelHoverIdx = -1;
	}
	INT oPos = bHScoll ? pData->panOffset.x : pData->panOffset.y;
	INT nPos = 0, nPage = 1000, nLine = 200;
	if (nCode == SB_THUMBPOSITION) {
		HEXOBJ hSB = Ex_ObjScrollGetControl(hObj, bHScoll ? SCROLLBAR_TYPE_HORZ : SCROLLBAR_TYPE_VERT);
		nPos = Ex_ObjScrollGetTrackPos(hSB, SCROLLBAR_TYPE_CONTROL);
	}
	else {
		if (nCode == SB_PAGEUP) nPos = oPos - nPage;
		else if (nCode == SB_PAGEDOWN) nPos = oPos + nPage;
		else if (nCode == SB_LINEUP) nPos = oPos - nLine;
		else if (nCode == SB_LINEDOWN) nPos = oPos + nLine;
		else if (nCode == SB_TOP) nPos = 0;
		else return;
	}
	nPos = __max(nMin, __min(nMax, nPos));
	if (nPos != oPos) {
		if (bHScoll) { pData->panOffset.x = nPos; Ex_ObjScrollSetPos(Ex_ObjScrollGetControl(hObj, SCROLLBAR_TYPE_HORZ), SCROLLBAR_TYPE_CONTROL, nPos, TRUE); }
		else { pData->panOffset.y = nPos; Ex_ObjScrollSetPos(Ex_ObjScrollGetControl(hObj, SCROLLBAR_TYPE_VERT), SCROLLBAR_TYPE_CONTROL, nPos, TRUE); }
		Ex_ObjInvalidateRect(hObj, 0);
	}
}
// ==================== 添加节点 ====================
INT _flowgraph_addnode(HEXOBJ hObj, EX_FLOWGRAPH_NODE* pNode)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData || !pNode) return 0;
	_flowgraph_calcnodesize(hObj, pNode); INT newCount = pData->nodeCount + 1;
	EX_FLOWGRAPH_NODE* newNodes = (EX_FLOWGRAPH_NODE*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE) * newCount);
	if (pData->nodeCount > 0) { memcpy(newNodes, pData->nodes, sizeof(EX_FLOWGRAPH_NODE) * pData->nodeCount); Ex_MemFree(pData->nodes); }
	pData->nodes = newNodes;
	EX_FLOWGRAPH_NODE* newNode = &newNodes[pData->nodeCount];

	// ★ 自动生成节点ID
	pData->nextAutoId++;
	newNode->id = pData->nextAutoId;
	newNode->cardType = pNode->cardType;
	newNode->width = pNode->width; newNode->height = pNode->height;
	newNode->dynamicPortCount = pNode->dynamicPortCount;
	newNode->dynamicPortCount2 = pNode->dynamicPortCount2;
	// ★ 自动定位：如果x,y为AUTO_POSITION，定位到视口中心
	if (pNode->x == FLOWGRAPH_AUTO_POSITION && pNode->y == FLOWGRAPH_AUTO_POSITION) {
		RECT rcClient; Ex_ObjGetClientRect(hObj, &rcClient);
		INT scrollX = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_HORZ);
		INT scrollY = Ex_ObjScrollGetPos(hObj, SCROLLBAR_TYPE_VERT);
		newNode->x = (rcClient.right - rcClient.left) / 2.0f / pData->zoom
			+ scrollX / pData->zoom - newNode->width / 2.0f;
		newNode->y = (rcClient.bottom - rcClient.top) / 2.0f / pData->zoom
			+ scrollY / pData->zoom - newNode->height / 2.0f;
		if (newNode->x < 0) newNode->x = 50.0f;
		if (newNode->y < 0) newNode->y = 50.0f;
	}
	else {
		newNode->x = pNode->x;
		newNode->y = pNode->y;
	}

	// ★ 自动避让重叠
	_flowgraph_avoid_overlap(pData, newNode);
	newNode->title = StrDupW(pNode->title); newNode->portCount = pNode->portCount;
	newNode->lastExecutedPass = 0;
	newNode->ports = (EX_FLOWGRAPH_PORT*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_PORT) * pNode->portCount);
	newNode->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;  // ★ 初始化执行状态
	for (INT i = 0; i < pNode->portCount; i++) {
		EX_FLOWGRAPH_PORT* src = &pNode->ports[i]; EX_FLOWGRAPH_PORT* dest = &newNode->ports[i];
		memcpy(dest, src, sizeof(EX_FLOWGRAPH_PORT)); dest->name = StrDupW(src->name); dest->isConnected = FALSE;
		if (src->imagePath) dest->imagePath = StrDupW(src->imagePath);
		else dest->imagePath = NULL;
		// ★ 自动生成端口ID
		pData->nextAutoId++;
		dest->id = pData->nextAutoId;
		 if (src->widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO) {
			 // ★ 完善：创建新的视频控件数据，并拷贝本地路径
			 EX_FLOWGRAPH_NODE_VIDEO_DATA* srcVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)src->widgetData;
			 EX_FLOWGRAPH_NODE_VIDEO_DATA* destVideo = _flowgraph_create_video_data(pData->libVlc, hObj);
			 if (srcVideo && srcVideo->videoPath) {
				 destVideo->videoPath = StrDupW(srcVideo->videoPath);
				 // 可选：自动加载封面 (取消注释则粘贴后自动显示封面)
				 // _flowgraph_video_load(destVideo, destVideo->videoPath, TRUE); 
			 }
			 dest->widgetData = (LPVOID)destVideo;
		 }
		 else if (src->widgetData) {
			if (src->widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT) {
				dest->widgetData = (LPVOID)StrDupW((LPCWSTR)src->widgetData);
			}
			else if (src->widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT) {
				dest->widgetData = (LPVOID)StrDupW((LPCWSTR)src->widgetData);
			}
			else if (src->widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO) {
				EX_FLOWGRAPH_NODE_COMBO_DATA* srcCombo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)src->widgetData;
				EX_FLOWGRAPH_NODE_COMBO_DATA* destCombo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE_COMBO_DATA));
				destCombo->count = srcCombo->count; destCombo->current = srcCombo->current;
				destCombo->options = (LPCWSTR*)Ex_MemAlloc(sizeof(LPCWSTR) * srcCombo->count);
				for (INT k = 0; k < srcCombo->count; k++) destCombo->options[k] = StrDupW(srcCombo->options[k]);
				dest->widgetData = destCombo;
			}
			else if (src->widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON) {
				EX_FLOWGRAPH_NODE_BUTTON_DATA* srcBtn = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)src->widgetData;
				EX_FLOWGRAPH_NODE_BUTTON_DATA* destBtn = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE_BUTTON_DATA));
				destBtn->caption = StrDupW(srcBtn->caption);
				dest->widgetData = destBtn;
			}
			else if (src->widgetType == FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON) {
				EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* srcDual = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)src->widgetData;
				EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* destDual = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA));
				destDual->caption1 = StrDupW(srcDual->caption1);
				destDual->caption2 = StrDupW(srcDual->caption2);
				dest->widgetData = destDual;
			}
			else if (src->widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE) {
				HEXIMAGE hDst = NULL;
				if (_img_copy((HEXIMAGE)src->widgetData, &hDst)) {
					dest->widgetData = (LPVOID)hDst;
				}
				else {
					dest->widgetData = NULL;
				}
			}
			else { dest->widgetData = src->widgetData; }
		}
	}
	pData->nodeCount++; 
	_flowgraph_updatelayout(hObj); 
	Ex_ObjInvalidateRect(hObj, 0);
	return newNode->id;
}
INT _flowgraph_removenode(HEXOBJ hObj, INT nodeId)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (_flowgraph_is_node_protected_by_busy(pData, nodeId)) {
		Ex_MessageBox(hObj, L"忙碌节点所处链路无法删除", L"警告", MB_ICONWARNING, MESSAGEBOX_FLAG_CENTEWINDOW);
		return 1; // ★ 拦截
	}
	if (pData->contextMenuNodeId == nodeId) {
		pData->showContextMenu = FALSE;
	}
	// ★ 如果正在编辑该节点，隐藏面板
	if (pData->editPanelTargetNode == nodeId) {
		pData->showEditPanel = FALSE;
		pData->editPanelTargetNode = -1;
		Ex_ObjShow(pData->editPanelEdit, FALSE);
	}
	if (!pData || pData->nodeCount == 0) return 0;

	INT index = -1;
	for (INT i = 0; i < pData->nodeCount; i++) { if (pData->nodes[i].id == nodeId) { index = i; break; } }
	if (index == -1) return 0;
	for (INT i = pData->connectionCount - 1; i >= 0; i--) {
		if (pData->connections[i].fromNode == nodeId || pData->connections[i].toNode == nodeId)
			_flowgraph_removeconnection(hObj, pData->connections[i].id);
	}
	EX_FLOWGRAPH_NODE* node = &pData->nodes[index];
	Ex_MemFree((void*)node->title);
	for (INT j = 0; j < node->portCount; j++) {
		Ex_MemFree((void*)node->ports[j].name);
		if (node->ports[j].widgetData) {
			if (node->ports[j].widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT) Ex_MemFree(node->ports[j].widgetData);
			else if (node->ports[j].widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT) Ex_MemFree(node->ports[j].widgetData);
			else if (node->ports[j].widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO) {
				EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)node->ports[j].widgetData;
				for (INT k = 0; k < combo->count; k++) Ex_MemFree((void*)combo->options[k]);
				Ex_MemFree(combo->options); Ex_MemFree(combo);
			}
			else if (node->ports[j].widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE) 
			{ 
				_img_destroy((HEXIMAGE)node->ports[j].widgetData);
				if (node->ports[j].imagePath) Ex_MemFree((void*)node->ports[j].imagePath);
			}
			else if (node->ports[j].widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON) {
				EX_FLOWGRAPH_NODE_BUTTON_DATA* btn = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)node->ports[j].widgetData;
				Ex_MemFree((void*)btn->caption); Ex_MemFree(btn);
			}
			else if (node->ports[j].widgetType == FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON) {
				EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* dualBtn = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)node->ports[j].widgetData;
				Ex_MemFree((void*)dualBtn->caption1);
				Ex_MemFree((void*)dualBtn->caption2);
				Ex_MemFree(dualBtn);
			}
			else if (node->ports[j].widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO) {
				EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)node->ports[j].widgetData;
				if (pVideo) {
					_flowgraph_video_cleanup(pVideo);
					DeleteCriticalSection(&pVideo->critsec);
					Ex_MemFree(pVideo);
				}
			}
		}
	}
	Ex_MemFree(node->ports);
	if (pData->nodeCount > 1) {
		EX_FLOWGRAPH_NODE* newNodes = (EX_FLOWGRAPH_NODE*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE) * (pData->nodeCount - 1));
		if (index > 0) memcpy(newNodes, pData->nodes, sizeof(EX_FLOWGRAPH_NODE) * index);
		if (index < pData->nodeCount - 1) memcpy(newNodes + index, pData->nodes + index + 1, sizeof(EX_FLOWGRAPH_NODE) * (pData->nodeCount - index - 1));
		Ex_MemFree(pData->nodes); pData->nodes = newNodes;
	}
	else { Ex_MemFree(pData->nodes); pData->nodes = NULL; }
	pData->nodeCount--; _flowgraph_updatelayout(hObj); Ex_ObjInvalidateRect(hObj, 0);
	return 1;
}
INT _flowgraph_addconnection(HEXOBJ hObj, EX_FLOWGRAPH_CONNECTION* pConn)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0); if (!pData || !pConn) return 0;
	for (INT i = 0; i < pData->connectionCount; i++) { if (pData->connections[i].toNode == pConn->toNode && pData->connections[i].toSlot == pConn->toSlot) { _flowgraph_removeconnection(hObj, pData->connections[i].id); break; } }
	INT newCount = pData->connectionCount + 1; EX_FLOWGRAPH_CONNECTION* newConns = (EX_FLOWGRAPH_CONNECTION*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_CONNECTION) * newCount);
	if (pData->connectionCount > 0) { memcpy(newConns, pData->connections, sizeof(EX_FLOWGRAPH_CONNECTION) * pData->connectionCount); Ex_MemFree(pData->connections); }
	pData->connections = newConns; EX_FLOWGRAPH_CONNECTION* newConn = &newConns[pData->connectionCount];
	newConn->id = (pConn->id != 0) ? pConn->id : (INT)GetTickCount64() + pData->connectionCount;
	newConn->fromNode = pConn->fromNode; newConn->fromSlot = pConn->fromSlot;
	newConn->toNode = pConn->toNode; newConn->toSlot = pConn->toSlot;
	EX_FLOWGRAPH_NODE* toNode = _flowgraph_findnode(pData, newConn->toNode);
	if (toNode && newConn->toSlot < toNode->portCount) {
		toNode->ports[newConn->toSlot].isConnected = TRUE;

		// ★ 新增：连接变更，失效下游状态
		toNode->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
		_flowgraph_invalidate_downstream(pData, toNode->id);

		_flowgraph_calcnodesize(hObj, toNode);
	}
	pData->connectionCount++;
	EX_FLOWGRAPH_NODE* fromNode = _flowgraph_findnode(pData, newConn->fromNode);
	if (fromNode && toNode) {
		POINTF fromPt = { fromNode->x + (fromNode->ports[newConn->fromSlot].portRect.left + fromNode->ports[newConn->fromSlot].portRect.right) / 2.0f, fromNode->y + (fromNode->ports[newConn->fromSlot].portRect.top + fromNode->ports[newConn->fromSlot].portRect.bottom) / 2.0f };
		POINTF toPt = { toNode->x + (toNode->ports[newConn->toSlot].portRect.left + toNode->ports[newConn->toSlot].portRect.right) / 2.0f, toNode->y + (toNode->ports[newConn->toSlot].portRect.top + toNode->ports[newConn->toSlot].portRect.bottom) / 2.0f };
		_flowgraph_initcontrolpoint(newConn, fromPt, toPt);
	}
	Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_CONNECTION_CREATED, newConn->id, 0); Ex_ObjInvalidateRect(hObj, 0);
	return 1;
}
void _flowgraph_initcontrolpoint(EX_FLOWGRAPH_CONNECTION* conn, POINTF virtualFromPt, POINTF virtualToPt) {
	FLOAT dx = virtualToPt.x - virtualFromPt.x; FLOAT dy = virtualToPt.y - virtualFromPt.y;
	conn->controlPoint1.x = virtualFromPt.x + dx * 0.25f; conn->controlPoint1.y = virtualFromPt.y + dy * 0.25f;
	conn->controlPoint2.x = virtualToPt.x - dx * 0.25f; conn->controlPoint2.y = virtualToPt.y - dy * 0.25f;
}
INT _flowgraph_removeconnection(HEXOBJ hObj, INT connId)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0); if (!pData || pData->connectionCount == 0) return 0;
	if (_flowgraph_is_connection_protected_by_busy(pData, connId)) {
		Ex_MessageBox(hObj, L"忙碌节点所处链路无法删除", L"警告", MB_ICONWARNING, MESSAGEBOX_FLAG_CENTEWINDOW);
		return 1; // ★ 拦截
	}
	INT index = -1; for (INT i = 0; i < pData->connectionCount; i++) { if (pData->connections[i].id == connId) { index = i; break; } }
	if (index == -1) return 0;
	EX_FLOWGRAPH_CONNECTION* conn = &pData->connections[index];
	EX_FLOWGRAPH_NODE* toNode = _flowgraph_findnode(pData, conn->toNode);
	if (toNode && conn->toSlot < toNode->portCount) {
		EX_FLOWGRAPH_PORT* dstPort = &toNode->ports[conn->toSlot];
		dstPort->isConnected = FALSE;
		// ★★★ 核心修复：去除卡片类型硬编码，改为基于“端口数据类型”的通用清理逻辑 ★★★
		// 规则：
		// 1. 媒体类数据（图片、音频）：占用内存大，断开连线即视为“移除素材”，自动清空释放内存。
		// 2. 文本类数据（STRING）：保留缓存。方便用户调整连线时不丢失辛苦编写的提示词/参数。
		if (dstPort->portType == FLOWGRAPH_PORTTYPE_INPUT)
		{
			if (dstPort->dataType == FLOWGRAPH_DATATYPE_IMAGE) {
				if (dstPort->widgetData) {
					_img_destroy((HEXIMAGE)dstPort->widgetData);
					dstPort->widgetData = NULL;
				}
				if (dstPort->imagePath) {
					Ex_MemFree((void*)dstPort->imagePath);
					dstPort->imagePath = NULL;
				}
			}
			else if (dstPort->dataType == FLOWGRAPH_DATATYPE_AUDIO) {
				if (dstPort->widgetData) {
					Ex_MemFree(dstPort->widgetData);
					dstPort->widgetData = NULL;
				}
			}
			// ★ STRING (文本) 类型不在此处清空，完美保留缓存数据，以便后续重连或继续执行
		}
		// ★ 修改：断开连线时保留输入端口的缓存数据，不清空、不释放
		// 这样在后续执行链路时，节点会使用断开前保留的数据作为输入继续往下执行
		// 如果重新连上线，_flowgraph_copy_input_data 会自动释放旧数据并覆盖为新数据，无内存泄漏

		// 级联失效下游节点状态（状态依然需要失效，因为数据流已改变）
		toNode->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
		_flowgraph_invalidate_downstream(pData, toNode->id);

		_flowgraph_calcnodesize(hObj, toNode);
	}

	// ... 后面的原有删除逻辑不变 ...
	if (pData->connectionCount > 1) {
		EX_FLOWGRAPH_CONNECTION* newConns = (EX_FLOWGRAPH_CONNECTION*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_CONNECTION) * (pData->connectionCount - 1));
		if (index > 0) memcpy(newConns, pData->connections, sizeof(EX_FLOWGRAPH_CONNECTION) * index);
		if (index < pData->connectionCount - 1) memcpy(newConns + index, pData->connections + index + 1, sizeof(EX_FLOWGRAPH_CONNECTION) * (pData->connectionCount - index - 1));
		Ex_MemFree(pData->connections); pData->connections = newConns;
	}
	else { Ex_MemFree(pData->connections); pData->connections = NULL; }
	pData->connectionCount--; if (pData->selectedConnection == connId) pData->selectedConnection = -1;
	Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_CONNECTION_REMOVED, connId, 0); Ex_ObjInvalidateRect(hObj, 0);
	return 1;
}
EX_FLOWGRAPH_NODE* _flowgraph_findnode(EX_FLOWGRAPH_DATA* pData, INT nodeId) {
	for (INT i = 0; i < pData->nodeCount; i++) if (pData->nodes[i].id == nodeId) return &pData->nodes[i];
	return NULL;
}
EX_FLOWGRAPH_CONNECTION* _flowgraph_findconnection(EX_FLOWGRAPH_DATA* pData, INT connId) {
	for (INT i = 0; i < pData->connectionCount; i++) if (pData->connections[i].id == connId) return &pData->connections[i];
	return NULL;
}
void _flowgraph_calcnodesize(HEXOBJ hObj, EX_FLOWGRAPH_NODE* node) {
	FLOAT titleHeight = 30.0f, width = 200.0f, currentY = titleHeight;
	INT inputCount = 0, outputCount = 0;
	for (INT i = 0; i < node->portCount; i++) {
		if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_OUTPUT) outputCount++;
		else if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_INPUT) inputCount++;
	}

	// 1. 计算内部 Widget 的宽度
	for (INT i = 0; i < node->portCount; i++) {
		auto& port = node->ports[i];
		BOOL showWidget = (port.portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) || (port.portType == FLOWGRAPH_PORTTYPE_INPUT && !port.isConnected);
		if (port.widgetType != 0 && showWidget) {
			if (port.widgetWidth == 0) {
				if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON) port.widgetWidth = width - 20.0f;
				else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO) port.widgetWidth = 300.0f;
				else port.widgetWidth = 300.0f;
			}
			width = __max(width, port.widgetWidth + 20.0f);
		}
	}

	// 2. 计算内部 Widget 的高度和 Y 坐标
	for (INT i = 0; i < node->portCount; i++) {
		auto& port = node->ports[i];
		BOOL showWidget = (port.portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) || (port.portType == FLOWGRAPH_PORTTYPE_INPUT && !port.isConnected);
		if (port.widgetType != 0 && showWidget) {
			if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT || port.widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT) {
				if (port.widgetData && lstrlenW((LPCWSTR)port.widgetData) > 0) {
					LPCWSTR text = (LPCWSTR)port.widgetData;
					FLOAT textWidth = port.widgetWidth - 10.0f;
					if (textWidth < 10.0f) textWidth = 10.0f;
					HEXCANVAS hCanvasCalc = _canvas_createindependent(1, 1, 0);
					if (hCanvasCalc) {
						HEXFONT hFontCalc = _font_createfromfamily(L"Arial", port.widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT ? 14 : 12, 0);
						if (hFontCalc) {
							FLOAT calcW = 0, calcH = 0;
							_canvas_calctextsize(hCanvasCalc, hFontCalc, text, -1, DT_LEFT | DT_TOP | DT_WORDBREAK, textWidth, 9999.0f, &calcW, &calcH);
							port.widgetHeight = calcH + 10.0f;
							_font_destroy(hFontCalc);
						}
						else { port.widgetHeight = 30.0f; }
						_canvas_destroy(hCanvasCalc);
					}
					else { port.widgetHeight = 30.0f; }
				}
				else { port.widgetHeight = 30.0f; }
			}
			else {
				if (port.widgetHeight == 0) {
					if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE) port.widgetHeight = 180.0f;
					else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO) port.widgetHeight = 30.0f;
					else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON) port.widgetHeight = 30.0f;
					else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO) port.widgetHeight = 180.0f;
				}
			}
			port.widgetRect = { 10, (LONG)currentY, (LONG)(10 + port.widgetWidth), (LONG)(currentY + port.widgetHeight) };
			currentY += port.widgetHeight + 5.0f;
		}
		else { port.widgetRect = { 0,0,0,0 }; }
	}

	// 3. 确定节点基础宽高
	node->width = width;
	node->height = currentY + 10.0f;

	// 4. ★ 保证节点高度足够容纳左右端口 (防止端口溢出节点底部)
	FLOAT portSpacing = 36.0f;// ★ 加大圆点间距 (原 24.0f 改为 36.0f)
	FLOAT minPortHeight = __max(inputCount, outputCount) * portSpacing + titleHeight + 10.0f;
	if (node->height < minPortHeight) {
		node->height = minPortHeight;
	}

	// 5. ★ 重新计算 INPUT 和 OUTPUT 的 portRect，使其在左右两侧外部垂直居中
	FLOAT inputStartY = (node->height - inputCount * portSpacing) / 2.0f;
	if (inputStartY < titleHeight + 5.0f) inputStartY = titleHeight + 5.0f; // 避免与标题栏重叠

	FLOAT outputStartY = (node->height - outputCount * portSpacing) / 2.0f;
	if (outputStartY < titleHeight + 5.0f) outputStartY = titleHeight + 5.0f;

	INT inIdx = 0, outIdx = 0;
	for (INT i = 0; i < node->portCount; i++) {
		auto& port = node->ports[i];
		if (port.portType == FLOWGRAPH_PORTTYPE_INPUT) {
			FLOAT cy = inputStartY + inIdx * portSpacing + portSpacing / 2.0f;
			// ★ HitTest 区域放大(宽30，高34)，视觉圆心在 x = -15
			port.portRect = { -30, (LONG)(cy - 17), 0, (LONG)(cy + 17) };
			inIdx++;
		}
		else if (port.portType == FLOWGRAPH_PORTTYPE_OUTPUT) {
			FLOAT cy = outputStartY + outIdx * portSpacing + portSpacing / 2.0f;
			// ★ HitTest 区域放大(宽30，高34)，视觉圆心在 x = width + 15
			port.portRect = { (LONG)width, (LONG)(cy - 17), (LONG)(width + 30), (LONG)(cy + 17) };
			outIdx++;
		}
		else {
			port.portRect = { 0,0,0,0 };
		}
	}
}
// ==================== 从上游端口拷贝数据到当前节点输入端口 ====================
void _flowgraph_copy_input_data(EX_FLOWGRAPH_NODE* node, EX_FLOWGRAPH_DATA* pData)
{
	for (INT i = 0; i < pData->connectionCount; i++) {
		if (pData->connections[i].toNode != node->id) continue;
		INT fromSlot = pData->connections[i].fromSlot;
		INT toSlot = pData->connections[i].toSlot;
		EX_FLOWGRAPH_NODE* fromNode = _flowgraph_findnode(pData, pData->connections[i].fromNode);
		if (!fromNode || fromSlot >= fromNode->portCount || toSlot >= node->portCount) continue;
		EX_FLOWGRAPH_PORT* srcPort = &fromNode->ports[fromSlot];
		EX_FLOWGRAPH_PORT* dstPort = &node->ports[toSlot];
		LPVOID data = srcPort->widgetData;
		INT dataType = srcPort->dataType;
		// 释放旧数据
		if (dstPort->widgetData) {
			if (dstPort->dataType == FLOWGRAPH_DATATYPE_STRING) Ex_MemFree(dstPort->widgetData);
			else if (dstPort->dataType == FLOWGRAPH_DATATYPE_IMAGE) { 
				_img_destroy((HEXIMAGE)dstPort->widgetData); 
			if (dstPort->imagePath) { Ex_MemFree((void*)dstPort->imagePath); dstPort->imagePath = NULL; }
			}
		}
		// 拷贝新数据
		if (dataType == FLOWGRAPH_DATATYPE_STRING) {
			dstPort->widgetData = data ? (LPVOID)StrDupW((LPCWSTR)data) : NULL;
		}
		else if (dataType == FLOWGRAPH_DATATYPE_IMAGE) {
			HEXIMAGE hDst = NULL;
			if (data && _img_copy((HEXIMAGE)data, &hDst)) dstPort->widgetData = (LPVOID)hDst;
			else dstPort->widgetData = NULL;
			if (srcPort->imagePath) dstPort->imagePath = StrDupW(srcPort->imagePath); // (注：在_executenode中 srcPort 应替换为 node->ports[fromSlot])
			else dstPort->imagePath = NULL;
		}
		else {
			dstPort->widgetData = data;
		}
	}
}
// ==================== 按卡片类型执行 ====================
void _flowgraph_execute_by_cardtype(HEXOBJ hObj, EX_FLOWGRAPH_DATA* pData, EX_FLOWGRAPH_NODE* node)
{
	INT inputCount = 0, outputCount = 0;
	for (INT i = 0; i < node->portCount; i++) {
		if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_INPUT || node->ports[i].portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) inputCount++;
		if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_OUTPUT) outputCount++;
	}

	EX_FLOWGRAPH_NODE_IO_DATA* inputs = (EX_FLOWGRAPH_NODE_IO_DATA*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE_IO_DATA) * inputCount);
	EX_FLOWGRAPH_NODE_IO_DATA* outputs = (EX_FLOWGRAPH_NODE_IO_DATA*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE_IO_DATA) * outputCount);
	memset(outputs, 0, sizeof(EX_FLOWGRAPH_NODE_IO_DATA) * outputCount);

	INT idx = 0;
	for (INT i = 0; i < node->portCount; i++) {
		if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_INPUT || node->ports[i].portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) {
			inputs[idx].portId = node->ports[i].id;
			inputs[idx].dataType = node->ports[i].dataType;
			inputs[idx].data = node->ports[i].widgetData; // 包含EDIT文本、COMBO结构体等
			idx++;
		}
	}
	idx = 0;
	for (INT i = 0; i < node->portCount; i++) {
		if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_OUTPUT) {
			outputs[idx].portId = node->ports[i].id;
			outputs[idx].dataType = node->ports[i].dataType;
			idx++;
		}
	}

	EX_FLOWGRAPH_EXECUTE_PARAMS params = { 0 };
	params.nodeId = node->id;
	params.cardType = node->cardType;
	params.inputCount = inputCount;
	params.inputs = inputs;
	params.outputCount = outputCount;
	params.outputs = outputs;
	params.executionResult = FLOWGRAPH_EXEC_RESULT_SUCCESS;

	// 统一派发通用执行事件
	Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_EXECUTE_NODE, node->id, (LPARAM)&params);

	if (params.executionResult != FLOWGRAPH_EXEC_RESULT_SUCCESS) {
		node->executionStatus = FLOWGRAPH_EXEC_STATUS_FAILED;
		Ex_MemFree(inputs); Ex_MemFree(outputs);
		return;
	}

	if (pData->asyncPendingNode == node->id) {
		Ex_MemFree(inputs); Ex_MemFree(outputs);
		return;
	}

	// 同步执行完成，自动应用 outputs 数组中的数据到节点端口
	idx = 0;
	for (INT i = 0; i < node->portCount; i++) {
		if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_OUTPUT) {
			if (params.outputs[idx].data != NULL) {
				if (node->ports[i].widgetData) {
					if (node->ports[i].dataType == FLOWGRAPH_DATATYPE_STRING) Ex_MemFree(node->ports[i].widgetData);
					else if (node->ports[i].dataType == FLOWGRAPH_DATATYPE_IMAGE) _img_destroy((HEXIMAGE)node->ports[i].widgetData);
				}
				node->ports[i].widgetData = params.outputs[idx].data;
			}
			idx++;
		}
	}
	Ex_MemFree(inputs); Ex_MemFree(outputs);
}

// ==================== 拓扑排序辅助(DFS后序) ====================
void _flowgraph_topo_visit(EX_FLOWGRAPH_DATA* pData, INT nodeId, BOOL* visited, INT* order, INT* orderCount)
{
	INT nodeIdx = -1;
	for (INT i = 0; i < pData->nodeCount; i++) {
		if (pData->nodes[i].id == nodeId) { nodeIdx = i; break; }
	}
	if (nodeIdx == -1 || visited[nodeIdx]) return;
	visited[nodeIdx] = TRUE;
	// 先访问所有上游节点
	for (INT i = 0; i < pData->connectionCount; i++) {
		if (pData->connections[i].toNode == nodeId) {
			_flowgraph_topo_visit(pData, pData->connections[i].fromNode, visited, order, orderCount);
		}
	}
	// 上游都处理完后，加入当前节点(保证上游在前)
	order[*orderCount] = nodeId;
	(*orderCount)++;
}

// ==================== 保存链式执行上下文 ====================
void _flowgraph_save_chain_context(EX_FLOWGRAPH_DATA* pData, EX_FLOWGRAPH_CHAIN_CTX* ctx)
{
	// 查找是否已有相同终点节点的上下文，有则替换
	for (INT i = 0; i < pData->chainContextCount; i++) {
		if (pData->chainContexts[i].chainEndNode == ctx->chainEndNode) {
			// ★ 修复：ctx 就是指向本条目的指针，nodeOrder 是同一个指针，不能释放！
			// 只需更新可变字段 currentIndex 即可
			pData->chainContexts[i].currentIndex = ctx->currentIndex;
			return;
		}
	}
	// 新增条目（第一次保存，ctx 是栈变量，需要完整拷贝）
	INT newCount = pData->chainContextCount + 1;
	EX_FLOWGRAPH_CHAIN_CTX* newCtxs = (EX_FLOWGRAPH_CHAIN_CTX*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_CHAIN_CTX) * newCount);
	if (pData->chainContextCount > 0) {
		memcpy(newCtxs, pData->chainContexts, sizeof(EX_FLOWGRAPH_CHAIN_CTX) * pData->chainContextCount);
		Ex_MemFree(pData->chainContexts);
	}
	newCtxs[pData->chainContextCount] = *ctx;
	pData->chainContexts = newCtxs;
	pData->chainContextCount = newCount;
}

// ==================== 移除链式执行上下文 ====================
void _flowgraph_remove_chain_context(EX_FLOWGRAPH_DATA* pData, INT chainEndNode)
{
	INT index = -1;
	for (INT i = 0; i < pData->chainContextCount; i++) {
		if (pData->chainContexts[i].chainEndNode == chainEndNode) { index = i; break; }
	}
	if (index == -1) return;
	Ex_MemFree(pData->chainContexts[index].nodeOrder);
	if (pData->chainContextCount > 1) {
		EX_FLOWGRAPH_CHAIN_CTX* newCtxs = (EX_FLOWGRAPH_CHAIN_CTX*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_CHAIN_CTX) * (pData->chainContextCount - 1));
		if (index > 0) memcpy(newCtxs, pData->chainContexts, sizeof(EX_FLOWGRAPH_CHAIN_CTX) * index);
		if (index < pData->chainContextCount - 1) memcpy(newCtxs + index, pData->chainContexts + index + 1, sizeof(EX_FLOWGRAPH_CHAIN_CTX) * (pData->chainContextCount - index - 1));
		Ex_MemFree(pData->chainContexts);
		pData->chainContexts = newCtxs;
	}
	else { Ex_MemFree(pData->chainContexts); pData->chainContexts = NULL; }
	pData->chainContextCount--;
}

// ==================== 查找节点所属的链式上下文 ====================
EX_FLOWGRAPH_CHAIN_CTX* _flowgraph_find_chain_context(EX_FLOWGRAPH_DATA* pData, INT nodeId)
{
	for (INT i = 0; i < pData->chainContextCount; i++) {
		EX_FLOWGRAPH_CHAIN_CTX* ctx = &pData->chainContexts[i];
		for (INT j = ctx->currentIndex; j < ctx->nodeOrderCount; j++) {
			if (ctx->nodeOrder[j] == nodeId) return ctx;
		}
	}
	return NULL;
}

// ==================== 迭代式链路执行步骤 ====================
void _flowgraph_execute_chain_step(HEXOBJ hObj, EX_FLOWGRAPH_DATA* pData, EX_FLOWGRAPH_CHAIN_CTX* ctx)
{
	while (ctx->currentIndex < ctx->nodeOrderCount) {
		INT nodeId = ctx->nodeOrder[ctx->currentIndex];
		EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, nodeId);
		if (!node || node->lastExecutedPass == ctx->executionPass) {
			ctx->currentIndex++;
			continue;
		}

		// ★ 继续执行模式下，跳过已完成的节点
		if (ctx->executeMode == FLOWGRAPH_EXECUTE_MODE_CONTINUE && node->executionStatus == FLOWGRAPH_EXEC_STATUS_COMPLETED) {
			ctx->currentIndex++;
			continue;
		}

		// 从上游拷贝输入数据
		_flowgraph_copy_input_data(node, pData);

		// 重置异步标记
		pData->asyncPendingNode = -1;

		// 标记已执行
		node->lastExecutedPass = ctx->executionPass;

		// 设置为执行中
		node->executionStatus = FLOWGRAPH_EXEC_STATUS_RUNNING;
		Ex_ObjInvalidateRect(hObj, 0);

		// 按卡片类型执行
		_flowgraph_execute_by_cardtype(hObj, pData, node);

		// 失败检查（优先于异步检查）
		if (node->executionStatus == FLOWGRAPH_EXEC_STATUS_FAILED) {
			INT endNode = ctx->chainEndNode;
			_flowgraph_remove_chain_context(pData, endNode);
			Ex_ObjInvalidateRect(hObj, 0);
			return;
		}

		// 检查是否为异步节点
		if (pData->asyncPendingNode == nodeId) {
			_flowgraph_save_chain_context(pData, ctx);
			return;
		}

		// 同步执行完成
		node->executionStatus = FLOWGRAPH_EXEC_STATUS_COMPLETED;
		ctx->currentIndex++;
	}

	// 链路执行完毕
	INT endNode = ctx->chainEndNode;
	_flowgraph_remove_chain_context(pData, endNode);
	Ex_ObjInvalidateRect(hObj, 0);
}

// ==================== 链式执行(迭代+异步支持) ====================
void _flowgraph_execute_chain(HEXOBJ hObj, INT mode, INT nodeId)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData) return;

	// ★ 新增：定义 targetNode 并拦截重复执行
	EX_FLOWGRAPH_NODE* targetNode = _flowgraph_findnode(pData, nodeId);
	if (targetNode && (targetNode->executionStatus == FLOWGRAPH_EXEC_STATUS_RUNNING || targetNode->executionStatus == FLOWGRAPH_EXEC_STATUS_PENDING)) {
		return; // ★ 禁止重复执行正在忙碌的节点
	}

	BOOL* visited = (BOOL*)Ex_MemAlloc(sizeof(BOOL) * pData->nodeCount);
	memset(visited, 0, sizeof(BOOL) * pData->nodeCount);
	INT* order = (INT*)Ex_MemAlloc(sizeof(INT) * pData->nodeCount);
	INT orderCount = 0;
	_flowgraph_topo_visit(pData, nodeId, visited, order, &orderCount);
	Ex_MemFree(visited);
	if (orderCount == 0) { Ex_MemFree(order); return; }

	pData->executionPass++;
	// ★ 仅在从头执行模式下，将链路中所有未执行节点设置为等待执行状态
	if (mode == FLOWGRAPH_EXECUTE_MODE_FRESH) {
		for (INT i = 0; i < orderCount; i++) {
			EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, order[i]);
			if (node && node->lastExecutedPass != pData->executionPass) {
				node->executionStatus = FLOWGRAPH_EXEC_STATUS_PENDING;
			}
		}
	}

	EX_FLOWGRAPH_CHAIN_CTX ctx = { 0 };
	ctx.chainEndNode = nodeId;
	ctx.nodeOrder = order;
	ctx.nodeOrderCount = orderCount;
	ctx.currentIndex = 0;
	ctx.executionPass = pData->executionPass;
	ctx.executeMode = mode;  // ★ 保存执行模式
	_flowgraph_execute_chain_step(hObj, pData, &ctx);

	BOOL wasSaved = FALSE;
	for (INT i = 0; i < pData->chainContextCount; i++) {
		if (pData->chainContexts[i].chainEndNode == nodeId) {
			wasSaved = TRUE;
			break;
		}
	}
	if (!wasSaved) {
		Ex_MemFree(order);
	}
	Ex_ObjInvalidateRect(hObj, 0);
}


// ==================== 异步执行完成消息处理 ====================
INT _flowgraph_node_execution_complete(HEXOBJ hObj, EX_FLOWGRAPH_ASYNC_RESULT* result)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData || !result) return 0;
	EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, result->nodeId);
	if (!node) return 0;
	// ★ 新增：如果节点已被强制取消（状态被重置为IDLE），直接丢弃异步结果
	if (node->executionStatus == FLOWGRAPH_EXEC_STATUS_IDLE) return 1;
	if (result->executionResult != FLOWGRAPH_EXEC_RESULT_SUCCESS) {
		node->executionStatus = FLOWGRAPH_EXEC_STATUS_FAILED;
	}
	else {
		// ★ 显式映射优先
		if (result->outputCount > 0 && result->outputs) {
			for (INT i = 0; i < result->outputCount; i++) {
				for (INT j = 0; j < node->portCount; j++) {
					if (node->ports[j].id == result->outputs[i].portId) {
						if (node->ports[j].widgetData) {
							if (node->ports[j].dataType == FLOWGRAPH_DATATYPE_STRING) Ex_MemFree(node->ports[j].widgetData);
							else if (node->ports[j].dataType == FLOWGRAPH_DATATYPE_IMAGE) { 
								_img_destroy((HEXIMAGE)node->ports[j].widgetData); 
								if (node->ports[j].imagePath) { Ex_MemFree((void*)node->ports[j].imagePath); node->ports[j].imagePath = NULL; }
							}
						}
						node->ports[j].widgetData = result->outputs[i].data;
						break;
					}
				}
			}
		}
		// ★ 智能自动映射 (兼容旧逻辑)
		else {
			// 1. 处理 outputText
			if (result->outputText) {
				for (INT i = 0; i < node->portCount; i++) {
					if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_OUTPUT && node->ports[i].dataType == FLOWGRAPH_DATATYPE_STRING) {
						if (node->ports[i].widgetData) Ex_MemFree(node->ports[i].widgetData);
						node->ports[i].widgetData = (LPVOID)StrDupW(result->outputText);
						break;
					}
				}
				// 检查是否有视频控件，如果有则自动加载
				for (INT i = 0; i < node->portCount; i++) {
					if (node->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO) {
						EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)node->ports[i].widgetData;
						if (!pVideo) {
							pVideo = _flowgraph_create_video_data(pData->libVlc, hObj);
							node->ports[i].widgetData = (LPVOID)pVideo;
						}
						_flowgraph_video_load(pVideo, result->outputText, TRUE);
						break;
					}
				}
			}
			// 2. 处理 outputImage
			if (result->outputImage) {
				for (INT i = 0; i < node->portCount; i++) {
					if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_OUTPUT && node->ports[i].dataType == FLOWGRAPH_DATATYPE_IMAGE) {
						if (node->ports[i].widgetData) _img_destroy((HEXIMAGE)node->ports[i].widgetData);
						node->ports[i].widgetData = (LPVOID)result->outputImage;
						break;
					}
				}
				// 自动复制一份到INTERMEDIATE/IMAGE预览端口
				HEXIMAGE hPreview = NULL;
				_img_copy(result->outputImage, &hPreview);
				if (hPreview) {
					for (INT i = 0; i < node->portCount; i++) {
						if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE && node->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE) {
							if (node->ports[i].widgetData) _img_destroy((HEXIMAGE)node->ports[i].widgetData);
							node->ports[i].widgetData = (LPVOID)hPreview;
							// ★★★ 新增：自适应预览框大小
							_flowgraph_adjust_image_size(&node->ports[i]);
							break;
						}
					}
				}
			}
		}
		node->executionStatus = FLOWGRAPH_EXEC_STATUS_COMPLETED;
		// ★ 新增：重新计算节点大小以适应图片
		_flowgraph_calcnodesize(hObj, node);
		_flowgraph_updatelayout(hObj);
	}

	pData->asyncPendingNode = -1;
	EX_FLOWGRAPH_CHAIN_CTX* ctx = _flowgraph_find_chain_context(pData, result->nodeId);
	if (!ctx) { Ex_ObjInvalidateRect(hObj, 0); return 1; }

	if (node->executionStatus == FLOWGRAPH_EXEC_STATUS_FAILED) {
		INT endNode = ctx->chainEndNode;
		_flowgraph_remove_chain_context(pData, endNode);
		Ex_ObjInvalidateRect(hObj, 0);
		return 1;
	}

	ctx->currentIndex++;
	_flowgraph_execute_chain_step(hObj, pData, ctx);
	return 1;
}

// ==================== 单节点执行(含上游自动执行) ====================
void _flowgraph_executenode(HEXOBJ hObj, INT mode, INT nodeId)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0); if (!pData) return;
	if (pData->executionDepth > 100) return;
	EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, nodeId);
	if (!node) return;

	// ★ 拦截重复执行
	if (node->executionStatus == FLOWGRAPH_EXEC_STATUS_RUNNING || node->executionStatus == FLOWGRAPH_EXEC_STATUS_PENDING) {
		return; // ★ 禁止重复执行正在忙碌的节点
	}

	// ★ 继续执行模式下，跳过已完成的节点
	if (mode == FLOWGRAPH_EXECUTE_MODE_CONTINUE && node->executionStatus == FLOWGRAPH_EXEC_STATUS_COMPLETED) {
		return;
	}

	// ★★★ 核心修复：如果是顶层入口调用，且为从头执行模式，必须递增 executionPass ★★★
	// 否则会因为 node->lastExecutedPass == pData->executionPass 而被拦截，导致无法重复执行
	BOOL isEntry = (pData->executionDepth == 0);
	if (isEntry && mode == FLOWGRAPH_EXECUTE_MODE_FRESH) {
		pData->executionPass++;
		// 重置当前节点状态，确保能够重新执行
		node->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
		// 级联失效下游状态（因为当前节点要重跑，下游数据也会变）
		_flowgraph_invalidate_downstream(pData, node->id);
	}

	if (node->lastExecutedPass == pData->executionPass) return;
	node->lastExecutedPass = pData->executionPass;
	node->executionStatus = FLOWGRAPH_EXEC_STATUS_RUNNING;
	Ex_ObjInvalidateRect(hObj, 0);
	pData->executionDepth++;

	// 先递归执行所有上游节点
	for (INT i = 0; i < pData->connectionCount; i++) {
		if (pData->connections[i].toNode == nodeId) {
			INT upstreamId = pData->connections[i].fromNode;
			EX_FLOWGRAPH_NODE* upNode = _flowgraph_findnode(pData, upstreamId);
			if (upNode && upNode->lastExecutedPass != pData->executionPass) {
				// ★ 修复：在 FRESH 模式下，强制重置上游节点状态为 IDLE，以便UI能正确显示 PENDING(黄) -> RUNNING(蓝)
				if (mode == FLOWGRAPH_EXECUTE_MODE_FRESH) {
					upNode->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
				}

				if (upNode->executionStatus == FLOWGRAPH_EXEC_STATUS_IDLE)
					upNode->executionStatus = FLOWGRAPH_EXEC_STATUS_PENDING;

				_flowgraph_executenode(hObj, mode, upstreamId);

				// ★ 上游失败则中断
				if (upNode->executionStatus == FLOWGRAPH_EXEC_STATUS_FAILED) {
					node->executionStatus = FLOWGRAPH_EXEC_STATUS_FAILED;
					pData->executionDepth--;
					Ex_ObjInvalidateRect(hObj, 0);
					return;
				}
			}
		}
	}

	// 从上游拷贝输入数据
	_flowgraph_copy_input_data(node, pData);
	// 按卡片类型执行
	_flowgraph_execute_by_cardtype(hObj, pData, node);

	// ★ 执行失败，不继续传播
	if (node->executionStatus == FLOWGRAPH_EXEC_STATUS_FAILED) {
		pData->executionDepth--;
		Ex_ObjInvalidateRect(hObj, 0);
		return;
	}

	if (pData->asyncPendingNode != nodeId) {
		node->executionStatus = FLOWGRAPH_EXEC_STATUS_COMPLETED;
	}

	// 将输出数据传播到下游
	for (INT i = 0; i < pData->connectionCount; i++) {
		if (pData->connections[i].fromNode != nodeId) continue;
		INT fromSlot = pData->connections[i].fromSlot;
		INT toNodeId = pData->connections[i].toNode;
		INT toSlot = pData->connections[i].toSlot;
		if (fromSlot >= node->portCount) continue;
		LPVOID data = node->ports[fromSlot].widgetData;
		INT dataType = node->ports[fromSlot].dataType;
		EX_FLOWGRAPH_NODE* toNode = _flowgraph_findnode(pData, toNodeId);
		if (!toNode || toSlot >= toNode->portCount) continue;
		EX_FLOWGRAPH_PORT* dstPort = &toNode->ports[toSlot];
		if (dstPort->widgetData) {
			if (dstPort->dataType == FLOWGRAPH_DATATYPE_STRING) Ex_MemFree(dstPort->widgetData);
			else if (dstPort->dataType == FLOWGRAPH_DATATYPE_IMAGE) {
				_img_destroy((HEXIMAGE)dstPort->widgetData);
				if (dstPort->imagePath) { Ex_MemFree((void*)dstPort->imagePath); dstPort->imagePath = NULL; }
			}
		}
		if (dataType == FLOWGRAPH_DATATYPE_STRING) {
			dstPort->widgetData = data ? (LPVOID)StrDupW((LPCWSTR)data) : NULL;
		}
		else if (dataType == FLOWGRAPH_DATATYPE_IMAGE) {
			HEXIMAGE hDst = NULL;
			if (data && _img_copy((HEXIMAGE)data, &hDst)) dstPort->widgetData = (LPVOID)hDst;
			else dstPort->widgetData = NULL;
			if (node->ports[fromSlot].imagePath) dstPort->imagePath = StrDupW(node->ports[fromSlot].imagePath);
			else dstPort->imagePath = NULL;
		}
		else {
			dstPort->widgetData = data;
		}
	}
	pData->executionDepth--;
	Ex_ObjInvalidateRect(hObj, 0);
}

void _flowgraph_execute_all(HEXOBJ hObj, INT mode)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData || pData->nodeCount == 0) return;

	// ★ 仅在从头执行模式下，重置所有节点执行状态
	if (mode == FLOWGRAPH_EXECUTE_MODE_FRESH) {
		for (INT i = 0; i < pData->nodeCount; i++) {
			pData->nodes[i].executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
		}
	}

	INT* hasDownstream = (INT*)Ex_MemAlloc(sizeof(INT) * pData->nodeCount);
	memset(hasDownstream, 0, sizeof(INT) * pData->nodeCount);

	for (INT i = 0; i < pData->connectionCount; i++) {
		INT fromId = pData->connections[i].fromNode;
		for (INT j = 0; j < pData->nodeCount; j++) {
			if (pData->nodes[j].id == fromId) { hasDownstream[j] = 1; break; }
		}
	}

	for (INT i = 0; i < pData->nodeCount; i++) {
		if (!hasDownstream[i]) {
			_flowgraph_execute_chain(hObj, mode, pData->nodes[i].id);
		}
	}

	Ex_MemFree(hasDownstream);
}

// ==================== 清空画布数据 ====================
void _flowgraph_clear_data(EX_FLOWGRAPH_DATA* pData) {
	if (!pData) return;
	// 释放节点资源
	for (INT i = 0; i < pData->nodeCount; i++) {
		EX_FLOWGRAPH_NODE& node = pData->nodes[i];
		Ex_MemFree((void*)node.title);
		for (INT j = 0; j < node.portCount; j++) {
			EX_FLOWGRAPH_PORT& port = node.ports[j];
			Ex_MemFree((void*)port.name);
			if (port.widgetData) {
				if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT) {
					Ex_MemFree(port.widgetData);
				}
				else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT) {
					Ex_MemFree(port.widgetData);
				}
				else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE) {
					_img_destroy((HEXIMAGE)port.widgetData);
					if (port.imagePath) Ex_MemFree((void*)port.imagePath);
				}
				else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO) {
					EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)port.widgetData;
					for (INT k = 0; k < combo->count; k++) {
						Ex_MemFree((void*)combo->options[k]);
					}
					Ex_MemFree(combo->options);
					Ex_MemFree(combo);
				}
				else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON) {
					EX_FLOWGRAPH_NODE_BUTTON_DATA* btn = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)port.widgetData;
					Ex_MemFree((void*)btn->caption);
					Ex_MemFree(btn);
				}
				else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON) {
					EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* dualBtn = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)port.widgetData;
					Ex_MemFree((void*)dualBtn->caption1);
					Ex_MemFree((void*)dualBtn->caption2);
					Ex_MemFree(dualBtn);
				}
				else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO) {
					EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)port.widgetData;
					if (pVideo) {
						_flowgraph_video_cleanup(pVideo);
						DeleteCriticalSection(&pVideo->critsec);
						Ex_MemFree(pVideo);
					}
				}
			}
		}
		Ex_MemFree(node.ports);
	}
	Ex_MemFree(pData->nodes);
	pData->nodes = NULL;
	pData->nodeCount = 0;
	// 释放连接线资源
	Ex_MemFree(pData->connections);
	pData->connections = NULL;
	pData->connectionCount = 0;
	// 重置状态
	pData->selectedNode = -1;
	pData->selectedConnection = -1;
	pData->selectedPortNode = -1;
	pData->selectedPortIndex = -1;
	pData->connectingNode = -1;
	pData->connectingSlot = -1;
	pData->draggingNode = -1;
	pData->hoverNode = -1;
	pData->hoverSlot = -1;
	pData->resizingNode = -1;
	pData->nextAutoId = 0;
	pData->hoverTitleNode = -1;  // ★ 重置标题悬停
	pData->sidebarShowCancelPanel = FALSE;
	pData->sidebarCancelPanelHover = -1;
	pData->showContextMenu = FALSE;
	pData->isSelecting = FALSE;
	pData->selectedNodes = NULL;
	pData->selectedNodeCount = 0;
	pData->clipboardNodes = NULL;
	pData->clipboardNodeCount = 0;
	pData->clipboardConnections = NULL;
	pData->clipboardConnectionCount = 0;
	pData->projectName[0] = L'\0'; // ★ 清空画布时重置工程名
	if (pData->selectedNodes) Ex_MemFree(pData->selectedNodes);
	// 释放剪贴板深拷贝数据
	if (pData->clipboardNodes) {
		for (INT i = 0; i < pData->clipboardNodeCount; i++) {
			_flowgraph_free_temp_node_data(&pData->clipboardNodes[i]);
		}
		Ex_MemFree(pData->clipboardNodes);
		pData->clipboardNodes = NULL; // ★★★ 修复：释放后置空 ★★★
	}
	pData->clipboardNodeCount = 0;
	if (pData->clipboardConnections) { 
		Ex_MemFree(pData->clipboardConnections); 
		pData->clipboardConnections = NULL;
	}
	pData->clipboardConnectionCount = 0;
	// ★ 修复：清空画布时隐藏编辑面板并重置状态，防止YAML导入时残留旧数据
	if (pData->showEditPanel) {
		pData->showEditPanel = FALSE;
		pData->editPanelTargetNode = -1;
		if (pData->editPanelEdit) Ex_ObjShow(pData->editPanelEdit, FALSE);
	}
}
// ==================== YAML导入时ID映射辅助 ====================
INT _flowgraph_map_yaml_id(INT oldId, INT* oldIds, INT* newIds, INT count) {
	for (INT i = 0; i < count; i++) {
		if (oldIds[i] == oldId) return newIds[i];
	}
	return -1;
}
// ==================== 导出YAML ====================
BOOL _flowgraph_export_to_yaml(HEXOBJ hObj, LPCWSTR filePath) {
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData) return FALSE;
	try {
		// ★★★ 核心修改：提取YAML所在目录及同名文件夹（去后缀） ★★★
		std::wstring yamlFullPath = filePath;
		size_t lastSlash = yamlFullPath.find_last_of(L"\\/");
		std::wstring yamlDir = (lastSlash != std::wstring::npos) ? yamlFullPath.substr(0, lastSlash + 1) : L"";
		std::wstring yamlFileName = (lastSlash != std::wstring::npos) ? yamlFullPath.substr(lastSlash + 1) : yamlFullPath;
		size_t dotPos = yamlFileName.find_last_of(L".");
		std::wstring baseName = (dotPos != std::wstring::npos) ? yamlFileName.substr(0, dotPos) : yamlFileName;
		std::wstring assetDir = yamlDir + baseName + L"\\";
		CreateDirectoryW(assetDir.c_str(), NULL); // 创建如 "我的工程\" 的同名文件夹
		ryml::Tree tree;
		ryml::NodeRef root = tree.rootref();
		root |= ryml::MAP;
		// 1. 导出视图设置
		ryml::NodeRef view = root["view"];
		view |= ryml::MAP;
		view["zoom"] << pData->zoom;
		view["pan_offset_x"] << pData->panOffset.x;
		view["pan_offset_y"] << pData->panOffset.y;
		// ★ 2. 导出卡片类型注册表
		ryml::NodeRef card_types = root["card_types"];
		card_types |= ryml::SEQ;
		for (INT i = 0; i < pData->cardRegistryCount; i++) {
			const EX_FLOWGRAPH_CARD_DESCRIPTOR& desc = pData->cardRegistry[i];
			ryml::NodeRef ct = card_types.append_child();
			ct |= ryml::MAP;
			ct["card_type"] << desc.cardType;
			ct["type_name"] << Ex_W2U(desc.typeName);
			ct["tag_color"] << (INT)desc.tagColor;
			ct["dynamic_port_base_index"] << desc.dynamicPortBaseIndex;
			ct["dynamic_port_min_count"] << desc.dynamicPortMinCount;
			ct["dynamic_port_max_count"] << desc.dynamicPortMaxCount;
			ct["dynamic_port_data_type"] << desc.dynamicPortDataType;
			if (desc.dynamicPortNamePrefix) ct["dynamic_port_name_prefix"] << Ex_W2U(desc.dynamicPortNamePrefix);
			else ct["dynamic_port_name_prefix"] << "";

			ct["dynamic_port2_base_index"] << desc.dynamicPort2BaseIndex;
			ct["dynamic_port2_min_count"] << desc.dynamicPort2MinCount;
			ct["dynamic_port2_max_count"] << desc.dynamicPort2MaxCount;
			ct["dynamic_port2_data_type"] << desc.dynamicPort2DataType;
			if (desc.dynamicPort2NamePrefix) ct["dynamic_port2_name_prefix"] << Ex_W2U(desc.dynamicPort2NamePrefix);
			else ct["dynamic_port2_name_prefix"] << "";

			ryml::NodeRef ports_yaml = ct["ports"];
			ports_yaml |= ryml::SEQ;
			for (INT j = 0; j < desc.portCount; j++) {
				const EX_FLOWGRAPH_PORT_DESC& port = desc.ports[j];
				ryml::NodeRef p = ports_yaml.append_child();
				p |= ryml::MAP;
				p["port_type"] << port.portType;
				p["data_type"] << port.dataType;
				p["name"] << Ex_W2U(port.name);
				p["widget_type"] << port.widgetType;
				p["widget_id"] << port.widgetId;
				p["widget_width"] << port.widgetWidth;
				p["widget_height"] << port.widgetHeight;
				// 默认数据暂略(因结构复杂)，内置卡片已硬编码注册，自定义卡片需外部提前注册
			}
		}
		// 3. 导出节点数据
		ryml::NodeRef nodes = root["nodes"];
		nodes |= ryml::SEQ;
		for (INT i = 0; i < pData->nodeCount; i++) {
			const EX_FLOWGRAPH_NODE& node = pData->nodes[i];
			ryml::NodeRef node_yaml = nodes.append_child();
			node_yaml |= ryml::MAP;
			node_yaml["id"] << node.id;
			node_yaml["card_type"] << node.cardType;
			node_yaml["dynamic_port_count"] << node.dynamicPortCount;
			node_yaml["dynamic_port_count2"] << node.dynamicPortCount2;
			node_yaml["x"] << node.x;
			node_yaml["y"] << node.y;
			node_yaml["width"] << node.width;
			node_yaml["height"] << node.height;
			node_yaml["title"] << Ex_W2U(node.title);
			// 导出端口数据
			ryml::NodeRef ports = node_yaml["ports"];
			ports |= ryml::SEQ;
			for (INT j = 0; j < node.portCount; j++) {
				const EX_FLOWGRAPH_PORT& port = node.ports[j];
				ryml::NodeRef port_yaml = ports.append_child();
				port_yaml |= ryml::MAP;
				port_yaml["id"] << port.id;
				port_yaml["port_type"] << port.portType;
				port_yaml["data_type"] << port.dataType;
				port_yaml["name"] << Ex_W2U(port.name);
				port_yaml["is_connected"] << port.isConnected;
				port_yaml["widget_type"] << port.widgetType;
				port_yaml["widget_id"] << port.widgetId;
				port_yaml["widget_width"] << port.widgetWidth;
				port_yaml["widget_height"] << port.widgetHeight;
				// ★ 新增：导出输入端口的缓存数据（断开连线后仍能保留数据的关键）
				 if (port.dataType == FLOWGRAPH_DATATYPE_AUDIO) {
					if (port.widgetData) {
						LPCWSTR audioPath = (LPCWSTR)port.widgetData;
						if (audioPath && lstrlenW(audioPath) > 0) {
							std::wstring srcPath = audioPath;
							for (auto& c : srcPath) if (c == L'/') c = L'\\';
							std::wstring currentAssetDir = assetDir;
							for (auto& c : currentAssetDir) if (c == L'/') c = L'\\';

							BOOL isInAssetDir = FALSE;
							if (srcPath.length() >= currentAssetDir.length()) {
								std::wstring srcDir = srcPath.substr(0, currentAssetDir.length());
								if (_wcsicmp(srcDir.c_str(), currentAssetDir.c_str()) == 0) isInAssetDir = TRUE;
							}

							if (isInAssetDir) {
								std::wstring relPath = srcPath.substr(currentAssetDir.length());
								if (!relPath.empty() && (relPath[0] == L'\\' || relPath[0] == L'/')) relPath = relPath.substr(1);
								port_yaml["audio_path"] << Ex_W2U((baseName + L"\\" + relPath).c_str());
							}
							else {
								size_t lastSlash = srcPath.find_last_of(L"\\/");
								std::wstring fileName = (lastSlash != std::wstring::npos) ? srcPath.substr(lastSlash + 1) : srcPath;
								WCHAR prefix[64]; swprintf_s(prefix, 64, L"%d_%d_", node.id, port.id);
								std::wstring destFileName = std::wstring(prefix) + fileName;
								std::wstring destPath = assetDir + destFileName;
								CopyFileW(srcPath.c_str(), destPath.c_str(), FALSE);
								port_yaml["audio_path"] << Ex_W2U((baseName + L"\\" + destFileName).c_str());
							}
						}
					}
					continue; // ★ 核心：直接跳过当前端口后续的 COMBO/TEXT/BUTTON 等所有导出逻辑
				}
				if (port.portType == FLOWGRAPH_PORTTYPE_INPUT && port.widgetData != NULL) {
					if (port.dataType == FLOWGRAPH_DATATYPE_STRING) {
						port_yaml["cached_input_text"] << Ex_W2U((LPCWSTR)port.widgetData);
					}
					// 如果需要保留IMAGE类型，可在此处扩展保存图片路径或Base64
				}
				// 导出COMBO数据（含选中索引）
				if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO && port.widgetData) {
					const EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (const EX_FLOWGRAPH_NODE_COMBO_DATA*)port.widgetData;
					ryml::NodeRef combo_yaml = port_yaml["combo_data"];
					combo_yaml |= ryml::MAP;
					combo_yaml["current"] << combo->current;
					ryml::NodeRef options = combo_yaml["options"];
					options |= ryml::SEQ;
					for (INT k = 0; k < combo->count; k++) {
						options.append_child() << Ex_W2U(combo->options[k]);
					}
				}
				// ★ 新增：导出按钮文本
				else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON && port.widgetData) {
					const EX_FLOWGRAPH_NODE_BUTTON_DATA* btn = (const EX_FLOWGRAPH_NODE_BUTTON_DATA*)port.widgetData;
					if (btn->caption) {
						port_yaml["button_caption"] << Ex_W2U(btn->caption);
					}
				}
				else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON && port.widgetData) {
					const EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* dualBtn = (const EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)port.widgetData;
					ryml::NodeRef dual_yaml = port_yaml["dual_button_data"];
					dual_yaml |= ryml::MAP;
					if (dualBtn->caption1) dual_yaml["caption1"] << Ex_W2U(dualBtn->caption1);
					if (dualBtn->caption2) dual_yaml["caption2"] << Ex_W2U(dualBtn->caption2);
				}
				// ★ 新增：导出编辑框文本
				else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT && port.widgetData) {
					port_yaml["edit_text"] << Ex_W2U((LPCWSTR)port.widgetData);
				}
				// ★ 新增：导出只读文本内容
				else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT && port.widgetData) {
					LPCWSTR text = (LPCWSTR)port.widgetData;
					// ★ 核心新增：如果是本地音频节点的路径显示文本，自动转为相对路径并打包素材
					if (node.cardType == FLOWGRAPH_CARD_TYPE_LOCAL_AUDIO && text && lstrlenW(text) > 0) {
						std::wstring srcPath = text;
						for (auto& c : srcPath) if (c == L'/') c = L'\\';

						// 检查是否是一个真实的文件路径
						if (GetFileAttributesW(srcPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
							std::wstring currentAssetDir = assetDir;
							for (auto& c : currentAssetDir) if (c == L'/') c = L'\\';
							BOOL isInAssetDir = FALSE;
							if (srcPath.length() >= currentAssetDir.length()) {
								std::wstring srcDir = srcPath.substr(0, currentAssetDir.length());
								if (_wcsicmp(srcDir.c_str(), currentAssetDir.c_str()) == 0) isInAssetDir = TRUE;
							}
							if (isInAssetDir) {
								std::wstring relPath = srcPath.substr(currentAssetDir.length());
								if (!relPath.empty() && (relPath[0] == L'\\' || relPath[0] == L'/')) relPath = relPath.substr(1);
								port_yaml["text_content"] << Ex_W2U((baseName + L"\\" + relPath).c_str());
							}
							else {
								size_t lastSlash = srcPath.find_last_of(L"\\/");
								std::wstring fileName = (lastSlash != std::wstring::npos) ? srcPath.substr(lastSlash + 1) : srcPath;
								WCHAR prefix[64]; swprintf_s(prefix, 64, L"%d_%d_", node.id, port.id);
								std::wstring destFileName = std::wstring(prefix) + fileName;
								std::wstring destPath = assetDir + destFileName;
								CopyFileW(srcPath.c_str(), destPath.c_str(), FALSE);
								port_yaml["text_content"] << Ex_W2U((baseName + L"\\" + destFileName).c_str());
							}
							continue; // 处理完毕，跳过默认的文本导出
						}
					}
					port_yaml["text_content"] << Ex_W2U(text);
				}
				else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO && port.widgetData) {
					EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)port.widgetData;
					if (pVideo->videoPath) {
						port_yaml["video_path"] << Ex_W2U(pVideo->videoPath);
					}
				}
				// ★★★ 核心修改：仅导出带有图片组件的 非OUTPUT 端口（即预览/加载端口） ★★★
				else if ((port.widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE && port.portType != FLOWGRAPH_PORTTYPE_OUTPUT)) {
					if (port.imagePath) {
						std::wstring srcPath = port.imagePath;
						for (auto& c : srcPath) if (c == L'/') c = L'\\'; // 统一斜杠
						std::wstring currentAssetDir = assetDir;
						for (auto& c : currentAssetDir) if (c == L'/') c = L'\\';

						// ★★★ 核心修复：判断文件是否已经在当前工程的 assetDir 中 ★★★
						BOOL isInAssetDir = FALSE;
						if (srcPath.length() >= currentAssetDir.length()) {
							std::wstring srcDir = srcPath.substr(0, currentAssetDir.length());
							if (_wcsicmp(srcDir.c_str(), currentAssetDir.c_str()) == 0) {
								isInAssetDir = TRUE;
							}
						}

						if (isInAssetDir) {
							// ★ 已经在资产目录中，直接提取相对路径，不重复复制和叠加前缀
							std::wstring relPath = srcPath.substr(currentAssetDir.length());
							if (!relPath.empty() && (relPath[0] == L'\\' || relPath[0] == L'/')) {
								relPath = relPath.substr(1); // 去掉前导斜杠
							}
							port_yaml["image_path"] << Ex_W2U((baseName + L"\\" + relPath).c_str());
						}
						else {
							// ★ 外部新加载的图片，复制到同名目录并防重名
							size_t lastSlash = srcPath.find_last_of(L"\\/");
							std::wstring fileName = (lastSlash != std::wstring::npos) ? srcPath.substr(lastSlash + 1) : srcPath;

							WCHAR prefix[64];
							swprintf_s(prefix, 64, L"%d_%d_", node.id, port.id);
							std::wstring destFileName = std::wstring(prefix) + fileName;
							std::wstring destPath = assetDir + destFileName;

							CopyFileW(srcPath.c_str(), destPath.c_str(), FALSE);
							std::wstring relPath = baseName + L"\\" + destFileName;
							port_yaml["image_path"] << Ex_W2U(relPath.c_str());
						}
					}
					else if (port.widgetData) {
						// ★★★ AI生成的内存图片，自动保存到同名文件夹 ★★★
						WCHAR fileName[256];
						swprintf_s(fileName, 256, L"node_%d_port_%d.png", node.id, port.id);
						std::wstring savePath = assetDir + fileName;

						// ⚠️ 注意：若您的框架保存图片API名为 _img_saveaspng 或 Ex_ImageSaveFile，请自行替换
						if (_img_savetofile((HEXIMAGE)port.widgetData, savePath.c_str())) {
							// 导出相对路径：包含同名文件夹前缀，如 "我的工程\node_1_port_0.png"
							std::wstring relPath = baseName + L"\\" + std::wstring(fileName);
							port_yaml["image_path"] << Ex_W2U(relPath.c_str());
						}
					}
				}
				
			}
		}
		// 4. 导出连接线数据
		ryml::NodeRef connections = root["connections"];
		connections |= ryml::SEQ;
		for (INT i = 0; i < pData->connectionCount; i++) {
			const EX_FLOWGRAPH_CONNECTION& conn = pData->connections[i];
			ryml::NodeRef conn_yaml = connections.append_child();
			conn_yaml |= ryml::MAP;
			conn_yaml["id"] << conn.id;
			conn_yaml["from_node"] << conn.fromNode;
			conn_yaml["from_slot"] << conn.fromSlot;
			conn_yaml["to_node"] << conn.toNode;
			conn_yaml["to_slot"] << conn.toSlot;
			ryml::NodeRef cp1 = conn_yaml["control_point1"];
			cp1 |= ryml::MAP;
			cp1["x"] << conn.controlPoint1.x;
			cp1["y"] << conn.controlPoint1.y;
			ryml::NodeRef cp2 = conn_yaml["control_point2"];
			cp2 |= ryml::MAP;
			cp2["x"] << conn.controlPoint2.x;
			cp2["y"] << conn.controlPoint2.y;
		}
		// 5. 写入YAML文件
		std::ofstream file(filePath);
		if (!file.is_open()) return FALSE;
		file << tree;
		file.close();
		// ★★★ 核心新增：提取YAML文件名(去后缀)作为工程名 ★★★
	

		lstrcpynW(pData->projectName, baseName.c_str(), 256);

		Ex_ObjInvalidateRect(hObj, 0); // 触发重绘显示工程名
		return TRUE;
	}
	catch (...) {
		return FALSE;
	}
}

// ==================== 导入YAML ====================
BOOL _flowgraph_import_from_yaml(HEXOBJ hObj, LPCWSTR filePath) {
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData) return FALSE;
	INT* oldNodeIds = NULL;
	INT* newNodeIds = NULL;
	INT yamlNodeCount = 0;
	try {
		// ★★★ 核心修改：提前计算YAML所在目录，用于后续拼接相对路径 ★★★
		std::wstring yamlFullPath = filePath;
		size_t lastSlash = yamlFullPath.find_last_of(L"\\/");
		std::wstring yamlDir = (lastSlash != std::wstring::npos) ? yamlFullPath.substr(0, lastSlash + 1) : L"";
		std::ifstream file(filePath);
		if (!file.is_open()) return FALSE;
		std::string yaml_str((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());
		file.close();
		ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(yaml_str));
		ryml::NodeRef root = tree.rootref();
		if (!root.is_map()) return FALSE;
		_flowgraph_clear_data(pData);
		// 1. 导入视图设置
		if (root.has_child("view")) {
			ryml::NodeRef view = root["view"];
			view["zoom"] >> pData->zoom;
			view["pan_offset_x"] >> pData->panOffset.x;
			view["pan_offset_y"] >> pData->panOffset.y;
		}
		// ★ 2. 导入并自动注册卡片类型
		if (root.has_child("card_types")) {
			ryml::NodeRef card_types = root["card_types"];
			if (card_types.is_seq()) {
				for (INT i = 0; i < (INT)card_types.num_children(); i++) {
					ryml::NodeRef ct = card_types[i];
					EX_FLOWGRAPH_CARD_DESCRIPTOR desc = { 0 };
					ct["card_type"] >> desc.cardType;
					if (_flowgraph_find_card_descriptor(pData, desc.cardType) != NULL) {
						continue;
					}
					std::string name_utf8; ct["type_name"] >> name_utf8;
					desc.typeName = StrDupW(Ex_U2W(name_utf8).c_str());
					INT tagColorInt = 0;
					ct["tag_color"] >> tagColorInt;
					desc.tagColor = (EXARGB)tagColorInt;
					ct["dynamic_port_base_index"] >> desc.dynamicPortBaseIndex;
					ct["dynamic_port_min_count"] >> desc.dynamicPortMinCount;
					ct["dynamic_port_max_count"] >> desc.dynamicPortMaxCount;
					ct["dynamic_port_data_type"] >> desc.dynamicPortDataType;
					std::string prefix_utf8;
					ct["dynamic_port_name_prefix"] >> prefix_utf8;
					desc.dynamicPortNamePrefix = StrDupW(Ex_U2W(prefix_utf8).c_str());

					ct["dynamic_port2_base_index"] >> desc.dynamicPort2BaseIndex;
					ct["dynamic_port2_min_count"] >> desc.dynamicPort2MinCount;
					ct["dynamic_port2_max_count"] >> desc.dynamicPort2MaxCount;
					ct["dynamic_port2_data_type"] >> desc.dynamicPort2DataType;
					std::string prefix2_utf8;
					ct["dynamic_port2_name_prefix"] >> prefix2_utf8;
					desc.dynamicPort2NamePrefix = StrDupW(Ex_U2W(prefix2_utf8).c_str());

					if (ct.has_child("ports")) {
						ryml::NodeRef ports_yaml = ct["ports"];
						desc.portCount = (INT)ports_yaml.num_children();
						desc.ports = (EX_FLOWGRAPH_PORT_DESC*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_PORT_DESC) * desc.portCount);
						memset(desc.ports, 0, sizeof(EX_FLOWGRAPH_PORT_DESC) * desc.portCount);
						for (INT j = 0; j < desc.portCount; j++) {
							ryml::NodeRef p = ports_yaml[j];
							p["port_type"] >> desc.ports[j].portType;
							p["data_type"] >> desc.ports[j].dataType;
							std::string pname_utf8; p["name"] >> pname_utf8;
							desc.ports[j].name = StrDupW(Ex_U2W(pname_utf8).c_str());
							p["widget_type"] >> desc.ports[j].widgetType;
							p["widget_id"] >> desc.ports[j].widgetId;
							p["widget_width"] >> desc.ports[j].widgetWidth;
							p["widget_height"] >> desc.ports[j].widgetHeight;
						}
					}
					_flowgraph_register_card_type(hObj, &desc);
					// 释放临时分配的内存（_flowgraph_register_card_type 内部做了深拷贝）
					if (desc.typeName) Ex_MemFree((void*)desc.typeName);
					if (desc.dynamicPortNamePrefix) Ex_MemFree((void*)desc.dynamicPortNamePrefix);
					if (desc.ports) {
						for (INT j = 0; j < desc.portCount; j++) {
							if (desc.ports[j].name) Ex_MemFree((void*)desc.ports[j].name);
						}
						Ex_MemFree(desc.ports);
					}
				}
			}
		}
		// ★ 3. 导入节点数据（基于描述符模板创建）
		if (root.has_child("nodes")) {
			ryml::NodeRef nodes = root["nodes"];
			if (nodes.is_seq()) {
				yamlNodeCount = (INT)nodes.num_children();
				oldNodeIds = (INT*)Ex_MemAlloc(sizeof(INT) * yamlNodeCount);
				newNodeIds = (INT*)Ex_MemAlloc(sizeof(INT) * yamlNodeCount);
				for (INT i = 0; i < yamlNodeCount; i++) {
					ryml::NodeRef node_yaml = nodes[i];
					INT oldId, cardType;
					FLOAT x, y;
					node_yaml["id"] >> oldId;
					node_yaml["card_type"] >> cardType;
					node_yaml["x"] >> x;
					node_yaml["y"] >> y;
					// ★ 使用描述符创建基础节点
					EX_FLOWGRAPH_CUSTOM_NODE_CREATE create = { 0 };
					create.cardType = cardType;
					create.x = x;
					create.y = y;
					EX_FLOWGRAPH_CARD_DESCRIPTOR* pDesc = _flowgraph_find_card_descriptor(pData, cardType);
					create.title = pDesc ? pDesc->typeName : L"未知节点";
					INT newId = _flowgraph_create_custom_node(hObj, &create);
					if (newId == 0) continue; // 找不到卡片描述符，跳过
					oldNodeIds[i] = oldId;
					newNodeIds[i] = newId;
					EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, newId);
					if (!node) continue;
					// ★ 恢复标题（覆盖注册时的默认名）
					if (node_yaml.has_child("title")) {
						std::string title_utf8;
						node_yaml["title"] >> title_utf8;
						Ex_MemFree((void*)node->title);
						node->title = StrDupW(Ex_U2W(title_utf8).c_str());
					}
					// ★ 恢复精确坐标（覆盖自动避让偏移）
					node->x = x;
					node->y = y;
					// ★ 动态补充端口数量
					INT dynPortCount = 0;
					if (node_yaml.has_child("dynamic_port_count")) {
						node_yaml["dynamic_port_count"] >> dynPortCount;
					}
					while (node->dynamicPortCount < dynPortCount) {
						_flowgraph_add_dynamic_port(hObj, node->id);
					}
					INT dynPortCount2 = 0;
					if (node_yaml.has_child("dynamic_port_count2")) {
						node_yaml["dynamic_port_count2"] >> dynPortCount2;
					}
					while (node->dynamicPortCount2 < dynPortCount2) {
						_flowgraph_add_dynamic_port2(hObj, node->id);
					}
					// ★ 按索引恢复端口状态（覆盖模板默认数据）
					if (node_yaml.has_child("ports")) {
						ryml::NodeRef ports_yaml = node_yaml["ports"];
						INT portCount = __min(node->portCount, (INT)ports_yaml.num_children());
						for (INT j = 0; j < portCount; j++) {
							ryml::NodeRef port_yaml = ports_yaml[j];
							EX_FLOWGRAPH_PORT& port = node->ports[j];
							// 先释放模板中创建的默认数据
							if (port.widgetData) {
								if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT || port.widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT)
									Ex_MemFree(port.widgetData);
								else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO) {
									EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)port.widgetData;
									for (INT k = 0; k < combo->count; k++) Ex_MemFree((void*)combo->options[k]);
									Ex_MemFree(combo->options); Ex_MemFree(combo);
								}
								else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE)
								{
									_img_destroy((HEXIMAGE)port.widgetData);
									if (port.imagePath) Ex_MemFree((void*)port.imagePath); // ★ 新增
									port.widgetData = NULL;
									port.imagePath = NULL;
								}
								else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON) {
									EX_FLOWGRAPH_NODE_BUTTON_DATA* btn = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)port.widgetData;
									Ex_MemFree((void*)btn->caption); Ex_MemFree(btn);
								}
								else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON) {
									EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* dualBtn = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)port.widgetData;
									Ex_MemFree((void*)dualBtn->caption1); Ex_MemFree((void*)dualBtn->caption2); Ex_MemFree(dualBtn);
								}
								else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO) {
									EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)port.widgetData;
									if (pVideo) { _flowgraph_video_cleanup(pVideo); DeleteCriticalSection(&pVideo->critsec); Ex_MemFree(pVideo); }
								}
								port.widgetData = NULL;
							}
							// 从 YAML 加载真实数据覆盖
							// ★★★ 核心修复：本地音频导入 (强制优先使用 audio_path，并智能拼接/Fallback路径) ★★★
							if (port_yaml.has_child("audio_path")) {
								std::string path_utf8; port_yaml["audio_path"] >> path_utf8;
								std::wstring wPath = Ex_U2W(path_utf8);

								// 智能路径处理：如果是相对路径，则自动拼接 YAML 所在目录
								BOOL isRelative = (wPath.length() < 2 || (wPath[1] != L':' && wPath[0] != L'\\' && wPath[0] != L'/'));
								if (isRelative) {
									for (auto& c : wPath) if (c == L'/') c = L'\\';
									wPath = yamlDir + wPath;
								}

								// 清理可能残留的旧数据
								if (port.widgetData) {
									Ex_MemFree(port.widgetData);
									port.widgetData = NULL;
								}

								if (GetFileAttributesW(wPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
									port.widgetData = (LPVOID)StrDupW(wPath.c_str());
								}
								else {
									// ★ Fallback 机制：如果拼接后找不到，尝试剥离工程名前缀（如 "我的工程\"），直接在 yamlDir 下查找
									size_t slashPos = wPath.find(L'\\');
									if (slashPos != std::wstring::npos) {
										std::wstring fallbackPath = yamlDir + wPath.substr(slashPos + 1);
										if (GetFileAttributesW(fallbackPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
											port.widgetData = (LPVOID)StrDupW(fallbackPath.c_str());
										}
										else {
											port.widgetData = NULL; // 彻底置空，防止UI渲染旧路径
										}
									}
									else {
										port.widgetData = NULL;
									}
								}
							}
							else if (port.dataType == FLOWGRAPH_DATATYPE_AUDIO && port_yaml.has_child("cached_input_text")) {
								// 兼容极旧版本 YAML：如果没有 audio_path 但有 cached_input_text
								std::string text_utf8; port_yaml["cached_input_text"] >> text_utf8;
								std::wstring wPath = Ex_U2W(text_utf8);
								if (port.widgetData) Ex_MemFree(port.widgetData);
								port.widgetData = (LPVOID)StrDupW(wPath.c_str());
							}
							else if (port.portType == FLOWGRAPH_PORTTYPE_INPUT && port_yaml.has_child("cached_input_text")) {
								std::string text_utf8; port_yaml["cached_input_text"] >> text_utf8;
								port.widgetData = (LPVOID)StrDupW(Ex_U2W(text_utf8).c_str());
							}
							else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO && port_yaml.has_child("combo_data")) {
								EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE_COMBO_DATA));
								memset(combo, 0, sizeof(EX_FLOWGRAPH_NODE_COMBO_DATA));
								ryml::NodeRef combo_yaml = port_yaml["combo_data"];
								combo_yaml["current"] >> combo->current;
								ryml::NodeRef options = combo_yaml["options"];
								combo->count = (INT)options.num_children();
								combo->options = (LPCWSTR*)Ex_MemAlloc(combo->count * sizeof(LPCWSTR));
								for (INT k = 0; k < combo->count; k++) {
									std::string opt_utf8; options[k] >> opt_utf8;
									combo->options[k] = StrDupW(Ex_U2W(opt_utf8).c_str());
								}
								port.widgetData = combo;
							}
							else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON) {
								EX_FLOWGRAPH_NODE_BUTTON_DATA* btn = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE_BUTTON_DATA));
								memset(btn, 0, sizeof(EX_FLOWGRAPH_NODE_BUTTON_DATA));
								if (port_yaml.has_child("button_caption")) {
									std::string caption_utf8; port_yaml["button_caption"] >> caption_utf8;
									btn->caption = StrDupW(Ex_U2W(caption_utf8).c_str());
								}
								else {
									btn->caption = StrDupW(L"按钮");
								}
								port.widgetData = btn;
							}
							else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON) {
								EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* dualBtn = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA));
								memset(dualBtn, 0, sizeof(EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA));
								if (port_yaml.has_child("dual_button_data")) {
									ryml::NodeRef dual_yaml = port_yaml["dual_button_data"];
									if (dual_yaml.has_child("caption1")) {
										std::string cap1_utf8; dual_yaml["caption1"] >> cap1_utf8;
										dualBtn->caption1 = StrDupW(Ex_U2W(cap1_utf8).c_str());
									}
									if (dual_yaml.has_child("caption2")) {
										std::string cap2_utf8; dual_yaml["caption2"] >> cap2_utf8;
										dualBtn->caption2 = StrDupW(Ex_U2W(cap2_utf8).c_str());
									}
								}
								port.widgetData = dualBtn;
							}
							else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT) {
								if (port_yaml.has_child("edit_text")) {
									std::string text_utf8; port_yaml["edit_text"] >> text_utf8;
									port.widgetData = (LPVOID)StrDupW(Ex_U2W(text_utf8).c_str());
								}
							}
							else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT) {
								if (port_yaml.has_child("text_content")) {
									std::string text_utf8; port_yaml["text_content"] >> text_utf8;
									std::wstring wText = Ex_U2W(text_utf8);

									// ★ 核心修复：如果是本地音频节点的路径文本，且为相对路径，自动拼接为绝对路径
									if (node->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_AUDIO && !wText.empty()) {
										BOOL isRelative = (wText.length() < 2 || (wText[1] != L':' && wText[0] != L'\\' && wText[0] != L'/'));
										if (isRelative) {
											for (auto& c : wText) if (c == L'/') c = L'\\';
											wText = yamlDir + wText;
										}
									}
									port.widgetData = (LPVOID)StrDupW(wText.c_str());
								}
							}
							else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO) {
								EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = _flowgraph_create_video_data(pData->libVlc, hObj);
								if (port_yaml.has_child("video_path")) {
									std::string path_utf8; port_yaml["video_path"] >> path_utf8;
									std::wstring wPath = Ex_U2W(path_utf8);

									// ★★★ 智能路径处理：如果是相对路径，则自动拼接 YAML 所在目录 ★★★
									BOOL isRelative = (wPath.length() < 2 || (wPath[1] != L':' && wPath[0] != L'\\'));
									if (isRelative) {
										std::wstring yamlDir = std::wstring(filePath).substr(0, std::wstring(filePath).find_last_of(L"\\/") + 1);
										wPath = yamlDir + wPath;
									}

									pVideo->videoPath = StrDupW(wPath.c_str());
									if (GetFileAttributesW(wPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
										_flowgraph_video_load(pVideo, pVideo->videoPath, TRUE);
									}
								}
								port.widgetData = (LPVOID)pVideo;
							}
							else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE && port.portType != FLOWGRAPH_PORTTYPE_OUTPUT && port_yaml.has_child("image_path")) {
								std::string path_utf8; port_yaml["image_path"] >> path_utf8;
								std::wstring wPath = Ex_U2W(path_utf8);

								// ★★★ 智能路径处理：相对路径自动拼接YAML所在目录 ★★★
								BOOL isRelative = (wPath.length() < 2 || (wPath[1] != L':' && wPath[0] != L'\\' && wPath[0] != L'/'));
								if (isRelative) {
									for (auto& c : wPath) if (c == L'/') c = L'\\'; // 统一斜杠风格
									wPath = yamlDir + wPath; // yamlDir已含末尾\，直接拼接 "我的工程\xxx.png"
								}

								port.imagePath = StrDupW(wPath.c_str());
								if (GetFileAttributesW(wPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
									HEXIMAGE hImg = 0;
									hImg = _flowgraph_load_image_no_lock(wPath.c_str());
									if (hImg) {
										port.widgetData = (LPVOID)hImg;
										_flowgraph_adjust_image_size(&port);
									}
								}
							}
							
							// 恢复组件自定义尺寸
							if (port_yaml.has_child("widget_width")) port_yaml["widget_width"] >> port.widgetWidth;
							if (port_yaml.has_child("widget_height")) port_yaml["widget_height"] >> port.widgetHeight;
						}
					}
					_flowgraph_calcnodesize(hObj, node);
				}
			}
		}
		// ★ 4. 导入连接线时进行ID映射
		if (root.has_child("connections")) {
			ryml::NodeRef connections = root["connections"];
			if (connections.is_seq()) {
				pData->connectionCount = (INT)connections.num_children();
				pData->connections = (EX_FLOWGRAPH_CONNECTION*)Ex_MemAlloc(pData->connectionCount * sizeof(EX_FLOWGRAPH_CONNECTION));
				memset(pData->connections, 0, pData->connectionCount * sizeof(EX_FLOWGRAPH_CONNECTION));
				for (INT i = 0; i < pData->connectionCount; i++) {
					ryml::NodeRef conn_yaml = connections[i];
					EX_FLOWGRAPH_CONNECTION& conn = pData->connections[i];
					conn_yaml["id"] >> conn.id;
					INT oldFromNode, oldToNode;
					conn_yaml["from_node"] >> oldFromNode;
					conn_yaml["to_node"] >> oldToNode;
					conn.fromNode = _flowgraph_map_yaml_id(oldFromNode, oldNodeIds, newNodeIds, yamlNodeCount);
					conn.toNode = _flowgraph_map_yaml_id(oldToNode, oldNodeIds, newNodeIds, yamlNodeCount);
					conn_yaml["from_slot"] >> conn.fromSlot;
					conn_yaml["to_slot"] >> conn.toSlot;
					ryml::NodeRef cp1 = conn_yaml["control_point1"];
					cp1["x"] >> conn.controlPoint1.x;
					cp1["y"] >> conn.controlPoint1.y;
					ryml::NodeRef cp2 = conn_yaml["control_point2"];
					cp2["x"] >> conn.controlPoint2.x;
					cp2["y"] >> conn.controlPoint2.y;
					// ★ 更新端口连接状态
					EX_FLOWGRAPH_NODE* toNode = _flowgraph_findnode(pData, conn.toNode);
					if (toNode && conn.toSlot < toNode->portCount) {
						toNode->ports[conn.toSlot].isConnected = TRUE;
					}
				}
			}
			// ★★★ 终极修复：强制校准所有节点的输入端口 isConnected 状态 ★★★
			// 原因：动态端口在导入时如果发生索引偏移，会导致 toSlot 指向错误的端口，
			// 从而使得真正的动态输入端口 isConnected 依然为 FALSE，导致 @ 面板漏显。

			// 1. 先将所有节点的输入端口重置为 FALSE
			for (INT i = 0; i < pData->nodeCount; i++) {
				EX_FLOWGRAPH_NODE* n = &pData->nodes[i];
				for (INT j = 0; j < n->portCount; j++) {
					if (n->ports[j].portType == FLOWGRAPH_PORTTYPE_INPUT) {
						n->ports[j].isConnected = FALSE;
					}
				}
			}

			// 2. 遍历导入后的连线数组，精准标记目标端口
			for (INT i = 0; i < pData->connectionCount; i++) {
				EX_FLOWGRAPH_CONNECTION& conn = pData->connections[i];
				EX_FLOWGRAPH_NODE* toNode = _flowgraph_findnode(pData, conn.toNode);
				if (toNode && conn.toSlot >= 0 && conn.toSlot < toNode->portCount) {
					toNode->ports[conn.toSlot].isConnected = TRUE;
				}
			}
		}
		// ★ 释放映射表
		if (oldNodeIds) Ex_MemFree(oldNodeIds);
		if (newNodeIds) Ex_MemFree(newNodeIds);
		// ★★★ 核心新增：提取YAML文件名(去后缀)作为工程名 ★★★

		std::wstring yamlFileName = (lastSlash != std::wstring::npos) ? yamlFullPath.substr(lastSlash + 1) : yamlFullPath;
		size_t dotPos = yamlFileName.find_last_of(L".");
		std::wstring baseName = (dotPos != std::wstring::npos) ? yamlFileName.substr(0, dotPos) : yamlFileName;
		lstrcpynW(pData->projectName, baseName.c_str(), 256);


		_flowgraph_updatelayout(hObj);
		Ex_ObjScrollSetPos(hObj, SCROLLBAR_TYPE_HORZ, (INT)pData->panOffset.x, TRUE);
		Ex_ObjScrollSetPos(hObj, SCROLLBAR_TYPE_VERT, (INT)pData->panOffset.y, TRUE);
		Ex_ObjInvalidateRect(hObj, 0);
		return TRUE;
	}
	catch (...) {
		if (oldNodeIds) Ex_MemFree(oldNodeIds);
		if (newNodeIds) Ex_MemFree(newNodeIds);
		_flowgraph_clear_data(pData);
		Ex_ObjInvalidateRect(hObj, 0);
		return FALSE;
	}
}

// ==================== 自动避让重叠位置 ====================
void _flowgraph_avoid_overlap(EX_FLOWGRAPH_DATA* pData, EX_FLOWGRAPH_NODE* newNode)
{
	const FLOAT spacing = 20.0f;   // 节点间距
	for (INT attempts = 0; attempts < 500; attempts++) {
		BOOL overlapping = FALSE;
		for (INT i = 0; i < pData->nodeCount; i++) {
			EX_FLOWGRAPH_NODE* existing = &pData->nodes[i];
			// AABB碰撞检测（含间距）
			if (newNode->x < existing->x + existing->width + spacing &&
				newNode->x + newNode->width + spacing > existing->x &&
				newNode->y < existing->y + existing->height + spacing &&
				newNode->y + newNode->height + spacing > existing->y) {
				overlapping = TRUE;
				// 向下偏移到已有节点下方
				newNode->y = existing->y + existing->height + spacing;
				break;
			}
		}
		if (!overlapping) return;
	}
}


// ==================== 获取卡片类型名称 ====================
LPCWSTR _flowgraph_get_cardtype_name(EX_FLOWGRAPH_DATA* pData, INT cardType) {
	EX_FLOWGRAPH_CARD_DESCRIPTOR* desc = _flowgraph_find_card_descriptor(pData, cardType);
	if (desc) return desc->typeName;
	return L"自定义";
}

// ==================== 获取执行状态文本 ====================
LPCWSTR _flowgraph_get_status_text(INT status) {
	switch (status) {
	case FLOWGRAPH_EXEC_STATUS_IDLE:      return L"○ 未执行";
	case FLOWGRAPH_EXEC_STATUS_PENDING:   return L"● 等待执行";
	case FLOWGRAPH_EXEC_STATUS_RUNNING:   return L"● 执行中...";
	case FLOWGRAPH_EXEC_STATUS_COMPLETED: return L"● 已完成";
	case FLOWGRAPH_EXEC_STATUS_FAILED:    return L"● 执行失败";
	default: return L"○ 未知";
	}
}

// ==================== 获取执行状态颜色 ====================
EXARGB _flowgraph_get_status_color(INT status) {
	switch (status) {
	case FLOWGRAPH_EXEC_STATUS_IDLE:      return ExARGB(128, 128, 128, 255);
	case FLOWGRAPH_EXEC_STATUS_PENDING:   return ExARGB(255, 200, 0, 255);
	case FLOWGRAPH_EXEC_STATUS_RUNNING:   return ExARGB(0, 150, 255, 255);
	case FLOWGRAPH_EXEC_STATUS_COMPLETED: return ExARGB(0, 200, 80, 255);
	case FLOWGRAPH_EXEC_STATUS_FAILED:    return ExARGB(255, 60, 60, 255);
	default: return ExARGB(128, 128, 128, 255);
	}
}

// ==================== 绘制节点信息提示框 ====================
void _flowgraph_draw_node_tooltip(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData,
EX_FLOWGRAPH_NODE* node, FLOAT mouseX, FLOAT mouseY, FLOAT canvasWidth, FLOAT canvasHeight)
{
	// ★★★ 1. 收集输入图片端口的预览图及端口名称 (支持未执行时透传上游本地图片) ★★★
	struct PreviewInfo {
		HEXIMAGE hImg;
		LPCWSTR name;
	};
	PreviewInfo previewImages[20]; // 最多支持20个输入图片端口
	INT imgCount = 0;
	for (INT i = 0; i < node->portCount && imgCount < 20; i++) {
		if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_INPUT &&
			node->ports[i].dataType == FLOWGRAPH_DATATYPE_IMAGE) {

			HEXIMAGE hImg = (HEXIMAGE)node->ports[i].widgetData;

			// ★★★ 核心修复：如果当前端口没数据，但已连线，尝试去上游“本地图片”节点透传获取 ★★★
			if (!hImg && node->ports[i].isConnected) {
				for (INT c = 0; c < pData->connectionCount; c++) {
					if (pData->connections[c].toNode == node->id && pData->connections[c].toSlot == i) {
						EX_FLOWGRAPH_NODE* fromNode = _flowgraph_findnode(pData, pData->connections[c].fromNode);
						// 仅当上游是“本地图片”节点时才透传预览 (文生图等不预览)
						if (fromNode && fromNode->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_IMAGE) {
							// ★★★ 修复：不再依赖 fromSlot (输出端口未执行时为空)，而是遍历上游节点的所有端口寻找图片 ★★★
							for (INT k = 0; k < fromNode->portCount; k++) {
								if (fromNode->ports[k].dataType == FLOWGRAPH_DATATYPE_IMAGE && fromNode->ports[k].widgetData) {
									hImg = (HEXIMAGE)fromNode->ports[k].widgetData;
									break;
								}
							}
						}
						break;
					}
				}
			}

			if (hImg) {
				previewImages[imgCount].hImg = hImg;
				previewImages[imgCount].name = node->ports[i].name; // ★ 记录端口名称
				imgCount++;
			}
		}
	}

	// ★★★ 2. 计算图片区域尺寸 ★★★
	FLOAT imgW = 200.0f, imgH = 200.0f, imgGap = 10.0f;
	INT imgRows = (imgCount + 2) / 3; // 每行3个，计算行数
	INT imgCols = (imgCount > 3) ? 3 : imgCount;
	FLOAT imgAreaW = (imgCount > 0) ? (imgCols * imgW + (imgCols > 1 ? (imgCols - 1) * imgGap : 0)) : 0;
	FLOAT imgAreaH = (imgCount > 0) ? (imgRows * imgH + (imgRows > 1 ? (imgRows - 1) * imgGap : 0)) : 0;
	FLOAT textImgGap = (imgCount > 0) ? 15.0f : 0; // 文本与图片的间距

	// 3. 构建提示文本 (原有逻辑)
	WCHAR tooltip[2048];
	INT pos = 0;
	pos += swprintf_s(tooltip + pos, 2048 - pos, L"节点: %s(nodeId:%d)\n", node->title, node->id);
	pos += swprintf_s(tooltip + pos, 2048 - pos, L"类型: %s\n", _flowgraph_get_cardtype_name(pData, node->cardType));
	pos += swprintf_s(tooltip + pos, 2048 - pos, L"状态: %s\n", _flowgraph_get_status_text(node->executionStatus));

	// 输入来源
	pos += swprintf_s(tooltip + pos, 2048 - pos, L"\n── 输入来源 ──");
	BOOL hasInputPort = FALSE;
	for (INT i = 0; i < node->portCount; i++) {
		if (node->ports[i].portType != FLOWGRAPH_PORTTYPE_INPUT) continue;
		hasInputPort = TRUE;
		BOOL connected = FALSE;
		for (INT j = 0; j < pData->connectionCount; j++) {
			if (pData->connections[j].toNode == node->id && pData->connections[j].toSlot == i) {
				EX_FLOWGRAPH_NODE* fromNode = _flowgraph_findnode(pData, pData->connections[j].fromNode);
				if (fromNode) {
					INT fromSlot = pData->connections[j].fromSlot;
					if (fromSlot < fromNode->portCount) {
						pos += swprintf_s(tooltip + pos, 2048 - pos, L"\n%s(portId:%d) ← %s(nodeId:%d):%s(portId:%d)",
							node->ports[i].name, node->ports[i].id,
							fromNode->title, fromNode->id,
							fromNode->ports[fromSlot].name, fromNode->ports[fromSlot].id);
					}
				}
				connected = TRUE;
				break;
			}
		}
		if (!connected) {
			pos += swprintf_s(tooltip + pos, 2048 - pos, L"\n%s(portId:%d): 未连接",
				node->ports[i].name, node->ports[i].id);
		}
	}
	if (!hasInputPort) {
		pos += swprintf_s(tooltip + pos, 2048 - pos, L"\n(无输入端口)");
	}

	// 输出目标
	pos += swprintf_s(tooltip + pos, 2048 - pos, L"\n── 输出目标 ──");
	BOOL hasOutputPort = FALSE;
	for (INT i = 0; i < node->portCount; i++) {
		if (node->ports[i].portType != FLOWGRAPH_PORTTYPE_OUTPUT) continue;
		hasOutputPort = TRUE;
		BOOL connected = FALSE;
		for (INT j = 0; j < pData->connectionCount; j++) {
			if (pData->connections[j].fromNode == node->id && pData->connections[j].fromSlot == i) {
				EX_FLOWGRAPH_NODE* toNode = _flowgraph_findnode(pData, pData->connections[j].toNode);
				if (toNode) {
					INT toSlot = pData->connections[j].toSlot;
					if (toSlot < toNode->portCount) {
						pos += swprintf_s(tooltip + pos, 2048 - pos, L"\n%s(portId:%d) → %s(nodeId:%d):%s(portId:%d)",
							node->ports[i].name, node->ports[i].id,
							toNode->title, toNode->id,
							toNode->ports[toSlot].name, toNode->ports[toSlot].id);
					}
				}
				connected = TRUE;
			}
		}
		if (!connected) {
			pos += swprintf_s(tooltip + pos, 2048 - pos, L"\n%s(portId:%d): 未连接",
				node->ports[i].name, node->ports[i].id);
		}
	}
	if (!hasOutputPort) {
		pos += swprintf_s(tooltip + pos, 2048 - pos, L"\n(无输出端口)");
	}

	// ★★★ 4. 计算文本与整体尺寸 ★★★
	HEXFONT hFont = _font_createfromfamily(L"微软雅黑", 10, 0);
	if (!hFont) return;
	FLOAT textW = 0, textH = 0;
	FLOAT calcMaxW = __max(410.0f, imgAreaW);
	_canvas_calctextsize(hCanvas, hFont, tooltip, -1, DT_LEFT | DT_TOP | DT_WORDBREAK, calcMaxW, 9999.0f, &textW, &textH);

	FLOAT totalW = __max(textW, imgAreaW) + 16.0f;
	FLOAT totalH = textH + imgAreaH + textImgGap + 16.0f;

	// 5. 定位提示框
	FLOAT tipX = mouseX + 15;
	FLOAT tipY = mouseY - totalH / 2;
	if (tipX + totalW + 16 > canvasWidth) tipX = mouseX - totalW - 20;
	if (tipX < 5) tipX = 5;
	if (tipY + totalH + 16 > canvasHeight) tipY = canvasHeight - totalH - 16;
	if (tipY < 5) tipY = 5;

	// 6. 绘制背景、状态色条、边框
	HEXBRUSH hBrushBg = _brush_create(ExARGB(30, 30, 35, 235));
	_canvas_fillrect(hCanvas, hBrushBg, tipX - 8, tipY - 8, tipX - 8 + totalW, tipY - 8 + totalH);
	_brush_destroy(hBrushBg);

	EXARGB statusColor = _flowgraph_get_status_color(node->executionStatus);
	HEXBRUSH hBrushStatus = _brush_create(statusColor);
	_canvas_fillrect(hCanvas, hBrushStatus, tipX - 8, tipY - 8, tipX - 4, tipY - 8 + totalH);
	_brush_destroy(hBrushStatus);

	HEXBRUSH hBrushBorder = _brush_create(ExARGB(80, 80, 100, 200));
	_canvas_drawrect(hCanvas, hBrushBorder, tipX - 8, tipY - 8, tipX - 8 + totalW, tipY - 8 + totalH, 1.0f, 0);
	_brush_destroy(hBrushBorder);

	// 7. 绘制文本
	_canvas_drawtext(hCanvas, hFont, ExARGB(220, 220, 220, 255), tooltip, -1,
		DT_LEFT | DT_TOP | DT_WORDBREAK,
		tipX, tipY, tipX + textW, tipY + textH);
	_font_destroy(hFont);

	// ★★★ 8. 绘制图片预览区及端口标识 ★★★
	if (imgCount > 0) {
		FLOAT startX = tipX;
		FLOAT startY = tipY + textH + textImgGap;

		// 创建标签字体
		HEXFONT hFontLabel = _font_createfromfamily(L"微软雅黑", 9, FONT_STYLE_BOLD);

		for (INT k = 0; k < imgCount; k++) {
			INT row = k / 3;
			INT col = k % 3;
			FLOAT ix = startX + col * (imgW + imgGap);
			FLOAT iy = startY + row * (imgH + imgGap);

			// 绘制图片底色与边框
			HEXBRUSH hBrushImgBg = _brush_create(ExARGB(20, 20, 25, 255));
			_canvas_fillrect(hCanvas, hBrushImgBg, ix, iy, ix + imgW, iy + imgH);
			_brush_destroy(hBrushImgBg);

			HEXBRUSH hBrushImgBorder = _brush_create(ExARGB(60, 60, 70, 255));
			_canvas_drawrect(hCanvas, hBrushImgBorder, ix, iy, ix + imgW, iy + imgH, 1.0f, 0);
			_brush_destroy(hBrushImgBorder);

			// 绘制真实图片
			HEXIMAGE hImg = previewImages[k].hImg;
			INT srcW, srcH;
			_img_getsize(hImg, &srcW, &srcH);
			if (srcW > 0 && srcH > 0) {
				_canvas_drawimagerectrect(hCanvas, hImg, ix, iy, ix + imgW, iy + imgH, 0, 0, srcW, srcH, 255);
			}

			// ★★★ 核心新增：绘制端口名称 OSD 标签条 ★★★
			FLOAT labelH = 22.0f;
			HEXBRUSH hBrushLabelBg = _brush_create(ExARGB(0, 0, 0, 160)); // 半透明黑底
			_canvas_fillrect(hCanvas, hBrushLabelBg, ix, iy, ix + imgW, iy + labelH);
			_brush_destroy(hBrushLabelBg);

			// 绘制标签文字 (带阴影效果更佳)
			if (previewImages[k].name) {
				// 文字阴影
				_canvas_drawtext(hCanvas, hFontLabel, ExARGB(0, 0, 0, 180), previewImages[k].name, -1,
					DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS,
					ix + 5, iy + 1, ix + imgW - 4, iy + labelH + 1);
				// 文字本体
				_canvas_drawtext(hCanvas, hFontLabel, ExARGB(255, 255, 255, 240), previewImages[k].name, -1,
					DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS,
					ix + 4, iy, ix + imgW - 4, iy + labelH);
			}
		}
		_font_destroy(hFontLabel);
	}
}

// ==================== 级联失效下游节点状态 ====================
void _flowgraph_invalidate_downstream(EX_FLOWGRAPH_DATA* pData, INT nodeId)
{
	for (INT i = 0; i < pData->connectionCount; i++) {
		if (pData->connections[i].fromNode == nodeId) {
			EX_FLOWGRAPH_NODE* toNode = _flowgraph_findnode(pData, pData->connections[i].toNode);
			if (toNode && toNode->executionStatus != FLOWGRAPH_EXEC_STATUS_IDLE) {
				toNode->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
				_flowgraph_invalidate_downstream(pData, toNode->id); // 递归失效
			}
		}
	}
}

// ==================== 绘制链路节点选择面板 ====================
void _flowgraph_draw_chain_panel(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, FLOAT canvasWidth, FLOAT canvasHeight)
{
	if (pData->nodeCount == 0) return;

	INT visibleCount = __min(pData->nodeCount, FLOWGRAPH_CHAIN_PANEL_MAX_VISIBLE);
	FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
	FLOAT panelY = (FLOAT)(pData->sidebarChainPanelMode == 0 ? FLOWGRAPH_SIDEBAR_BTN_Y1 : FLOWGRAPH_SIDEBAR_BTN_Y2); // ★ 原Y3/Y4
	FLOAT panelW = FLOWGRAPH_CHAIN_PANEL_WIDTH;
	FLOAT panelH = FLOWGRAPH_CHAIN_PANEL_HEADER + visibleCount * FLOWGRAPH_CHAIN_PANEL_ITEM_H + 4;

	// 阴影
	HEXBRUSH hBrushShadow = _brush_create(ExARGB(0, 0, 0, 50));
	_canvas_fillrect(hCanvas, hBrushShadow, panelX + 3, panelY + 3, panelX + panelW + 3, panelY + panelH + 3);
	_brush_destroy(hBrushShadow);

	// 背景
	HEXBRUSH hBrushPanelBg = _brush_create(ExARGB(28, 28, 36, 248));
	_canvas_fillrect(hCanvas, hBrushPanelBg, panelX, panelY, panelX + panelW, panelY + panelH);
	_brush_destroy(hBrushPanelBg);

	// 边框
	HEXBRUSH hBrushBorder = _brush_create(ExARGB(65, 65, 80, 255));
	_canvas_drawrect(hCanvas, hBrushBorder, panelX, panelY, panelX + panelW, panelY + panelH, 1.0f, 0);
	_brush_destroy(hBrushBorder);

	// 标题
	LPCWSTR title = (pData->sidebarChainPanelMode == 0) ? L"链路从头 - 选择节点" : L"链路继续 - 选择节点";
	HEXFONT hFontTitle = _font_createfromfamily(L"微软雅黑", 10, 0);
	_canvas_drawtext(hCanvas, hFontTitle, ExARGB(120, 120, 135, 255), title, -1,
		DT_LEFT | DT_VCENTER, panelX + 8, panelY + 2, panelX + panelW - 8, panelY + FLOWGRAPH_CHAIN_PANEL_HEADER);
	_font_destroy(hFontTitle);

	// 节点列表
	FLOAT itemStartY = panelY + FLOWGRAPH_CHAIN_PANEL_HEADER;
	FLOAT itemH = FLOWGRAPH_CHAIN_PANEL_ITEM_H;

	for (INT i = 0; i < visibleCount; i++) {
		EX_FLOWGRAPH_NODE* node = &pData->nodes[i];
		FLOAT iy = itemStartY + i * itemH;
		BOOL isHover = (pData->sidebarChainPanelHover == i);

		if (isHover) {
			HEXBRUSH hBrushItemHover = _brush_create(ExARGB(50, 50, 65, 200));
			_canvas_fillrect(hCanvas, hBrushItemHover, panelX + 1, iy + 1, panelX + panelW - 1, iy + itemH - 1);
			_brush_destroy(hBrushItemHover);
		}

		EXARGB tagColor = ExARGB(100, 100, 100, 255);
		EX_FLOWGRAPH_CARD_DESCRIPTOR* desc = _flowgraph_find_card_descriptor(pData, node->cardType);
		if (desc) tagColor = desc->tagColor;
		
		HEXBRUSH hBrushColor = _brush_create(tagColor);
		_canvas_fillrect(hCanvas, hBrushColor, panelX + 8, iy + (itemH - 14) / 2, panelX + 12, iy + (itemH + 14) / 2);
		_brush_destroy(hBrushColor);

		EXARGB statusColor = _flowgraph_get_status_color(node->executionStatus);
		HEXBRUSH hBrushStatus = _brush_create(statusColor);
		_canvas_fillellipse(hCanvas, hBrushStatus, panelX + 20, iy + itemH / 2, 3.5f, 3.5f);
		_brush_destroy(hBrushStatus);

		WCHAR nodeText[256];
		swprintf_s(nodeText, 256, L"%s(Id:%d)", node->title, node->id);
		HEXFONT hFont = _font_createfromfamily(L"微软雅黑", 11, 0);
		EXARGB textColor = isHover ? ExARGB(240, 240, 245, 255) : ExARGB(185, 185, 200, 255);
		_canvas_drawtext(hCanvas, hFont, textColor, nodeText, -1,
			DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS, panelX + 28, iy, panelX + panelW - 10, iy + itemH);
		_font_destroy(hFont);
	}

	if (pData->nodeCount > FLOWGRAPH_CHAIN_PANEL_MAX_VISIBLE) {
		WCHAR moreText[64];
		swprintf_s(moreText, 64, L"...还有%d个节点", pData->nodeCount - FLOWGRAPH_CHAIN_PANEL_MAX_VISIBLE);
		HEXFONT hFontMore = _font_createfromfamily(L"微软雅黑", 9, 0);
		_canvas_drawtext(hCanvas, hFontMore, ExARGB(100, 100, 115, 255), moreText, -1,
			DT_LEFT | DT_VCENTER, panelX + 8, panelY + panelH - 18, panelX + panelW - 8, panelY + panelH - 2);
		_font_destroy(hFontMore);
	}
}

// ==================== 通用：增加动态端口 ====================
INT _flowgraph_add_dynamic_port(HEXOBJ hObj, INT nodeId)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData) return 0;
	EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, nodeId);
	if (!node) return 0;

	EX_FLOWGRAPH_CARD_DESCRIPTOR* desc = _flowgraph_find_card_descriptor(pData, node->cardType);
	if (!desc || desc->dynamicPortBaseIndex < 0) return 0;

	if (node->dynamicPortCount >= desc->dynamicPortMaxCount) return 0;

	// 在动态端口区域的末尾追加，即 baseIndex + currentCount
	INT insertIdx = desc->dynamicPortBaseIndex + node->dynamicPortCount;

	INT newPortCount = node->portCount + 1;
	EX_FLOWGRAPH_PORT* newPorts = (EX_FLOWGRAPH_PORT*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_PORT) * newPortCount);

	// 拷贝插入位置之前的端口
	if (insertIdx > 0) memcpy(newPorts, node->ports, sizeof(EX_FLOWGRAPH_PORT) * insertIdx);
	// 插入位置及之后的端口后移一位
	memcpy(newPorts + insertIdx + 1, node->ports + insertIdx, sizeof(EX_FLOWGRAPH_PORT) * (node->portCount - insertIdx));

	// 初始化新增动态端口
	WCHAR name[32];
	swprintf_s(name, 32, L"%s%d", desc->dynamicPortNamePrefix, node->dynamicPortCount);
	pData->nextAutoId++;
	newPorts[insertIdx].id = pData->nextAutoId;
	newPorts[insertIdx].portType = FLOWGRAPH_PORTTYPE_INPUT;
	newPorts[insertIdx].dataType = desc->dynamicPortDataType;
	newPorts[insertIdx].name = StrDupW(name);
	newPorts[insertIdx].widgetType = 0;
	newPorts[insertIdx].widgetId = 0;
	newPorts[insertIdx].widgetWidth = 0;
	newPorts[insertIdx].widgetHeight = 0;
	memset(&newPorts[insertIdx].widgetRect, 0, sizeof(RECT));
	memset(&newPorts[insertIdx].portRect, 0, sizeof(RECT));
	newPorts[insertIdx].widgetData = NULL;
	newPorts[insertIdx].isConnected = FALSE;
	newPorts[insertIdx].imagePath = NULL;
	Ex_MemFree(node->ports);
	node->ports = newPorts;
	node->portCount = newPortCount;
	node->dynamicPortCount++; // ★ 更新动态端口计数

	// 更新插入位置之后端口的连线索引
	for (INT i = 0; i < pData->connectionCount; i++) {
		if (pData->connections[i].fromNode == nodeId && pData->connections[i].fromSlot >= insertIdx)
			pData->connections[i].fromSlot++;
		if (pData->connections[i].toNode == nodeId && pData->connections[i].toSlot >= insertIdx)
			pData->connections[i].toSlot++;
	}

	node->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
	_flowgraph_invalidate_downstream(pData, node->id);

	_flowgraph_calcnodesize(hObj, node);
	_flowgraph_updatelayout(hObj);
	Ex_ObjInvalidateRect(hObj, 0);
	return 1;
}

// ==================== 通用：减少动态端口 ====================
INT _flowgraph_remove_dynamic_port(HEXOBJ hObj, INT nodeId)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData) return 0;
	EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, nodeId);
	if (!node) return 0;

	EX_FLOWGRAPH_CARD_DESCRIPTOR* desc = _flowgraph_find_card_descriptor(pData, node->cardType);
	if (!desc || desc->dynamicPortBaseIndex < 0) return 0;

	if (node->dynamicPortCount <= desc->dynamicPortMinCount) return 0;

	// 移除动态端口区域的最后一个
	INT removeIdx = desc->dynamicPortBaseIndex + node->dynamicPortCount - 1;

	// 先删除连接到该端口的连线
	for (INT i = pData->connectionCount - 1; i >= 0; i--) {
		EX_FLOWGRAPH_CONNECTION* conn = &pData->connections[i];
		if ((conn->toNode == nodeId && conn->toSlot == removeIdx) ||
			(conn->fromNode == nodeId && conn->fromSlot == removeIdx)) {
			_flowgraph_removeconnection(hObj, conn->id);
		}
	}

	// 更新被删除端口之后端口的连线索引
	for (INT i = 0; i < pData->connectionCount; i++) {
		if (pData->connections[i].fromNode == nodeId && pData->connections[i].fromSlot > removeIdx)
			pData->connections[i].fromSlot--;
		if (pData->connections[i].toNode == nodeId && pData->connections[i].toSlot > removeIdx)
			pData->connections[i].toSlot--;
	}

	// 释放被删除端口的资源
	Ex_MemFree((void*)node->ports[removeIdx].name);
	if (node->ports[removeIdx].widgetData) {
		if (node->ports[removeIdx].dataType == FLOWGRAPH_DATATYPE_IMAGE)
		{
			_img_destroy((HEXIMAGE)node->ports[removeIdx].widgetData);
			if (node->ports[removeIdx].imagePath) Ex_MemFree((void*)node->ports[removeIdx].imagePath);
		}
		else if (node->ports[removeIdx].dataType == FLOWGRAPH_DATATYPE_STRING)
			Ex_MemFree(node->ports[removeIdx].widgetData);
	}

	// 内存前移，缩小端口数组
	for (INT i = removeIdx; i < node->portCount - 1; i++) {
		node->ports[i] = node->ports[i + 1];
	}
	node->portCount--;
	node->dynamicPortCount--; // ★ 更新动态端口计数

	EX_FLOWGRAPH_PORT* newPorts = (EX_FLOWGRAPH_PORT*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_PORT) * node->portCount);
	memcpy(newPorts, node->ports, sizeof(EX_FLOWGRAPH_PORT) * node->portCount);
	Ex_MemFree(node->ports);
	node->ports = newPorts;

	node->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
	_flowgraph_invalidate_downstream(pData, node->id);

	_flowgraph_calcnodesize(hObj, node);
	_flowgraph_updatelayout(hObj);
	Ex_ObjInvalidateRect(hObj, 0);
	return 1;
}


// ==================== 释放临时节点数据(创建函数用) ====================
void _flowgraph_free_temp_node_data(EX_FLOWGRAPH_NODE* node)
{
	if (!node || !node->ports) return;
	for (INT i = 0; i < node->portCount; i++) {
		// 释放端口名称(创建函数中统一使用StrDupW分配)
		if (node->ports[i].name) { Ex_MemFree((void*)node->ports[i].name); node->ports[i].name = NULL; }
		// 释放子组件数据
		if (node->ports[i].widgetData) {
			switch (node->ports[i].widgetType) {
			case FLOWGRAPH_NODEDATA_TYPE_EDIT:
			case FLOWGRAPH_NODEDATA_TYPE_TEXT:
				Ex_MemFree(node->ports[i].widgetData);
				break;
			case FLOWGRAPH_NODEDATA_TYPE_COMBO: {
				EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)node->ports[i].widgetData;
				for (INT k = 0; k < combo->count; k++) Ex_MemFree((void*)combo->options[k]);
				Ex_MemFree(combo->options);
				Ex_MemFree(combo);
				break;
			}
			case FLOWGRAPH_NODEDATA_TYPE_BUTTON: {
				EX_FLOWGRAPH_NODE_BUTTON_DATA* btn = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)node->ports[i].widgetData;
				Ex_MemFree((void*)btn->caption);
				Ex_MemFree(btn);
				break;
			}
			case FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON: {
				EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* dualBtn = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)node->ports[i].widgetData;
				Ex_MemFree((void*)dualBtn->caption1);
				Ex_MemFree((void*)dualBtn->caption2);
				Ex_MemFree(dualBtn);
				break;
			}
			case FLOWGRAPH_NODEDATA_TYPE_IMAGE:
				if (node->ports[i].imagePath) Ex_MemFree((void*)node->ports[i].imagePath);
				break;
			case FLOWGRAPH_NODEDATA_TYPE_VIDEO:
				// temp节点中videoData为NULL，无需释放
				break;
			default:
				break;
			}
			node->ports[i].widgetData = NULL;
		}
	}
	Ex_MemFree(node->ports);
	node->ports = NULL;
	// 注意: node->title 由调用者所有，不释放
}




// ==================== 创建视频控件数据 ====================
EX_FLOWGRAPH_NODE_VIDEO_DATA* _flowgraph_create_video_data(libvlc_instance_t* libVlc, HEXOBJ hFlowGraphObj)
{
	EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE_VIDEO_DATA));
	memset(pVideo, 0, sizeof(EX_FLOWGRAPH_NODE_VIDEO_DATA));
	InitializeCriticalSection(&pVideo->critsec);
	pVideo->libVlc = libVlc;
	pVideo->hFlowGraphObj = hFlowGraphObj;
	pVideo->nVolume = 100;
	pVideo->nCurrentTime = 0;
	pVideo->nDuration = 0;
	pVideo->videoPath = NULL;
	return pVideo;
}

// ==================== 视频数据清理 ====================
void _flowgraph_video_cleanup(EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo)
{
	if (!pVideo) return;

	if (pVideo->mediaPlayer) {
		libvlc_media_player_stop(pVideo->mediaPlayer);
		libvlc_media_player_release(pVideo->mediaPlayer);
		pVideo->mediaPlayer = NULL;
	}
	if (pVideo->pixelBuff) {
		free(pVideo->pixelBuff);
		pVideo->pixelBuff = NULL;
	}
	EnterCriticalSection(&pVideo->critsec);
	if (pVideo->hCurrentFrame) { _img_destroy(pVideo->hCurrentFrame); pVideo->hCurrentFrame = NULL; }
	if (pVideo->hCoverImg) { _img_destroy(pVideo->hCoverImg); pVideo->hCoverImg = NULL; }
	if (pVideo->hPendingFree) { _img_destroy(pVideo->hPendingFree); pVideo->hPendingFree = NULL; }
	LeaveCriticalSection(&pVideo->critsec);

	pVideo->bHasCover = FALSE;
	pVideo->bIsPlaying = FALSE;
	pVideo->bIsPaused = FALSE;
	pVideo->bIsLoaded = FALSE;
	pVideo->bLoadOnly = FALSE;
	pVideo->nCurrentTime = 0;
	pVideo->nDuration = 0;
	if (pVideo->videoPath) { Ex_MemFree((void*)pVideo->videoPath); pVideo->videoPath = NULL; }
}

// ==================== 视频加载 ====================
void _flowgraph_video_load(EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo, LPCWSTR filePath, BOOL bLoadOnly)
{
	if (!pVideo || !pVideo->libVlc || !filePath) return;
	_flowgraph_video_cleanup(pVideo);
	pVideo->bLoadOnly = bLoadOnly;
	if (pVideo->videoPath) Ex_MemFree((void*)pVideo->videoPath);
	pVideo->videoPath = StrDupW(filePath);
	std::string utf8Path = Ex_W2U(filePath);
	std::replace(utf8Path.begin(), utf8Path.end(), '\\', '/');
	std::string uri = "file:///" + utf8Path;

	// 优先使用 location (URI)，兼容性更强
	libvlc_media_t* m = libvlc_media_new_location(pVideo->libVlc, uri.c_str());
	if (!m) {
		// 兜底：如果 URI 失败，回退到传统的 path
		m = libvlc_media_new_path(pVideo->libVlc, utf8Path.c_str());
	}

	if (!m) return;
	pVideo->mediaPlayer = libvlc_media_player_new_from_media(m);
	libvlc_media_release(m);
	libvlc_video_set_format_callbacks(pVideo->mediaPlayer, _flowgraph_video_format_cb, NULL);
	libvlc_video_set_callbacks(pVideo->mediaPlayer, _flowgraph_video_lock_cb, _flowgraph_video_unlock_cb, _flowgraph_video_display_cb, pVideo);
	libvlc_event_attach(libvlc_media_player_event_manager(pVideo->mediaPlayer), libvlc_MediaPlayerPlaying, _flowgraph_video_event_cb, pVideo);
	libvlc_event_attach(libvlc_media_player_event_manager(pVideo->mediaPlayer), libvlc_MediaPlayerPaused, _flowgraph_video_event_cb, pVideo);
	libvlc_event_attach(libvlc_media_player_event_manager(pVideo->mediaPlayer), libvlc_MediaPlayerStopped, _flowgraph_video_event_cb, pVideo);
	libvlc_event_attach(libvlc_media_player_event_manager(pVideo->mediaPlayer), libvlc_MediaPlayerEndReached, _flowgraph_video_event_cb, pVideo);
	libvlc_event_attach(libvlc_media_player_event_manager(pVideo->mediaPlayer), libvlc_MediaPlayerLengthChanged, _flowgraph_video_event_cb, pVideo);
	libvlc_event_attach(libvlc_media_player_event_manager(pVideo->mediaPlayer), libvlc_MediaPlayerEncounteredError, _flowgraph_video_event_cb, pVideo);
	libvlc_audio_set_volume(pVideo->mediaPlayer, pVideo->nVolume);
	libvlc_media_player_play(pVideo->mediaPlayer);
}

// ==================== 视频VLC回调 ====================
unsigned int _flowgraph_video_format_cb(void** object, char* chroma, unsigned int* width, unsigned int* height, unsigned int* pitches, unsigned int* lines) {
	EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)*object;
	strcpy(chroma, "BGRA");
	*pitches = *width * 4;
	*lines = *height;
	pVideo->videoWidth = *width;
	pVideo->videoHeight = *height;
	if (pVideo->pixelBuff) free(pVideo->pixelBuff);
	pVideo->pixelBuff = calloc(1, *width * *height * 4 + 1);
	return 1;
}
void* _flowgraph_video_lock_cb(void* object, void** planes) {
	EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)object;
	EnterCriticalSection(&pVideo->critsec);
	*planes = pVideo->pixelBuff;
	return NULL;
}
void _flowgraph_video_unlock_cb(void* object, void* picture, void* const* planes) {
	EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)object;
	HEXIMAGE img = NULL;
	_img_createfrompngbits2(pVideo->videoWidth, pVideo->videoHeight, (BYTE*)*planes, &img);
	if (!pVideo->bHasCover && img) {
		HEXIMAGE hCover = NULL;
		_img_copy(img, &hCover);
		pVideo->hCoverImg = hCover;
		pVideo->bHasCover = TRUE;
	}
	if (pVideo->hPendingFree) _img_destroy(pVideo->hPendingFree);
	pVideo->hPendingFree = pVideo->hCurrentFrame;
	pVideo->hCurrentFrame = img;
	LeaveCriticalSection(&pVideo->critsec);
}
void _flowgraph_video_display_cb(void* object, void* picture) {
	EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)object;
	if (pVideo->hFlowGraphObj)  Ex_ObjPostMessage(pVideo->hFlowGraphObj, FLOWGRAPH_MESSAGE_VIDEO_REFRESH, 0, 0);
}
void _flowgraph_video_event_cb(const libvlc_event_t* event, void* object) {
	EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)object;
	switch (event->type) {
	case libvlc_MediaPlayerPlaying:
		pVideo->bIsPlaying = TRUE;
		pVideo->bIsPaused = FALSE;
		pVideo->bIsLoaded = TRUE;
		pVideo->nDuration = libvlc_media_player_get_length(pVideo->mediaPlayer);
		if (pVideo->bLoadOnly) {
			pVideo->bLoadOnly = FALSE;
			libvlc_media_player_set_pause(pVideo->mediaPlayer, 1);
			return;
		}
		break;
	case libvlc_MediaPlayerEncounteredError:
		// ✅ 新增：捕获 VLC 致命错误（路径错、文件损坏等）
		//OutputDebugStringW(L"[VLC Error] 视频加载失败，请检查路径或文件！\n");
		pVideo->bIsPlaying = FALSE;
		pVideo->bIsLoaded = FALSE;
		break;
	case libvlc_MediaPlayerPaused:
		pVideo->bIsPlaying = FALSE;
		pVideo->bIsPaused = TRUE;
		break;
	case libvlc_MediaPlayerStopped:
		pVideo->bIsPlaying = FALSE;
		pVideo->bIsPaused = FALSE;
		break;
	case libvlc_MediaPlayerEndReached:
		pVideo->bIsPlaying = FALSE;
		pVideo->bIsPaused = FALSE;
		pVideo->bIsLoaded = TRUE;
		break;
	case libvlc_MediaPlayerLengthChanged:
		pVideo->nDuration = libvlc_media_player_get_length(pVideo->mediaPlayer);
		break;
	}
	if (pVideo->hFlowGraphObj) Ex_ObjPostMessage(pVideo->hFlowGraphObj, FLOWGRAPH_MESSAGE_VIDEO_REFRESH, 0, 0);
}

// ==================== 视频时间格式化 ====================
void _flowgraph_format_video_time(INT64 ms, WCHAR* buf, INT bufSize) {
	if (ms < 0) ms = 0;
	INT totalSec = (INT)(ms / 1000);
	INT h = totalSec / 3600;
	INT m = (totalSec % 3600) / 60;
	INT s = totalSec % 60;
	if (h > 0) swprintf_s(buf, bufSize, L"%d:%02d:%02d", h, m, s);
	else swprintf_s(buf, bufSize, L"%02d:%02d", m, s);
}

// ==================== 视频定时器更新 ====================
void _flowgraph_update_video_timers(HEXOBJ hObj, EX_FLOWGRAPH_DATA* pData)
{
	BOOL hasPlaying = FALSE;
	for (INT i = 0; i < pData->nodeCount; i++) {
		EX_FLOWGRAPH_NODE* node = &pData->nodes[i];
		for (INT j = 0; j < node->portCount; j++) {
			if (node->ports[j].widgetType != FLOWGRAPH_NODEDATA_TYPE_VIDEO || !node->ports[j].widgetData) continue;
			EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)node->ports[j].widgetData;
			if (pVideo->mediaPlayer && pVideo->bIsPlaying) {
				INT64 newTime = libvlc_media_player_get_time(pVideo->mediaPlayer);
				if (newTime >= 0) pVideo->nCurrentTime = newTime;
				INT64 newDur = libvlc_media_player_get_length(pVideo->mediaPlayer);
				if (newDur > 0) pVideo->nDuration = newDur;
				hasPlaying = TRUE;
			}
		}
	}
	if (hasPlaying && !pData->videoTimerActive) {
		Ex_ObjSetTimer(hObj, 100);
		pData->videoTimerActive = TRUE;
	}
	else if (!hasPlaying && pData->videoTimerActive) {
		Ex_ObjKillTimer(hObj);
		pData->videoTimerActive = FALSE;
	}

}

// ==================== 视频控件绘制 ====================
void _flowgraph_draw_video_widget(HEXCANVAS hCanvas, EX_FLOWGRAPH_NODE* node,
	EX_FLOWGRAPH_PORT* port, FLOAT zoom, INT scrollX, INT scrollY)
{
	EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)port->widgetData;
	FLOAT wx = (node->x + port->widgetRect.left) * zoom - scrollX;
	FLOAT wy = (node->y + port->widgetRect.top) * zoom - scrollY;
	FLOAT ww = (port->widgetRect.right - port->widgetRect.left) * zoom;
	FLOAT wh = (port->widgetRect.bottom - port->widgetRect.top) * zoom;
	FLOAT ctrlH = FLOWGRAPH_VIDEO_CTRL_HEIGHT * zoom;
	FLOAT progressH = FLOWGRAPH_VIDEO_PROGRESS_H * zoom;
	FLOAT videoH = wh - ctrlH;
	// 1. 绘制视频帧/封面/黑底
	if (pVideo) {
		EnterCriticalSection(&pVideo->critsec);
		HEXIMAGE hDrawImg = (pVideo->bIsPlaying || pVideo->bIsPaused) ? pVideo->hCurrentFrame : pVideo->hCoverImg;
		if (hDrawImg > 0) {
			INT imgW, imgH;
			_img_getsize(hDrawImg, &imgW, &imgH);
			_canvas_drawimagerectrect(hCanvas, hDrawImg, wx, wy, wx + ww, wy + videoH, 0, 0, imgW, imgH, 255);
		}
		else {
			HEXBRUSH hBrushBg = _brush_create(ExARGB(0, 0, 0, 255));
			_canvas_fillrect(hCanvas, hBrushBg, wx, wy, wx + ww, wy + videoH);
			_brush_destroy(hBrushBg);
		}
		if (pVideo->hPendingFree) { _img_destroy(pVideo->hPendingFree); pVideo->hPendingFree = NULL; }
		LeaveCriticalSection(&pVideo->critsec);
	}
	else {
		HEXBRUSH hBrushBg = _brush_create(ExARGB(0, 0, 0, 255));
		_canvas_fillrect(hCanvas, hBrushBg, wx, wy, wx + ww, wy + videoH);
		_brush_destroy(hBrushBg);
		// "无视频" 提示
		HEXFONT hFont = _font_createfromfamily(L"微软雅黑", 11 * zoom, 0);
		_canvas_drawtext(hCanvas, hFont, ExARGB(100, 100, 100, 255), L"无视频", -1,
			DT_CENTER | DT_VCENTER, wx, wy, wx + ww, wy + videoH);
		_font_destroy(hFont);
	}
	// 2. 非播放状态时绘制居中播放按钮
	if (!pVideo || !pVideo->bIsPlaying) {
		FLOAT cx = wx + ww / 2.0f;
		FLOAT cy = wy + videoH / 2.0f;
		FLOAT r = 14.0f * zoom;
		HEXBRUSH hBrushOverlay = _brush_create(ExARGB(0, 0, 0, 120));
		_canvas_fillellipse(hCanvas, hBrushOverlay, cx, cy, r, r);
		HEXBRUSH hBrushPlay = _brush_create(ExARGB(255, 255, 255, 200));
		POINTF pts[3] = { {cx - r * 0.3f, cy - r * 0.5f}, {cx - r * 0.3f, cy + r * 0.5f}, {cx + r * 0.5f, cy} };
		HEXPATH hPath; _path_create(PATH_FLAG_DISABLESCALE, &hPath); _path_open(hPath);
		_path_beginfigure2(hPath, pts[0].x, pts[0].y);
		_path_addline(hPath, pts[0].x, pts[0].y, pts[1].x, pts[1].y);
		_path_addline(hPath, pts[1].x, pts[1].y, pts[2].x, pts[2].y);
		_path_endfigure(hPath, TRUE); _path_close(hPath);
		_canvas_fillpath(hCanvas, hPath, hBrushPlay);
		_path_destroy(hPath);
		_brush_destroy(hBrushPlay); _brush_destroy(hBrushOverlay);
	}
	// 3. 控制栏背景
	HEXBRUSH hBrushCtrlBg = _brush_create(ExARGB(20, 20, 25, 220));
	_canvas_fillrect(hCanvas, hBrushCtrlBg, wx, wy + videoH, wx + ww, wy + wh);
	_brush_destroy(hBrushCtrlBg);
	// 4. 进度条
	FLOAT progressY = wy + videoH;
	FLOAT progressPos = 0.0f;
	if (pVideo && pVideo->nDuration > 0 && pVideo->nCurrentTime > 0) {
		progressPos = (FLOAT)pVideo->nCurrentTime / (FLOAT)pVideo->nDuration * ww;
	}
	HEXBRUSH hBrushTrack = _brush_create(ExARGB(60, 60, 70, 255));
	HEXBRUSH hBrushFill = _brush_create(ExARGB(0, 150, 255, 255));
	_canvas_fillrect(hCanvas, hBrushTrack, wx, progressY, wx + ww, progressY + progressH);
	_canvas_fillrect(hCanvas, hBrushFill, wx, progressY, wx + progressPos, progressY + progressH);
	_brush_destroy(hBrushTrack); _brush_destroy(hBrushFill);
	// 5. 播放/暂停按钮 + 时间文本
	FLOAT btnY = wy + videoH + progressH + (ctrlH - progressH) / 2.0f;
	HEXBRUSH hBrushIcon = _brush_create(ExARGB(200, 200, 200, 255));
	FLOAT playX = wx + 14.0f * zoom;
	FLOAT btnR = 5.0f * zoom;
	if (pVideo && pVideo->bIsPlaying) {
		// 暂停图标
		_canvas_drawline(hCanvas, hBrushIcon, playX - 3 * zoom, btnY - btnR, playX - 3 * zoom, btnY + btnR, 1.5f * zoom, 0);
		_canvas_drawline(hCanvas, hBrushIcon, playX + 3 * zoom, btnY - btnR, playX + 3 * zoom, btnY + btnR, 1.5f * zoom, 0);
	}
	else {
		// 播放图标
		POINTF pts[3] = { {playX - 4 * zoom, btnY - btnR}, {playX - 4 * zoom, btnY + btnR}, {playX + 5 * zoom, btnY} };
		HEXPATH hPath; _path_create(PATH_FLAG_DISABLESCALE, &hPath); _path_open(hPath);
		_path_beginfigure2(hPath, pts[0].x, pts[0].y);
		_path_addline(hPath, pts[0].x, pts[0].y, pts[1].x, pts[1].y);
		_path_addline(hPath, pts[1].x, pts[1].y, pts[2].x, pts[2].y);
		_path_endfigure(hPath, TRUE); _path_close(hPath);
		_canvas_fillpath(hCanvas, hPath, hBrushIcon);
		_path_destroy(hPath);
	}
	// 时间文本
	if (pVideo) {
		WCHAR timeStr[64], curStr[32], durStr[32];
		_flowgraph_format_video_time(pVideo->nCurrentTime, curStr, 32);
		_flowgraph_format_video_time(pVideo->nDuration, durStr, 32);
		swprintf_s(timeStr, L"%s/%s", curStr, durStr);
		HEXFONT hFont = _font_createfromfamily(L"微软雅黑", 9 * zoom, 0);
		_canvas_drawtext(hCanvas, hFont, ExARGB(180, 180, 180, 255), timeStr, -1,
			DT_RIGHT | DT_VCENTER, wx + 4 * zoom, wy + videoH + progressH, wx + ww - 4 * zoom, wy + wh);
		_font_destroy(hFont);
	}
	_brush_destroy(hBrushIcon);
}

// ==================== 绘制自定义面板 ====================
void _flowgraph_draw_custom_panel(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, FLOAT canvasWidth, FLOAT canvasHeight)
{
	if (pData->customItemCount == 0) {
		// 无条目时显示空面板
		FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
		FLOAT panelY = (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y6;
		FLOAT panelW = FLOWGRAPH_CUSTOM_PANEL_WIDTH;
		FLOAT panelH = FLOWGRAPH_CUSTOM_PANEL_HEADER + 36;

		// 阴影
		HEXBRUSH hBrushShadow = _brush_create(ExARGB(0, 0, 0, 50));
		_canvas_fillrect(hCanvas, hBrushShadow, panelX + 3, panelY + 3, panelX + panelW + 3, panelY + panelH + 3);
		_brush_destroy(hBrushShadow);

		// 背景
		HEXBRUSH hBrushPanelBg = _brush_create(ExARGB(28, 28, 36, 248));
		_canvas_fillrect(hCanvas, hBrushPanelBg, panelX, panelY, panelX + panelW, panelY + panelH);
		_brush_destroy(hBrushPanelBg);

		// 边框
		HEXBRUSH hBrushBorder = _brush_create(ExARGB(65, 65, 80, 255));
		_canvas_drawrect(hCanvas, hBrushBorder, panelX, panelY, panelX + panelW, panelY + panelH, 1.0f, 0);
		_brush_destroy(hBrushBorder);

		// 标题
		HEXFONT hFontTitle = _font_createfromfamily(L"微软雅黑", 10, 0);
		_canvas_drawtext(hCanvas, hFontTitle, ExARGB(120, 120, 135, 255), L"自定义", -1,
			DT_LEFT | DT_VCENTER, panelX + 10, panelY + 2, panelX + panelW - 10, panelY + FLOWGRAPH_CUSTOM_PANEL_HEADER);
		_font_destroy(hFontTitle);

		// 空提示
		HEXFONT hFontEmpty = _font_createfromfamily(L"微软雅黑", 11, 0);
		_canvas_drawtext(hCanvas, hFontEmpty, ExARGB(90, 90, 105, 255), L"暂无条目", -1,
			DT_CENTER | DT_VCENTER, panelX + 4, panelY + FLOWGRAPH_CUSTOM_PANEL_HEADER, panelX + panelW - 4, panelY + panelH - 4);
		_font_destroy(hFontEmpty);
		return;
	}

	INT visibleCount = __min(pData->customItemCount, FLOWGRAPH_CUSTOM_PANEL_MAX_VISIBLE);
	FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
	FLOAT panelY = (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y6;
	FLOAT panelW = FLOWGRAPH_CUSTOM_PANEL_WIDTH;
	FLOAT panelH = FLOWGRAPH_CUSTOM_PANEL_HEADER + visibleCount * FLOWGRAPH_CUSTOM_PANEL_ITEM_H + 4;

	// 阴影
	HEXBRUSH hBrushShadow = _brush_create(ExARGB(0, 0, 0, 50));
	_canvas_fillrect(hCanvas, hBrushShadow, panelX + 3, panelY + 3, panelX + panelW + 3, panelY + panelH + 3);
	_brush_destroy(hBrushShadow);

	// 背景
	HEXBRUSH hBrushPanelBg = _brush_create(ExARGB(28, 28, 36, 248));
	_canvas_fillrect(hCanvas, hBrushPanelBg, panelX, panelY, panelX + panelW, panelY + panelH);
	_brush_destroy(hBrushPanelBg);

	// 边框
	HEXBRUSH hBrushBorder = _brush_create(ExARGB(65, 65, 80, 255));
	_canvas_drawrect(hCanvas, hBrushBorder, panelX, panelY, panelX + panelW, panelY + panelH, 1.0f, 0);
	_brush_destroy(hBrushBorder);

	// 标题
	HEXFONT hFontTitle = _font_createfromfamily(L"微软雅黑", 10, 0);
	_canvas_drawtext(hCanvas, hFontTitle, ExARGB(120, 120, 135, 255), L"自定义", -1,
		DT_LEFT | DT_VCENTER, panelX + 10, panelY + 2, panelX + panelW - 10, panelY + FLOWGRAPH_CUSTOM_PANEL_HEADER);
	_font_destroy(hFontTitle);

	// 条目列表
	FLOAT itemStartY = panelY + FLOWGRAPH_CUSTOM_PANEL_HEADER;
	FLOAT itemH = FLOWGRAPH_CUSTOM_PANEL_ITEM_H;

	for (INT i = 0; i < visibleCount; i++) {
		FLOAT iy = itemStartY + i * itemH;
		BOOL isHover = (pData->sidebarCustomPanelHover == i);

		if (isHover) {
			HEXBRUSH hBrushItemHover = _brush_create(ExARGB(50, 50, 65, 200));
			_canvas_fillrect(hCanvas, hBrushItemHover, panelX + 1, iy + 1, panelX + panelW - 1, iy + itemH - 1);
			_brush_destroy(hBrushItemHover);
		}

		// 左侧橙色小条
		HEXBRUSH hBrushColor = _brush_create(ExARGB(220, 160, 60, 255));
		_canvas_fillrect(hCanvas, hBrushColor, panelX + 8, iy + (itemH - 14) / 2, panelX + 12, iy + (itemH + 14) / 2);
		_brush_destroy(hBrushColor);

		// 条目文字
		HEXFONT hFont = _font_createfromfamily(L"微软雅黑", 11, 0);
		EXARGB textColor = isHover ? ExARGB(240, 240, 245, 255) : ExARGB(185, 185, 200, 255);
		_canvas_drawtext(hCanvas, hFont, textColor, pData->customItems[i], -1,
			DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS, panelX + 20, iy, panelX + panelW - 12, iy + itemH);
		_font_destroy(hFont);
	}

	// 超出最大显示数量提示
	if (pData->customItemCount > FLOWGRAPH_CUSTOM_PANEL_MAX_VISIBLE) {
		WCHAR moreText[64];
		swprintf_s(moreText, 64, L"...还有%d项", pData->customItemCount - FLOWGRAPH_CUSTOM_PANEL_MAX_VISIBLE);
		HEXFONT hFontMore = _font_createfromfamily(L"微软雅黑", 9, 0);
		_canvas_drawtext(hCanvas, hFontMore, ExARGB(100, 100, 115, 255), moreText, -1,
			DT_LEFT | DT_VCENTER, panelX + 8, panelY + panelH - 18, panelX + panelW - 8, panelY + panelH - 2);
		_font_destroy(hFontMore);
	}
}

// ==================== 设置自定义条目 ====================
INT _flowgraph_set_custom_items(HEXOBJ hObj, EX_FLOWGRAPH_CUSTOM_ITEMS* pItems)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData || !pItems) return 0;

	// 释放旧条目
	if (pData->customItems) {
		for (INT i = 0; i < pData->customItemCount; i++) {
			Ex_MemFree((void*)pData->customItems[i]);
		}
		Ex_MemFree(pData->customItems);
		pData->customItems = NULL;
		pData->customItemCount = 0;
	}

	// 拷贝新条目
	if (pItems->count > 0 && pItems->items) {
		pData->customItemCount = pItems->count;
		pData->customItems = (LPCWSTR*)Ex_MemAlloc(sizeof(LPCWSTR) * pItems->count);
		for (INT i = 0; i < pItems->count; i++) {
			pData->customItems[i] = StrDupW(pItems->items[i]);
		}
	}

	pData->sidebarCustomPanelHover = -1;
	Ex_ObjInvalidateRect(hObj, 0);
	return 1;
}

// ==================== 深拷贝组件数据 ====================
LPVOID _flowgraph_copy_widget_data(INT widgetType, LPVOID srcData)
{
	if (!srcData) return NULL;
	if (widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT || widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT) {
		return (LPVOID)StrDupW((LPCWSTR)srcData);
	}
	else if (widgetType == FLOWGRAPH_NODEDATA_TYPE_COMBO) {
		EX_FLOWGRAPH_NODE_COMBO_DATA* src = (EX_FLOWGRAPH_NODE_COMBO_DATA*)srcData;
		EX_FLOWGRAPH_NODE_COMBO_DATA* dst = (EX_FLOWGRAPH_NODE_COMBO_DATA*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE_COMBO_DATA));
		dst->count = src->count; dst->current = src->current;
		dst->options = (LPCWSTR*)Ex_MemAlloc(sizeof(LPCWSTR) * src->count);
		for (INT k = 0; k < src->count; k++) dst->options[k] = StrDupW(src->options[k]);
		return (LPVOID)dst;
	}
	else if (widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON) {
		EX_FLOWGRAPH_NODE_BUTTON_DATA* src = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)srcData;
		EX_FLOWGRAPH_NODE_BUTTON_DATA* dst = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE_BUTTON_DATA));
		dst->caption = StrDupW(src->caption);
		return (LPVOID)dst;
	}
	else if (widgetType == FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON) {
		EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* src = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)srcData;
		EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* dst = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA));
		dst->caption1 = StrDupW(src->caption1);
		dst->caption2 = StrDupW(src->caption2);
		return (LPVOID)dst;
	}
	// IMAGE/VIDEO 等类型不从模板拷贝
	return NULL;
}

// ==================== 释放卡片描述符深拷贝数据 ====================
void _flowgraph_free_card_descriptor(EX_FLOWGRAPH_CARD_DESCRIPTOR* desc)
{
	if (!desc) return;
	if (desc->typeName) { Ex_MemFree((void*)desc->typeName); desc->typeName = NULL; }
	if (desc->dynamicPortNamePrefix) { Ex_MemFree((void*)desc->dynamicPortNamePrefix); desc->dynamicPortNamePrefix = NULL; }
	if (desc->dynamicPort2NamePrefix) { Ex_MemFree((void*)desc->dynamicPort2NamePrefix); desc->dynamicPort2NamePrefix = NULL; } // ★ 新增

	if (desc->ports) {
		for (INT i = 0; i < desc->portCount; i++) {
			EX_FLOWGRAPH_PORT_DESC* port = &desc->ports[i];
			if (port->name) { Ex_MemFree((void*)port->name); port->name = NULL; }
			if (port->defaultWidgetData) {
				switch (port->widgetType) {
				case FLOWGRAPH_NODEDATA_TYPE_EDIT:
				case FLOWGRAPH_NODEDATA_TYPE_TEXT:
					Ex_MemFree(port->defaultWidgetData); break;
				case FLOWGRAPH_NODEDATA_TYPE_COMBO: {
					EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)port->defaultWidgetData;
					for (INT k = 0; k < combo->count; k++) Ex_MemFree((void*)combo->options[k]);
					Ex_MemFree(combo->options); Ex_MemFree(combo); break;
				}
				case FLOWGRAPH_NODEDATA_TYPE_BUTTON: {
					EX_FLOWGRAPH_NODE_BUTTON_DATA* btn = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)port->defaultWidgetData;
					Ex_MemFree((void*)btn->caption); Ex_MemFree(btn); break;
				}
				case FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON: {
					EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* dual = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)port->defaultWidgetData;
					Ex_MemFree((void*)dual->caption1); Ex_MemFree((void*)dual->caption2);
					Ex_MemFree(dual); break;
				}
				}
				port->defaultWidgetData = NULL;
			}
		}
		Ex_MemFree(desc->ports); desc->ports = NULL;
	}
}

// ==================== 查找已注册的卡片描述符 ====================
EX_FLOWGRAPH_CARD_DESCRIPTOR* _flowgraph_find_card_descriptor(EX_FLOWGRAPH_DATA* pData, INT cardType)
{
	for (INT i = 0; i < pData->cardRegistryCount; i++) {
		if (pData->cardRegistry[i].cardType == cardType)
			return &pData->cardRegistry[i];
	}
	return NULL;
}

// ==================== 注册卡片类型 ====================
INT _flowgraph_register_card_type(HEXOBJ hObj, EX_FLOWGRAPH_CARD_DESCRIPTOR* pDesc)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData || !pDesc) return 0;
	if (pDesc->cardType < 1) return 0;

	// 已存在则替换
	for (INT i = 0; i < pData->cardRegistryCount; i++) {
		if (pData->cardRegistry[i].cardType == pDesc->cardType) {
			_flowgraph_free_card_descriptor(&pData->cardRegistry[i]);
			pData->cardRegistry[i].cardType = pDesc->cardType;
			pData->cardRegistry[i].typeName = StrDupW(pDesc->typeName);
			pData->cardRegistry[i].tagColor = pDesc->tagColor;
			pData->cardRegistry[i].portCount = pDesc->portCount;
			pData->cardRegistry[i].ports = (EX_FLOWGRAPH_PORT_DESC*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_PORT_DESC) * pDesc->portCount);
			for (INT j = 0; j < pDesc->portCount; j++) {
				pData->cardRegistry[i].ports[j] = pDesc->ports[j];
				pData->cardRegistry[i].ports[j].name = StrDupW(pDesc->ports[j].name);
				pData->cardRegistry[i].ports[j].defaultWidgetData = _flowgraph_copy_widget_data(pDesc->ports[j].widgetType, pDesc->ports[j].defaultWidgetData);
			}
			// 第一组动态端口
			pData->cardRegistry[i].dynamicPortBaseIndex = pDesc->dynamicPortBaseIndex;
			pData->cardRegistry[i].dynamicPortMinCount = pDesc->dynamicPortMinCount;
			pData->cardRegistry[i].dynamicPortMaxCount = pDesc->dynamicPortMaxCount;
			pData->cardRegistry[i].dynamicPortDataType = pDesc->dynamicPortDataType;
			pData->cardRegistry[i].dynamicPortNamePrefix = pDesc->dynamicPortNamePrefix ? StrDupW(pDesc->dynamicPortNamePrefix) : NULL;
			// ★ 第二组动态端口 (音频)
			pData->cardRegistry[i].dynamicPort2BaseIndex = pDesc->dynamicPort2BaseIndex;
			pData->cardRegistry[i].dynamicPort2MinCount = pDesc->dynamicPort2MinCount;
			pData->cardRegistry[i].dynamicPort2MaxCount = pDesc->dynamicPort2MaxCount;
			pData->cardRegistry[i].dynamicPort2DataType = pDesc->dynamicPort2DataType;
			pData->cardRegistry[i].dynamicPort2NamePrefix = pDesc->dynamicPort2NamePrefix ? StrDupW(pDesc->dynamicPort2NamePrefix) : NULL;
			return 1;
		}
	}

	// 新增
	INT newCount = pData->cardRegistryCount + 1;
	EX_FLOWGRAPH_CARD_DESCRIPTOR* newArr = (EX_FLOWGRAPH_CARD_DESCRIPTOR*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_CARD_DESCRIPTOR) * newCount);
	if (pData->cardRegistryCount > 0) {
		memcpy(newArr, pData->cardRegistry, sizeof(EX_FLOWGRAPH_CARD_DESCRIPTOR) * pData->cardRegistryCount);
		Ex_MemFree(pData->cardRegistry);
	}
	pData->cardRegistry = newArr;
	EX_FLOWGRAPH_CARD_DESCRIPTOR* dst = &newArr[pData->cardRegistryCount];
	dst->cardType = pDesc->cardType;
	dst->typeName = StrDupW(pDesc->typeName);
	dst->tagColor = pDesc->tagColor;
	dst->portCount = pDesc->portCount;
	dst->ports = (EX_FLOWGRAPH_PORT_DESC*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_PORT_DESC) * pDesc->portCount);
	// 第一组
	dst->dynamicPortBaseIndex = pDesc->dynamicPortBaseIndex;
	dst->dynamicPortMinCount = pDesc->dynamicPortMinCount;
	dst->dynamicPortMaxCount = pDesc->dynamicPortMaxCount;
	dst->dynamicPortDataType = pDesc->dynamicPortDataType;
	dst->dynamicPortNamePrefix = pDesc->dynamicPortNamePrefix ? StrDupW(pDesc->dynamicPortNamePrefix) : NULL;
	// ★ 第二组
	dst->dynamicPort2BaseIndex = pDesc->dynamicPort2BaseIndex;
	dst->dynamicPort2MinCount = pDesc->dynamicPort2MinCount;
	dst->dynamicPort2MaxCount = pDesc->dynamicPort2MaxCount;
	dst->dynamicPort2DataType = pDesc->dynamicPort2DataType;
	dst->dynamicPort2NamePrefix = pDesc->dynamicPort2NamePrefix ? StrDupW(pDesc->dynamicPort2NamePrefix) : NULL;

	for (INT j = 0; j < pDesc->portCount; j++) {
		dst->ports[j] = pDesc->ports[j];
		dst->ports[j].name = StrDupW(pDesc->ports[j].name);
		dst->ports[j].defaultWidgetData = _flowgraph_copy_widget_data(pDesc->ports[j].widgetType, pDesc->ports[j].defaultWidgetData);
	}
	pData->cardRegistryCount = newCount;
	return 1;
}

// ==================== 注销卡片类型 ====================
INT _flowgraph_unregister_card_type(HEXOBJ hObj, INT cardType)
{
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData) return 0;
	INT index = -1;
	for (INT i = 0; i < pData->cardRegistryCount; i++) {
		if (pData->cardRegistry[i].cardType == cardType) { index = i; break; }
	}
	if (index == -1) return 0;
	_flowgraph_free_card_descriptor(&pData->cardRegistry[index]);
	if (pData->cardRegistryCount > 1) {
		EX_FLOWGRAPH_CARD_DESCRIPTOR* newArr = (EX_FLOWGRAPH_CARD_DESCRIPTOR*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_CARD_DESCRIPTOR) * (pData->cardRegistryCount - 1));
		if (index > 0) memcpy(newArr, pData->cardRegistry, sizeof(EX_FLOWGRAPH_CARD_DESCRIPTOR) * index);
		if (index < pData->cardRegistryCount - 1) memcpy(newArr + index, pData->cardRegistry + index + 1, sizeof(EX_FLOWGRAPH_CARD_DESCRIPTOR) * (pData->cardRegistryCount - index - 1));
		Ex_MemFree(pData->cardRegistry);
		pData->cardRegistry = newArr;
	}
	else { Ex_MemFree(pData->cardRegistry); pData->cardRegistry = NULL; }
	pData->cardRegistryCount--;
	return 1;
}

// ==================== 创建自定义节点 ====================
INT _flowgraph_create_custom_node(HEXOBJ hObj, EX_FLOWGRAPH_CUSTOM_NODE_CREATE* p)
{
	if (!p) return 0;
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData) return 0;
	EX_FLOWGRAPH_CARD_DESCRIPTOR* desc = _flowgraph_find_card_descriptor(pData, p->cardType);
	if (!desc || desc->portCount <= 0) return 0;

	EX_FLOWGRAPH_NODE node = { 0 };
	node.cardType = p->cardType;
	node.x = p->x; node.y = p->y;
	node.title = p->title;
	node.portCount = desc->portCount;

	// ★ 初始化两组动态端口的当前数量
	node.dynamicPortCount = 0;
	node.dynamicPortCount2 = 0;

	// 第一组动态端口 (如：参考图片)
	if (desc->dynamicPortBaseIndex >= 0) {
		node.dynamicPortCount = desc->dynamicPortMinCount;
	}
	// ★ 第二组动态端口 (如：参考音频)
	if (desc->dynamicPort2BaseIndex >= 0) {
		node.dynamicPortCount2 = desc->dynamicPort2MinCount;
	}

	node.ports = (EX_FLOWGRAPH_PORT*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_PORT) * desc->portCount);
	memset(node.ports, 0, sizeof(EX_FLOWGRAPH_PORT) * desc->portCount);

	for (INT i = 0; i < desc->portCount; i++) {
		EX_FLOWGRAPH_PORT_DESC* srcPort = &desc->ports[i];
		EX_FLOWGRAPH_PORT* dstPort = &node.ports[i];

		dstPort->portType = srcPort->portType;
		dstPort->dataType = srcPort->dataType;
		dstPort->name = StrDupW(srcPort->name);
		dstPort->widgetType = srcPort->widgetType;
		dstPort->widgetId = srcPort->widgetId;
		dstPort->widgetWidth = srcPort->widgetWidth;
		dstPort->widgetHeight = srcPort->widgetHeight;
		dstPort->isConnected = FALSE;

		// VIDEO类型自动创建播放器数据
		if (srcPort->widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO) {
			dstPort->widgetData = (LPVOID)_flowgraph_create_video_data(pData->libVlc, hObj);
		}
		else {
			dstPort->widgetData = _flowgraph_copy_widget_data(srcPort->widgetType, srcPort->defaultWidgetData);
		}
	}

	INT result = _flowgraph_addnode(hObj, &node);
	_flowgraph_free_temp_node_data(&node);
	return result;
}

// ==================== 图片组件自适应大小 ====================
void _flowgraph_adjust_image_size(EX_FLOWGRAPH_PORT* port)
{
	if (port->widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE && port->widgetData) {
		INT imgW, imgH;
		_img_getsize((HEXIMAGE)port->widgetData, &imgW, &imgH);
		if (imgW > 0 && imgH > 0) {
			// 宽度为1/4，最小300；高度按宽度等比例缩放
			FLOAT newW = __max(300.0f, (FLOAT)imgW / 3.0f);
			FLOAT ratio = newW / (FLOAT)imgW;
			FLOAT newH = (FLOAT)imgH * ratio;
			port->widgetWidth = newW;
			port->widgetHeight = newH;
		}
	}
}

// ==================== 查找点击位置最顶层节点(解决重叠穿透) ====================
EX_FLOWGRAPH_NODE* _flowgraph_find_topmost_node_at(EX_FLOWGRAPH_DATA* pData, FLOAT virtualX, FLOAT virtualY)
{
	// 选中节点始终在最顶层绘制，优先检测
	if (pData->selectedNode != -1) {
		EX_FLOWGRAPH_NODE* selNode = _flowgraph_findnode(pData, pData->selectedNode);
		if (selNode && virtualX >= selNode->x && virtualX <= selNode->x + selNode->width &&
			virtualY >= selNode->y && virtualY <= selNode->y + selNode->height) {
			return selNode;
		}
	}
	// 其他节点：数组靠后的节点后绘制（z-order更高）
	for (INT i = pData->nodeCount - 1; i >= 0; i--) {
		EX_FLOWGRAPH_NODE* node = &pData->nodes[i];
		if (node->id == pData->selectedNode) continue; // 已检测过
		if (virtualX >= node->x && virtualX <= node->x + node->width &&
			virtualY >= node->y && virtualY <= node->y + node->height) {
			return node;
		}
	}
	return NULL;
}

// ==================== 检查是否为上游节点(带防环保护) ====================
BOOL _flowgraph_is_upstream_of_internal(EX_FLOWGRAPH_DATA* pData, INT nodeId, INT targetId, BOOL* visited) {
	if (nodeId == targetId) return TRUE;
	for (INT i = 0; i < pData->nodeCount; i++) {
		if (pData->nodes[i].id == targetId) {
			if (visited[i]) return FALSE;
			visited[i] = TRUE;
			break;
		}
	}
	for (INT i = 0; i < pData->connectionCount; i++) {
		if (pData->connections[i].toNode == targetId) {
			if (_flowgraph_is_upstream_of_internal(pData, nodeId, pData->connections[i].fromNode, visited)) return TRUE;
		}
	}
	return FALSE;
}

BOOL _flowgraph_is_upstream_of(EX_FLOWGRAPH_DATA* pData, INT nodeId, INT targetId) {
	if (pData->nodeCount == 0) return FALSE;
	BOOL* visited = (BOOL*)calloc(pData->nodeCount, sizeof(BOOL));
	BOOL res = _flowgraph_is_upstream_of_internal(pData, nodeId, targetId, visited);
	free(visited);
	return res;
}

BOOL _flowgraph_is_node_protected_by_busy(EX_FLOWGRAPH_DATA* pData, INT nodeId) {
	for (INT i = 0; i < pData->nodeCount; i++) {
		EX_FLOWGRAPH_NODE* n = &pData->nodes[i];
		if (n->executionStatus == FLOWGRAPH_EXEC_STATUS_RUNNING || n->executionStatus == FLOWGRAPH_EXEC_STATUS_PENDING) {
			if (_flowgraph_is_upstream_of(pData, nodeId, n->id)) return TRUE;
		}
	}
	return FALSE;
}

BOOL _flowgraph_is_connection_protected_by_busy(EX_FLOWGRAPH_DATA* pData, INT connId) {
	EX_FLOWGRAPH_CONNECTION* conn = _flowgraph_findconnection(pData, connId);
	if (!conn) return FALSE;
	return _flowgraph_is_node_protected_by_busy(pData, conn->toNode);
}

// ==================== 强制取消节点执行 ====================
void _flowgraph_cancel_node(HEXOBJ hObj, INT nodeId) {
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData) return;
	EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, nodeId);
	if (!node) return;
	BOOL wasBusy = (node->executionStatus == FLOWGRAPH_EXEC_STATUS_RUNNING ||
		node->executionStatus == FLOWGRAPH_EXEC_STATUS_PENDING);
	node->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
	_flowgraph_invalidate_downstream(pData, nodeId);

	for (INT i = pData->chainContextCount - 1; i >= 0; i--) {
		EX_FLOWGRAPH_CHAIN_CTX* ctx = &pData->chainContexts[i];
		BOOL contains = FALSE;
		for (INT j = 0; j < ctx->nodeOrderCount; j++) {
			if (ctx->nodeOrder[j] == nodeId) { contains = TRUE; break; }
		}
		if (contains) {
			_flowgraph_remove_chain_context(pData, ctx->chainEndNode);
		}
	}

	if (pData->asyncPendingNode == nodeId) {
		pData->asyncPendingNode = -1;
	}
	if (wasBusy) {
		Ex_ObjDispatchNotify(hObj, FLOWGRAPH_EVENT_NODE_CANCELED, (WPARAM)nodeId, (LPARAM)node->cardType);
	}
	Ex_ObjInvalidateRect(hObj, 0);
}

void _flowgraph_draw_cancel_panel(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, FLOAT canvasWidth, FLOAT canvasHeight) {
	INT busyNodes[1024]; INT busyCount = 0;
	for (INT i = 0; i < pData->nodeCount && busyCount < 1024; i++) {
		if (pData->nodes[i].executionStatus == FLOWGRAPH_EXEC_STATUS_RUNNING || pData->nodes[i].executionStatus == FLOWGRAPH_EXEC_STATUS_PENDING) {
			busyNodes[busyCount++] = i;
		}
	}
	if (busyCount == 0) { pData->sidebarShowCancelPanel = FALSE; return; }

	INT visibleCount = __min(busyCount, FLOWGRAPH_CANCEL_PANEL_MAX_VISIBLE);
	FLOAT panelX = FLOWGRAPH_SIDEBAR_WIDTH;
	FLOAT panelY = (FLOAT)FLOWGRAPH_SIDEBAR_BTN_Y3; // ★ 原Y5
	FLOAT panelW = FLOWGRAPH_CANCEL_PANEL_WIDTH;
	FLOAT panelH = FLOWGRAPH_CANCEL_PANEL_HEADER + visibleCount * FLOWGRAPH_CANCEL_PANEL_ITEM_H + 4;

	HEXBRUSH hBrushShadow = _brush_create(ExARGB(0, 0, 0, 50));
	_canvas_fillrect(hCanvas, hBrushShadow, panelX + 3, panelY + 3, panelX + panelW + 3, panelY + panelH + 3);
	_brush_destroy(hBrushShadow);
	HEXBRUSH hBrushPanelBg = _brush_create(ExARGB(36, 28, 28, 248));
	_canvas_fillrect(hCanvas, hBrushPanelBg, panelX, panelY, panelX + panelW, panelY + panelH);
	_brush_destroy(hBrushPanelBg);
	HEXBRUSH hBrushBorder = _brush_create(ExARGB(80, 65, 65, 255));
	_canvas_drawrect(hCanvas, hBrushBorder, panelX, panelY, panelX + panelW, panelY + panelH, 1.0f, 0);
	_brush_destroy(hBrushBorder);

	HEXFONT hFontTitle = _font_createfromfamily(L"微软雅黑", 10, 0);
	_canvas_drawtext(hCanvas, hFontTitle, ExARGB(255, 120, 120, 255), L"强制取消执行", -1, DT_LEFT | DT_VCENTER, panelX + 8, panelY + 2, panelX + panelW - 8, panelY + FLOWGRAPH_CANCEL_PANEL_HEADER);
	_font_destroy(hFontTitle);

	FLOAT itemStartY = panelY + FLOWGRAPH_CANCEL_PANEL_HEADER;
	FLOAT itemH = FLOWGRAPH_CANCEL_PANEL_ITEM_H;
	for (INT i = 0; i < visibleCount; i++) {
		EX_FLOWGRAPH_NODE* node = &pData->nodes[busyNodes[i]];
		FLOAT iy = itemStartY + i * itemH;
		BOOL isHover = (pData->sidebarCancelPanelHover == i);
		if (isHover) {
			HEXBRUSH hBrushItemHover = _brush_create(ExARGB(65, 50, 50, 200));
			_canvas_fillrect(hCanvas, hBrushItemHover, panelX + 1, iy + 1, panelX + panelW - 1, iy + itemH - 1);
			_brush_destroy(hBrushItemHover);
		}
		EXARGB statusColor = _flowgraph_get_status_color(node->executionStatus);
		HEXBRUSH hBrushStatus = _brush_create(statusColor);
		_canvas_fillellipse(hCanvas, hBrushStatus, panelX + 20, iy + itemH / 2, 4.0f, 4.0f);
		_brush_destroy(hBrushStatus);

		WCHAR nodeText[256];
		swprintf_s(nodeText, 256, L"%s (Id:%d)", node->title, node->id);
		HEXFONT hFont = _font_createfromfamily(L"微软雅黑", 11, 0);
		EXARGB textColor = isHover ? ExARGB(245, 240, 240, 255) : ExARGB(200, 185, 185, 255);
		_canvas_drawtext(hCanvas, hFont, textColor, nodeText, -1, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS, panelX + 32, iy, panelX + panelW - 10, iy + itemH);
		_font_destroy(hFont);
	}
}

// ==================== 判断卡片类型是否支持浮动编辑面板 ====================
BOOL _flowgraph_card_type_has_edit_panel(EX_FLOWGRAPH_NODE* node) {
	if (!node) return FALSE;
	// ★ 核心修复：不再硬编码 cardType，而是动态检测节点是否包含可编辑的长文本端口
	for (INT i = 0; i < node->portCount; i++) {
		if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE &&
			(node->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT ||
				node->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT)) {
			return TRUE;
		}
	}
	return FALSE;
}

// ==================== 获取浮动编辑面板矩形区域 ====================
void _flowgraph_get_edit_panel_rect(HEXOBJ hObj, FLOAT* outX, FLOAT* outY, FLOAT* outW, FLOAT* outH) {
	RECT rcClient;
	Ex_ObjGetClientRect(hObj, &rcClient);
	FLOAT clientW = (FLOAT)(rcClient.right - rcClient.left);
	FLOAT clientH = (FLOAT)(rcClient.bottom - rcClient.top);
	*outW = Ex_Scale(FLOWGRAPH_EDIT_PANEL_WIDTH);
	*outH = Ex_Scale(FLOWGRAPH_EDIT_PANEL_HEIGHT);
	*outX = __max((FLOAT)FLOWGRAPH_SIDEBAR_WIDTH + 10, clientW - *outW - 20);
	*outY = clientH - *outH - 20;
}
// ==================== 更新浮动编辑面板状态 ====================
void _flowgraph_update_edit_panel(HEXOBJ hObj) {
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData) return;
	if (pData->selectedNode != -1) {
		EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, pData->selectedNode);
		if (node && _flowgraph_card_type_has_edit_panel(node)) {
			// 找到该节点第一个INTERMEDIATE/TEXT端口的当前内容
			LPCWSTR currentText = L"";
			for (INT i = 0; i < node->portCount; i++) {
				if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE &&
					(node->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT ||
						node->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT) &&
					node->ports[i].widgetData) {
					currentText = (LPCWSTR)node->ports[i].widgetData;
					break;
				}
			}
			// 计算面板位置
			FLOAT px, py, pw, ph;
			_flowgraph_get_edit_panel_rect(hObj, &px, &py, &pw, &ph);
			// 移动编辑框到面板内（顶部留标题栏30px，底部留按钮区50px）
			Ex_ObjMove(pData->editPanelEdit, (INT)px + 10, (INT)py + 32, (INT)pw - 20, (INT)ph - 82, TRUE);
			// 设置编辑框内容
			Ex_ObjSendMessage(pData->editPanelEdit, FLOWGRAPHEDIT_MESSAGE_SETINITTEXT, 0, (LPARAM)currentText);
			// 显示面板
			pData->showEditPanel = TRUE;
			pData->editPanelTargetNode = node->id;
			pData->editPanelBtnHover = FALSE;
			Ex_ObjShow(pData->editPanelEdit, TRUE);
			Ex_ObjInvalidateRect(hObj, 0);
			return;
		}
	}
	// 隐藏面板
	if (pData->showEditPanel) {
		pData->showEditPanel = FALSE;
		pData->editPanelTargetNode = -1;
		pData->editPanelBtnHover = FALSE;
		Ex_ObjShow(pData->editPanelEdit, FALSE);
		Ex_ObjInvalidateRect(hObj, 0);
	}
}
// ==================== 浮动编辑面板提交内容 ====================
void _flowgraph_editpanel_submit(HEXOBJ hObj) {
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData || pData->editPanelTargetNode == -1) return;

	// 1. 获取纯文本
	INT len = (INT)Ex_ObjSendMessage(pData->editPanelEdit, FLOWGRAPHEDIT_MESSAGE_GETPLAINTEXT, 0, 0);
	std::wstring text;
	text.resize(len + 1);
	Ex_ObjSendMessage(pData->editPanelEdit, FLOWGRAPHEDIT_MESSAGE_GETPLAINTEXT, len + 1, (LPARAM)text.data());
	while (!text.empty() && text.back() == L'\0') text.pop_back();

	// 2. ★ 核心修复：立即分配持久化内存，杜绝野指针！
	LPWSTR duplicatedText = StrDupW(text.c_str());

	// 3. 直接查找节点并更新底层数据，不再通过 SendMessage 绕一圈
	EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, pData->editPanelTargetNode);
	if (node) {
		for (INT i = 0; i < node->portCount; i++) {
			if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE &&
				(node->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT ||
					node->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT)) {

				// 释放旧数据
				if (node->ports[i].widgetData) Ex_MemFree(node->ports[i].widgetData);

				// 赋值新分配的持久内存
				node->ports[i].widgetData = (LPVOID)duplicatedText;

				// 触发 UI 刷新和下游失效
				node->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
				_flowgraph_invalidate_downstream(pData, node->id);
				_flowgraph_calcnodesize(hObj, node);
				_flowgraph_updatelayout(hObj);
				Ex_ObjInvalidateRect(hObj, 0);
				return; // 成功，直接返回
			}
		}
	}
	// 如果没找到对应端口，释放内存防止泄漏
	Ex_MemFree(duplicatedText);
}
// ==================== 绘制浮动编辑面板 ====================
void _flowgraph_draw_edit_panel(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, HEXOBJ hObj, FLOAT canvasWidth, FLOAT canvasHeight) {
	FLOAT px, py, pw, ph;
	_flowgraph_get_edit_panel_rect(hObj, &px, &py, &pw, &ph);
	// 阴影
	HEXBRUSH hBrushShadow = _brush_create(ExARGB(0, 0, 0, 60));
	_canvas_fillrect(hCanvas, hBrushShadow, px + 5, py + 5, px + pw + 5, py + ph + 5);
	_brush_destroy(hBrushShadow);
	// 背景
	HEXBRUSH hBrushBg = _brush_create(ExARGB(32, 32, 40, 250));
	_canvas_fillroundedrect(hCanvas, hBrushBg, px, py, px + pw, py + ph, 6.0f, 6.0f);
	_brush_destroy(hBrushBg);
	// 边框
	HEXBRUSH hBrushBorder = _brush_create(ExARGB(75, 75, 95, 255));
	_canvas_drawroundedrect(hCanvas, hBrushBorder, px, py, px + pw, py + ph, 6.0f, 6.0f, 1.0f, 0);
	_brush_destroy(hBrushBorder);
	// 标题栏背景
	HEXBRUSH hBrushTitleBg = _brush_create(ExARGB(40, 40, 50, 255));
	_canvas_fillrect(hCanvas, hBrushTitleBg, px + 1, py + 1, px + pw - 1, py + 28);
	_brush_destroy(hBrushTitleBg);
	// 标题文字
	EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, pData->editPanelTargetNode);
	WCHAR titleText[256];
	if (node) swprintf_s(titleText, 256, L"📝 编辑内容 - %s(Id:%d)", node->title, node->id);
	else lstrcpyW(titleText, L"📝 编辑内容");
	HEXFONT hFontTitle = _font_createfromfamily(L"微软雅黑", 11, FONT_STYLE_BOLD);
	_canvas_drawtext(hCanvas, hFontTitle, ExARGB(210, 210, 220, 255), titleText, -1,
		DT_LEFT | DT_VCENTER, px + 12, py + 2, px + pw - 12, py + 28);
	_font_destroy(hFontTitle);
	// 提示文字（编辑框上方）
	HEXFONT hFontHint = _font_createfromfamily(L"微软雅黑", 9, 0);
	_canvas_drawtext(hCanvas, hFontHint, ExARGB(120, 120, 140, 255), L"在下方编辑内容，完成后点击提交内容按钮", -1, DT_LEFT | DT_VCENTER, px + 10, py + ph - 78, px + pw - 10, py + ph - 58);
	_font_destroy(hFontHint);
	// 提交按钮
	FLOAT btnX = px + pw - 120, btnY = py + ph - 45, btnW = 110, btnH = 35;
	EXARGB btnBgColor = pData->editPanelBtnHover ? ExARGB(70, 130, 220, 255) : ExARGB(55, 100, 190, 255);
	HEXBRUSH hBrushBtn = _brush_create(btnBgColor);
	_canvas_fillroundedrect(hCanvas, hBrushBtn, btnX, btnY, btnX + btnW, btnY + btnH, 4.0f, 4.0f);
	_brush_destroy(hBrushBtn);
	HEXBRUSH hBrushBtnBorder = _brush_create(ExARGB(90, 150, 240, 255));
	_canvas_drawroundedrect(hCanvas, hBrushBtnBorder, btnX, btnY, btnX + btnW, btnY + btnH, 4.0f, 4.0f, 1.0f, 0);
	_brush_destroy(hBrushBtnBorder);
	HEXFONT hFontBtn = _font_createfromfamily(L"微软雅黑", 11, 0);
	EXARGB btnTextColor = pData->editPanelBtnHover ? ExARGB(255, 255, 255, 255) : ExARGB(220, 230, 245, 255);
	_canvas_drawtext(hCanvas, hFontBtn, btnTextColor, L"提交内容", -1,
		DT_CENTER | DT_VCENTER, btnX, btnY, btnX + btnW, btnY + btnH);
	_font_destroy(hFontBtn);
}

LRESULT CALLBACK _flowgraph_edit_at_triggered(HEXOBJ hObj, INT nID, INT nCode, WPARAM wParam, LPARAM lParam)
{
	HEXOBJ hFlowGraph = Ex_ObjGetParent(hObj);
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hFlowGraph, 0);
	if (!pData) return 0;
	// 清空旧的条目
	Ex_ObjSendMessage(hObj, FLOWGRAPHEDIT_MESSAGE_CLEARITEMS, 0, 0);
	if (pData->selectedNode != -1) {
		EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, pData->selectedNode);
		if (node) {
			// 遍历所有端口，找到有连线的 IMAGE 类型 INPUT 端口
			for (INT i = 0; i < node->portCount; i++) {
				EX_FLOWGRAPH_PORT& port = node->ports[i];
				if (port.portType == FLOWGRAPH_PORTTYPE_INPUT &&
					(port.dataType == FLOWGRAPH_DATATYPE_IMAGE || port.dataType == FLOWGRAPH_DATATYPE_AUDIO) &&
					port.isConnected) {
					FLOWGRAPHEDIT_ITEM item;
					item.szName = port.name; // 例如 "参考图片0"
					Ex_ObjSendMessage(hObj, FLOWGRAPHEDIT_MESSAGE_ADDITEM, 0, (LPARAM)&item);
				}
			}
		}
	}
	// 添加完条目后显示面板
	Ex_ObjSendMessage(hObj, FLOWGRAPHEDIT_MESSAGE_SHOWPANEL, 0, 0);
	return 0;
}

INT _flowgraph_add_dynamic_port2(HEXOBJ hObj, INT nodeId) {
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData) return 0;
	EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, nodeId);
	if (!node) return 0;
	EX_FLOWGRAPH_CARD_DESCRIPTOR* desc = _flowgraph_find_card_descriptor(pData, node->cardType);
	if (!desc || desc->dynamicPort2BaseIndex < 0 || desc->dynamicPort2MaxCount <= 0) return 0;
	if (node->dynamicPortCount2 >= desc->dynamicPort2MaxCount) return 0;

	INT insertIdx = desc->dynamicPortBaseIndex + node->dynamicPortCount + desc->dynamicPort2BaseIndex + node->dynamicPortCount2;
	INT newPortCount = node->portCount + 1;
	EX_FLOWGRAPH_PORT* newPorts = (EX_FLOWGRAPH_PORT*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_PORT) * newPortCount);
	if (insertIdx > 0) memcpy(newPorts, node->ports, sizeof(EX_FLOWGRAPH_PORT) * insertIdx);
	memcpy(newPorts + insertIdx + 1, node->ports + insertIdx, sizeof(EX_FLOWGRAPH_PORT) * (node->portCount - insertIdx));

	WCHAR name[32];
	swprintf_s(name, 32, L"%s%d", desc->dynamicPort2NamePrefix, node->dynamicPortCount2);
	pData->nextAutoId++;
	newPorts[insertIdx].id = pData->nextAutoId;
	newPorts[insertIdx].portType = FLOWGRAPH_PORTTYPE_INPUT;
	newPorts[insertIdx].dataType = desc->dynamicPort2DataType;
	newPorts[insertIdx].name = StrDupW(name);
	newPorts[insertIdx].widgetType = 0; newPorts[insertIdx].widgetId = 0;
	newPorts[insertIdx].widgetWidth = 0; newPorts[insertIdx].widgetHeight = 0;
	memset(&newPorts[insertIdx].widgetRect, 0, sizeof(RECT));
	memset(&newPorts[insertIdx].portRect, 0, sizeof(RECT));
	newPorts[insertIdx].widgetData = NULL; newPorts[insertIdx].isConnected = FALSE;
	newPorts[insertIdx].imagePath = NULL;
	Ex_MemFree(node->ports); node->ports = newPorts; node->portCount = newPortCount;
	node->dynamicPortCount2++;

	for (INT i = 0; i < pData->connectionCount; i++) {
		if (pData->connections[i].fromNode == nodeId && pData->connections[i].fromSlot >= insertIdx) pData->connections[i].fromSlot++;
		if (pData->connections[i].toNode == nodeId && pData->connections[i].toSlot >= insertIdx) pData->connections[i].toSlot++;
	}
	node->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
	_flowgraph_invalidate_downstream(pData, node->id);
	_flowgraph_calcnodesize(hObj, node); _flowgraph_updatelayout(hObj); Ex_ObjInvalidateRect(hObj, 0);
	return 1;
}

INT _flowgraph_remove_dynamic_port2(HEXOBJ hObj, INT nodeId) {
	EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
	if (!pData) return 0;
	EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, nodeId);
	if (!node) return 0;
	EX_FLOWGRAPH_CARD_DESCRIPTOR* desc = _flowgraph_find_card_descriptor(pData, node->cardType);
	if (!desc || desc->dynamicPort2BaseIndex < 0) return 0;
	if (node->dynamicPortCount2 <= desc->dynamicPort2MinCount) return 0;

	INT removeIdx = desc->dynamicPortBaseIndex + node->dynamicPortCount + desc->dynamicPort2BaseIndex + node->dynamicPortCount2 - 1;
	for (INT i = pData->connectionCount - 1; i >= 0; i--) {
		EX_FLOWGRAPH_CONNECTION* conn = &pData->connections[i];
		if ((conn->toNode == nodeId && conn->toSlot == removeIdx) || (conn->fromNode == nodeId && conn->fromSlot == removeIdx))
			_flowgraph_removeconnection(hObj, conn->id);
	}
	for (INT i = 0; i < pData->connectionCount; i++) {
		if (pData->connections[i].fromNode == nodeId && pData->connections[i].fromSlot > removeIdx) pData->connections[i].fromSlot--;
		if (pData->connections[i].toNode == nodeId && pData->connections[i].toSlot > removeIdx) pData->connections[i].toSlot--;
	}
	Ex_MemFree((void*)node->ports[removeIdx].name);
	if (node->ports[removeIdx].widgetData) {
		if (node->ports[removeIdx].dataType == FLOWGRAPH_DATATYPE_IMAGE) 
		{ 
			_img_destroy((HEXIMAGE)node->ports[removeIdx].widgetData); 
			if (node->ports[removeIdx].imagePath) Ex_MemFree((void*)node->ports[removeIdx].imagePath);
		}
		else if (node->ports[removeIdx].dataType == FLOWGRAPH_DATATYPE_STRING || node->ports[removeIdx].dataType == FLOWGRAPH_DATATYPE_AUDIO) Ex_MemFree(node->ports[removeIdx].widgetData);
	}
	for (INT i = removeIdx; i < node->portCount - 1; i++) node->ports[i] = node->ports[i + 1];
	node->portCount--; node->dynamicPortCount2--;

	EX_FLOWGRAPH_PORT* newPorts = (EX_FLOWGRAPH_PORT*)Ex_MemAlloc(sizeof(EX_FLOWGRAPH_PORT) * node->portCount);
	memcpy(newPorts, node->ports, sizeof(EX_FLOWGRAPH_PORT) * node->portCount);
	Ex_MemFree(node->ports); node->ports = newPorts;
	node->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
	_flowgraph_invalidate_downstream(pData, node->id);
	_flowgraph_calcnodesize(hObj, node); _flowgraph_updatelayout(hObj); Ex_ObjInvalidateRect(hObj, 0);
	return 1;
}

// ==================== 绘制右键菜单面板 ====================
void _flowgraph_draw_context_menu(HEXOBJ hObj, HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData) {
	FLOAT menuW = FLOWGRAPH_CONTEXT_MENU_WIDTH;
	FLOAT menuH = FLOWGRAPH_CONTEXT_MENU_HEADER * 2 + FLOWGRAPH_CONTEXT_MENU_ITEM_H * 4;
	FLOAT mx = pData->contextMenuX;
	FLOAT my = pData->contextMenuY;

	// 边界检测，防止菜单超出画布
	RECT rcClient;
	Ex_ObjGetClientRect(hObj, &rcClient);
	FLOAT clientW = (FLOAT)(rcClient.right - rcClient.left);
	FLOAT clientH = (FLOAT)(rcClient.bottom - rcClient.top);
	if (mx + menuW > clientW) mx = clientW - menuW;
	if (my + menuH > clientH) my = clientH - menuH;
	if (mx < 0) mx = 0;
	if (my < 0) my = 0;

	// 阴影
	HEXBRUSH hBrushShadow = _brush_create(ExARGB(0, 0, 0, 60));
	_canvas_fillrect(hCanvas, hBrushShadow, mx + 4, my + 4, mx + menuW + 4, my + menuH + 4);
	_brush_destroy(hBrushShadow);

	// 背景
	HEXBRUSH hBrushBg = _brush_create(ExARGB(35, 35, 45, 245));
	_canvas_fillroundedrect(hCanvas, hBrushBg, mx, my, mx + menuW, my + menuH, 4.0f, 4.0f);
	_brush_destroy(hBrushBg);

	// 边框
	HEXBRUSH hBrushBorder = _brush_create(ExARGB(70, 70, 90, 255));
	_canvas_drawroundedrect(hCanvas, hBrushBorder, mx, my, mx + menuW, my + menuH, 4.0f, 4.0f, 1.0f, 0);
	_brush_destroy(hBrushBorder);

	LPCWSTR items[] = { L"从头执行", L"继续执行", L"取消执行", L"清空数据" };
	HEXFONT hFont = _font_createfromfamily(L"微软雅黑", 11, 0);

	EX_FLOWGRAPH_NODE* node = _flowgraph_findnode(pData, pData->contextMenuNodeId);

	for (INT i = 0; i < 4; i++) {
		FLOAT iy = my + FLOWGRAPH_CONTEXT_MENU_HEADER + i * FLOWGRAPH_CONTEXT_MENU_ITEM_H;
		BOOL isHover = (pData->contextMenuHover == i);

		// 悬停高亮
		if (isHover) {
			HEXBRUSH hBrushHover = _brush_create(ExARGB(60, 120, 200, 200));
			_canvas_fillrect(hCanvas, hBrushHover, mx + 4, iy + 2, mx + menuW - 4, iy + FLOWGRAPH_CONTEXT_MENU_ITEM_H - 2);
			_brush_destroy(hBrushHover);
		}

		EXARGB textColor = ExARGB(220, 220, 230, 255);
		BOOL disabled = FALSE;

		// 智能禁用：如果节点未在运行，灰显“取消执行”
		if (node) 
		{
			if(i == 2) 
			{
				if (node->executionStatus != FLOWGRAPH_EXEC_STATUS_RUNNING && node->executionStatus != FLOWGRAPH_EXEC_STATUS_PENDING) {
					disabled = TRUE;
				}
			}
			else if (i == 3) { // ★ 清空数据
				// 输入源节点禁用清空
				if (node->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_TEXT ||
					node->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_IMAGE ||
					node->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_AUDIO) {
					disabled = TRUE;
				}
				else {
					// 智能检测：如果没有可清空的数据，则灰显禁用
					BOOL hasData = FALSE;
					for (INT j = 0; j < node->portCount; j++) {
						EX_FLOWGRAPH_PORT& port = node->ports[j];
						if (port.portType == FLOWGRAPH_PORTTYPE_OUTPUT && port.widgetData) { hasData = TRUE; break; }
						if (port.portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) {
							if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE && port.widgetData) { hasData = TRUE; break; }
							if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO && port.widgetData) {
								EX_FLOWGRAPH_NODE_VIDEO_DATA* pv = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)port.widgetData;
								if (pv->bIsLoaded || pv->bIsPlaying || pv->bIsPaused || pv->hCoverImg) { hasData = TRUE; break; }
							}
							if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT && port.widgetData && node->cardType == FLOWGRAPH_CARD_TYPE_TEXT_RENDER) {
								if (lstrlenW((LPCWSTR)port.widgetData) > 0) { hasData = TRUE; break; }
							}
						}
					}
					if (!hasData) disabled = TRUE;
				}
			}
		}
		if (disabled) textColor = ExARGB(100, 100, 110, 255);

		_canvas_drawtext(hCanvas, hFont, textColor, items[i], -1, DT_CENTER | DT_VCENTER, mx, iy, mx + menuW, iy + FLOWGRAPH_CONTEXT_MENU_ITEM_H);
	}
	_font_destroy(hFont);
}

// ==================== 清空节点生成数据 ====================
void _flowgraph_clear_node_outputs(HEXOBJ hObj, EX_FLOWGRAPH_NODE* node) {
	if (!node) return;
	// ★ 保护输入源节点：本地文本、本地图、本地音频不参与清空
	if (node->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_TEXT ||
		node->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_IMAGE ||
		node->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_AUDIO) {
		return;
	}

	BOOL changed = FALSE;
	for (INT i = 0; i < node->portCount; i++) {
		EX_FLOWGRAPH_PORT& port = node->ports[i];

		// 1. 清空所有 OUTPUT 端口的数据 (如生成的图片、输出的文本/视频路径)
		if (port.portType == FLOWGRAPH_PORTTYPE_OUTPUT && port.widgetData) {
			if (port.dataType == FLOWGRAPH_DATATYPE_STRING) {
				Ex_MemFree(port.widgetData);
				port.widgetData = NULL;
				changed = TRUE;
			}
			else if (port.dataType == FLOWGRAPH_DATATYPE_IMAGE) {
				_img_destroy((HEXIMAGE)port.widgetData);
				port.widgetData = NULL;
				if (port.imagePath) { Ex_MemFree((void*)port.imagePath); port.imagePath = NULL; }
				changed = TRUE;
			}
			// ★ 修复：补全 VIDEO 类型输出端口的资源释放，防止内存泄漏
			else if (port.dataType == FLOWGRAPH_DATATYPE_VIDEO) {
				EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)port.widgetData;
				if (pVideo) {
					_flowgraph_video_cleanup(pVideo);
					DeleteCriticalSection(&pVideo->critsec);
					Ex_MemFree(pVideo);
				}
				port.widgetData = NULL;
				changed = TRUE;
			}
			else {
				port.widgetData = NULL;
				changed = TRUE;
			}
		}
		// 2. 清空 INTERMEDIATE 的预览数据
		else if (port.portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE) {
			// 清空生成的预览图 (排除本地图节点的用户选图)
			if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE && port.widgetData) {
				_img_destroy((HEXIMAGE)port.widgetData);
				port.widgetData = NULL;
				if (port.imagePath) { Ex_MemFree((void*)port.imagePath); port.imagePath = NULL; }
				changed = TRUE;
			}
			// 清空预览视频 (停止播放并释放帧缓存)
			else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO && port.widgetData) {
				EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)port.widgetData;
				_flowgraph_video_cleanup(pVideo);
				changed = TRUE;
			}
			// 仅清空“文本显示”节点的生成结果，保留其他节点的输入提示词
			else if (port.widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT && port.widgetData) {
				if (node->cardType == FLOWGRAPH_CARD_TYPE_TEXT_RENDER) {
					Ex_MemFree(port.widgetData);
					port.widgetData = (LPVOID)StrDupW(L"");
					changed = TRUE;
				}
			}
		}
	}

	// 如果数据发生了改变，重置状态并级联失效下游
	if (changed) {
		node->executionStatus = FLOWGRAPH_EXEC_STATUS_IDLE;
		EX_FLOWGRAPH_DATA* pData = (EX_FLOWGRAPH_DATA*)Ex_ObjGetLong(hObj, 0);
		if (pData) {
			// ★★★ 核心修复：级联清空下游节点的输入缓存和生成数据 ★★★
			for (INT i = 0; i < pData->connectionCount; i++) {
				if (pData->connections[i].fromNode == node->id) {
					INT toNodeId = pData->connections[i].toNode;
					INT toSlot = pData->connections[i].toSlot;
					EX_FLOWGRAPH_NODE* toNode = _flowgraph_findnode(pData, toNodeId);
					if (toNode && toSlot < toNode->portCount) {
						// 1. 清空下游节点的 INPUT 端口缓存数据
						EX_FLOWGRAPH_PORT& dstPort = toNode->ports[toSlot];
						if (dstPort.portType == FLOWGRAPH_PORTTYPE_INPUT && dstPort.widgetData) {
							if (dstPort.dataType == FLOWGRAPH_DATATYPE_STRING) {
								Ex_MemFree(dstPort.widgetData);
								dstPort.widgetData = NULL;
							}
							else if (dstPort.dataType == FLOWGRAPH_DATATYPE_IMAGE) {
								_img_destroy((HEXIMAGE)dstPort.widgetData);
								dstPort.widgetData = NULL;
								if (dstPort.imagePath) { Ex_MemFree((void*)dstPort.imagePath); dstPort.imagePath = NULL; }
							}
							else if (dstPort.dataType == FLOWGRAPH_DATATYPE_VIDEO) {
								// 视频数据在 INPUT 端口是浅拷贝引用，不能释放，只能置空
								dstPort.widgetData = NULL;
							}
							else {
								dstPort.widgetData = NULL;
							}
						}
						// 2. 递归清空下游节点的生成数据 (彻底清除整条链路的视觉残留)
						_flowgraph_clear_node_outputs(hObj, toNode);
					}
				}
			}
			_flowgraph_invalidate_downstream(pData, node->id);
		}
		_flowgraph_calcnodesize(hObj, node);
		_flowgraph_updatelayout(hObj);
		Ex_ObjInvalidateRect(hObj, 0);
	}
}

// ==================== 选择状态管理 ====================
void _flowgraph_clear_selection(EX_FLOWGRAPH_DATA* pData) {
	if (pData->selectedNodes) {
		Ex_MemFree(pData->selectedNodes);
		pData->selectedNodes = NULL;
	}
	pData->selectedNodeCount = 0;
	pData->selectedNode = -1;
	pData->selectedConnection = -1;
	pData->selectedPortNode = -1;
	pData->selectedPortIndex = -1;
}

void _flowgraph_add_to_selection(EX_FLOWGRAPH_DATA* pData, INT nodeId) {
	for (INT i = 0; i < pData->selectedNodeCount; i++) {
		if (pData->selectedNodes[i] == nodeId) return;
	}
	INT* newArr = (INT*)Ex_MemAlloc(sizeof(INT) * (pData->selectedNodeCount + 1));
	if (pData->selectedNodeCount > 0) {
		memcpy(newArr, pData->selectedNodes, sizeof(INT) * pData->selectedNodeCount);
		Ex_MemFree(pData->selectedNodes);
	}
	newArr[pData->selectedNodeCount] = nodeId;
	pData->selectedNodes = newArr;
	pData->selectedNodeCount++;
	pData->selectedNode = nodeId; // 保持单选兼容
}

BOOL _flowgraph_is_node_selected(EX_FLOWGRAPH_DATA* pData, INT nodeId) {
	for (INT i = 0; i < pData->selectedNodeCount; i++) {
		if (pData->selectedNodes[i] == nodeId) return TRUE;
	}
	return FALSE;
}

void _flowgraph_update_selection_rect(EX_FLOWGRAPH_DATA* pData) {
	_flowgraph_clear_selection(pData);
	FLOAT minX = __min(pData->selectStartPos.x, pData->selectEndPos.x);
	FLOAT maxX = __max(pData->selectStartPos.x, pData->selectEndPos.x);
	FLOAT minY = __min(pData->selectStartPos.y, pData->selectEndPos.y);
	FLOAT maxY = __max(pData->selectStartPos.y, pData->selectEndPos.y);

	for (INT i = 0; i < pData->nodeCount; i++) {
		EX_FLOWGRAPH_NODE* node = &pData->nodes[i];
		if (node->x + node->width >= minX && node->x <= maxX &&
			node->y + node->height >= minY && node->y <= maxY) {
			_flowgraph_add_to_selection(pData, node->id);
		}
	}
}

HEXIMAGE _flowgraph_load_image_no_lock(LPCWSTR filePath) {
	// ★ 方案1：尝试读取到内存并从内存创建（最高效，彻底不锁定原文件）
	HANDLE hFile = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD fileSize = GetFileSize(hFile, NULL);
		if (fileSize > 0 && fileSize < 100 * 1024 * 1024) { // 限制100MB防止内存溢出
			BYTE* pBuffer = (BYTE*)malloc(fileSize);
			DWORD bytesRead = 0;
			if (ReadFile(hFile, pBuffer, fileSize, &bytesRead, NULL) && bytesRead == fileSize) {
				CloseHandle(hFile);
				HEXIMAGE hImg = NULL;
				// ⚠️ 注意：如果您的 ExDuilib 版本中该函数名为 Ex_ImageCreateFromMemory 或 _img_createfrombuffer，请自行替换
				// 如果编译报错提示找不到此函数，请直接删除【方案1】的全部代码，仅保留【方案2】即可。
				if (_img_createfrommemory(pBuffer, bytesRead, &hImg)) {
					free(pBuffer);
					return hImg;
				}
			}
			if (pBuffer) free(pBuffer);
		}
		CloseHandle(hFile);
	}

	// ★ 方案2：回退到“复制到临时目录再加载”（确保原文件不被锁定）
	WCHAR tempPath[MAX_PATH];
	GetTempPathW(MAX_PATH, tempPath);
	WCHAR tempFile[MAX_PATH];
	GetTempFileNameW(tempPath, L"FG_", 0, tempFile);

	// 补全原文件后缀，防止某些解码器不识别无后缀的临时文件
	std::wstring src = filePath;
	size_t dot = src.find_last_of(L".");
	if (dot != std::wstring::npos) {
		std::wstring ext = src.substr(dot);
		std::wstring newTemp = std::wstring(tempFile) + ext;
		MoveFileW(tempFile, newTemp.c_str());
		lstrcpyW(tempFile, newTemp.c_str());
	}

	if (CopyFileW(filePath, tempFile, FALSE)) {
		HEXIMAGE hImg = NULL;
		_img_createfromfile(tempFile, &hImg); // 加载临时副本，原文件彻底自由
		return hImg;
	}
	return NULL;
}