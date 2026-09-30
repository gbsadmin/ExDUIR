#pragma once
#include "vlc/vlc.h"


// ================= 侧边栏常量 =================
#define FLOWGRAPH_SIDEBAR_WIDTH 60
#define FLOWGRAPH_SIDEBAR_BTN_H 70
// 组内间距12px，组间间距20px
#define FLOWGRAPH_SIDEBAR_BTN_Y0 10   // 添加
#define FLOWGRAPH_SIDEBAR_BTN_Y1 110  // 链路从头 (组间)
#define FLOWGRAPH_SIDEBAR_BTN_Y2 202  // 链路继续 (组内)
#define FLOWGRAPH_SIDEBAR_BTN_Y3 302  // 取消 (组间)
#define FLOWGRAPH_SIDEBAR_BTN_Y4 402  // 导出 (组间)
#define FLOWGRAPH_SIDEBAR_BTN_Y5 494  // 导入 (组内)
#define FLOWGRAPH_SIDEBAR_BTN_Y6 594  // 自定义 (组间)
#define FLOWGRAPH_SIDEBAR_BTN_Y7 694  // 帮助 (组间)
#define FLOWGRAPH_SIDEBAR_BTN_COUNT 8 // ★ 修改：按钮总数改为8
#define FLOWGRAPH_ADD_PANEL_WIDTH 230
#define FLOWGRAPH_ADD_PANEL_ITEM_H 38
#define FLOWGRAPH_ADD_PANEL_HEADER 24
#define FLOWGRAPH_ADD_PANEL_COUNT 8
#define FLOWGRAPH_CHAIN_PANEL_WIDTH 220
#define FLOWGRAPH_CHAIN_PANEL_ITEM_H 34
#define FLOWGRAPH_CHAIN_PANEL_MAX_VISIBLE 10
#define FLOWGRAPH_CHAIN_PANEL_HEADER 24

	// ================= 自定义面板常量 =================
#define FLOWGRAPH_CUSTOM_PANEL_WIDTH 200
#define FLOWGRAPH_CUSTOM_PANEL_ITEM_H 34
#define FLOWGRAPH_CUSTOM_PANEL_MAX_VISIBLE 10
#define FLOWGRAPH_CUSTOM_PANEL_HEADER 24

// ★ 新增：取消面板常量
#define FLOWGRAPH_CANCEL_PANEL_WIDTH 220
#define FLOWGRAPH_CANCEL_PANEL_ITEM_H 34
#define FLOWGRAPH_CANCEL_PANEL_MAX_VISIBLE 10
#define FLOWGRAPH_CANCEL_PANEL_HEADER 24

	// ================= 双按钮点击位置 =================
#define FLOWGRAPH_DUAL_BUTTON_LEFT   1
#define FLOWGRAPH_DUAL_BUTTON_RIGHT  2

// ================= 参考图端口限制 =================
#define FLOWGRAPH_REF_IMAGE_MIN_REFS  1
#define FLOWGRAPH_REF_IMAGE_MAX_REFS  10


// ================= 连线槽类型 =================
// 节点图_连线槽类型_输入
#define FLOWGRAPH_SLOTTYPE_INPUT  1
// 节点图_连线槽类型_输出
#define FLOWGRAPH_SLOTTYPE_OUTPUT 2

// ================= LONG偏移 =================
// 节点图_长整型_数据指针
#define FLOWGRAPH_LONG_DATA               0
// 节点图_长整型_鼠标X
#define FLOWGRAPH_LONG_MOUSE_X            1
// 节点图_长整型_鼠标Y
#define FLOWGRAPH_LONG_MOUSE_Y            2
// 节点图_长整型_背景颜色
#define FLOWGRAPH_LONG_BACKGROUNDCOLOR    3

// ==================== 节点执行状态 ====================
// 节点图_执行状态_未执行
#define FLOWGRAPH_EXEC_STATUS_IDLE      0
// 节点图_执行状态_等待执行(排队中)
#define FLOWGRAPH_EXEC_STATUS_PENDING    1
// 节点图_执行状态_正在执行
#define FLOWGRAPH_EXEC_STATUS_RUNNING    2
// 节点图_执行状态_执行完成
#define FLOWGRAPH_EXEC_STATUS_COMPLETED  3
// 节点图_执行状态_执行失败
#define FLOWGRAPH_EXEC_STATUS_FAILED     4

// ==================== 执行模式 ====================
// 节点图_执行模式_从头执行(清除状态重新执行)
#define FLOWGRAPH_EXECUTE_MODE_FRESH     0
// 节点图_执行模式_继续执行(跳过COMPLETED节点)
#define FLOWGRAPH_EXECUTE_MODE_CONTINUE  1

// ================= 浮动编辑面板常量 =================
#define FLOWGRAPH_EDIT_PANEL_WIDTH  600
#define FLOWGRAPH_EDIT_PANEL_HEIGHT 300

#define FLOWGRAPH_REF_VIDEO_MIN_REFS  1
#define FLOWGRAPH_REF_VIDEO_MAX_REFS  10

// ================= 视频控件布局常量 =================
#define FLOWGRAPH_VIDEO_CTRL_HEIGHT  28
#define FLOWGRAPH_VIDEO_PROGRESS_H   5

// ================= 右键菜单面板常量 =================
#define FLOWGRAPH_CONTEXT_MENU_WIDTH 120
#define FLOWGRAPH_CONTEXT_MENU_ITEM_H 32
#define FLOWGRAPH_CONTEXT_MENU_HEADER 8

// ==================== 链式执行上下文 ====================
#pragma pack(4)
struct EX_FLOWGRAPH_CHAIN_CTX {
	INT chainEndNode;       // 终点节点ID
	INT* nodeOrder;         // 拓扑排序后的节点ID数组(上游在前)
	INT nodeOrderCount;     // 节点数量
	INT currentIndex;       // 当前执行位置
	INT executionPass;      // 执行趟次
	INT executeMode;        // 执行模式 (FLOWGRAPH_EXECUTE_MODE_)
};
#pragma pack()


// ==================== 画布核心数据(内部使用) ====================
#pragma pack(4)
struct EX_FLOWGRAPH_DATA {
	FLOAT zoom;             // 缩放比例
	POINTF panOffset;       // 平移偏移量
	INT selectedNode;       // 选中的节点ID (-1无)
	INT draggingNode;       // 拖拽中的节点ID (-1无)
	POINTF dragStartPos;    // 拖拽起始坐标
	INT connectingSlot;     // 正在连线的端口索引 (-1无)
	INT connectingNode;     // 正在连线的节点ID (-1无)
	INT connectingSlotType; // 正在连线的端口槽类型 (FLOWGRAPH_SLOTTYPE_)
	INT hoverNode;          // 鼠标悬停的节点ID (-1无)
	INT hoverSlot;          // 鼠标悬停的端口索引 (-1无)
	INT hoverSlotType;      // 鼠标悬停的端口槽类型
	EX_FLOWGRAPH_NODE* nodes; // 节点数组
	INT nodeCount;          // 节点数量
	EX_FLOWGRAPH_CONNECTION* connections; // 连线数组
	INT connectionCount;    // 连线数量
	INT selectedConnection; // 选中的连线ID (-1无)
	BOOL draggingControlPoint; // 是否正在拖拽控制点
	INT draggingWhichPoint; // 拖拽的控制点(1或2)
	INT selectedPortNode;   // 选中的端口所属节点ID (-1无)
	INT selectedPortIndex;  // 选中的端口索引 (-1无)
	INT executionDepth;     // 递归执行深度(防死循环)
	INT executionPass;      // 执行趟次计数器
	INT resizingNode;       // 正在调整大小的节点ID (-1无)
	INT resizingPortIdx;    // 正在调整大小的端口索引 (-1无)
	BOOL isPanning;         // 是否正在中键拖拽画布
	INT panStartMouseX;     // 中键拖拽起始鼠标X
	INT panStartMouseY;     // 中键拖拽起始鼠标Y
	INT panStartScrollX;    // 中键拖拽起始滚动X
	INT panStartScrollY;    // 中键拖拽起始滚动Y
	EX_FLOWGRAPH_CHAIN_CTX* chainContexts; // 链式执行上下文数组
	INT chainContextCount;  // 上下文数量
	INT asyncPendingNode;   // 异步等待中的节点ID (-1无)
	INT nextAutoId;         // 自增ID计数器
	INT hoverTitleNode;     // 悬停在标题区域的节点ID (-1无)
	INT sidebarHoverBtn;    // 侧边栏悬停按钮 (-1=无, 0=添加, 1=继续执行, 2=全部执行, 3=链路从头, 4=链路继续, 5=导出, 6=导入)
	BOOL sidebarShowAddPanel; // 是否显示添加面板
	INT sidebarAddPanelHover; // 添加面板悬停项 (-1=无, 0-5) 
	BOOL sidebarShowChainPanel; // 是否显示链路节点选择面板 
	INT sidebarChainPanelHover; // 链路面板悬停项索引 (-1=无)
	INT sidebarChainPanelMode;  // 0=从头, 1=继续
	libvlc_instance_t* libVlc;          // 共享VLC实例
	BOOL videoTimerActive;              // 视频更新定时器是否激活
	INT videoDragNode;                  // 正在拖拽进度的视频节点ID (-1无)
	INT videoDragPortIdx;               // 正在拖拽进度的视频端口索引 (-1无)
	LPCWSTR* customItems;          // 自定义面板条目名称数组
	INT customItemCount;           // 自定义条目数量
	BOOL sidebarShowCustomPanel;   // 是否显示自定义面板
	INT sidebarCustomPanelHover;   // 自定义面板悬停项索引 (-1=无)
	EX_FLOWGRAPH_CARD_DESCRIPTOR* cardRegistry;  // 卡片类型注册表
	INT cardRegistryCount;                       // 注册的卡片类型数量
	BOOL sidebarShowCancelPanel;
	INT sidebarCancelPanelHover;
	BOOL showEditPanel;          // 是否显示浮动编辑面板
	INT editPanelTargetNode;     // 面板目标节点ID (-1无)
	HEXOBJ editPanelEdit;        // 面板编辑框句柄
	BOOL editPanelBtnHover;      // 面板提交按钮悬停

	INT comboExpandedNode;    // 当前展开下拉面板的节点ID (-1表示无)
	INT comboExpandedPortIdx; // 当前展开下拉面板的端口索引
	INT comboPanelHoverIdx;   // 下拉面板中悬停的选项索引
	INT clipboardNodeId;     // ★ 新增：剪贴板节点ID (-1表示无)

	// ★ 新增：多选与框选状态
	BOOL isSelecting;           // 是否正在拖拽框选
	POINTF selectStartPos;      // 框选起始虚拟坐标
	POINTF selectEndPos;        // 框选结束虚拟坐标
	INT* selectedNodes;         // 当前选中的节点ID数组
	INT selectedNodeCount;      // 选中的节点数量
	BOOL isDraggingSelection;   // ★ 新增：是否正在拖拽多选节点组

	// ★ 新增：多节点剪贴板数据
	EX_FLOWGRAPH_NODE* clipboardNodes;
	INT clipboardNodeCount;
	EX_FLOWGRAPH_CONNECTION* clipboardConnections;
	INT clipboardConnectionCount;
	POINTF clipboardCenter;     // 复制时的包围盒中心点

	BOOL showContextMenu;       // 是否显示右键菜单
	INT contextMenuNodeId;      // 右键菜单关联的节点ID
	FLOAT contextMenuX;         // 菜单弹出X坐标(客户区)
	FLOAT contextMenuY;         // 菜单弹出Y坐标(客户区)
	INT contextMenuHover;       // 菜单悬停项索引 (0:从头, 1:继续, 2:取消)

	WCHAR projectName[256];     // ★ 新增：当前工程名(YAML文件名去后缀)
};
#pragma pack()


// ================= 函数声明 =================
void _flowgraph_register();
LRESULT CALLBACK _flowgraph_proc(HWND hWnd, HEXOBJ hObj, INT uMsg, WPARAM wParam, LPARAM lParam);
void _flowgraph_paint(HEXOBJ hObj);
void _flowgraph_updatelayout(HEXOBJ hObj);
void _flowgraph_onmousemove(HEXOBJ hObj, INT x, INT y);
void _flowgraph_onlbuttondown(HEXOBJ hObj, INT x, INT y);
void _flowgraph_onlbuttonup(HEXOBJ hObj, INT x, INT y);
void _flowgraph_onmousewheel(HEXOBJ hObj, SHORT delta);
void _flowgraph_onscrollbar(HEXOBJ hObj, INT uMsg, WPARAM wParam, LPARAM lParam);
INT _flowgraph_addnode(HEXOBJ hObj, EX_FLOWGRAPH_NODE* pNode);
INT _flowgraph_removenode(HEXOBJ hObj, INT nodeId);
INT _flowgraph_addconnection(HEXOBJ hObj, EX_FLOWGRAPH_CONNECTION* pConn);
INT _flowgraph_removeconnection(HEXOBJ hObj, INT connId);
void _flowgraph_calcnodesize(HEXOBJ hObj, EX_FLOWGRAPH_NODE* node);
EX_FLOWGRAPH_NODE* _flowgraph_findnode(EX_FLOWGRAPH_DATA* pData, INT nodeId);
EX_FLOWGRAPH_CONNECTION* _flowgraph_findconnection(EX_FLOWGRAPH_DATA* pData, INT connId);
void _flowgraph_initcontrolpoint(EX_FLOWGRAPH_CONNECTION* conn, POINTF virtualFromPt, POINTF virtualToPt);
FLOAT _flowgraph_dist_to_segment(POINTF p, POINTF v, POINTF w);
void _flowgraph_drawnode(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, EX_FLOWGRAPH_NODE* node, BOOL selected, FLOAT zoom, INT scrollX, INT scrollY);
void _flowgraph_draw_triangle_arrow(HEXCANVAS hCanvas, HEXBRUSH hBrush, FLOAT x, FLOAT y, FLOAT size, BOOL left);
EXARGB _flowgraph_get_port_color(INT dataType);
POINTF _flowgraph_get_port_center(EX_FLOWGRAPH_NODE* node, INT slotIndex);
// 执行相关
void _flowgraph_executenode(HEXOBJ hObj, INT mode, INT nodeId);
void _flowgraph_execute_chain(HEXOBJ hObj, INT mode, INT nodeId);
void _flowgraph_execute_by_cardtype(HEXOBJ hObj, EX_FLOWGRAPH_DATA* pData, EX_FLOWGRAPH_NODE* node);
void _flowgraph_copy_input_data(EX_FLOWGRAPH_NODE* node, EX_FLOWGRAPH_DATA* pData);
void _flowgraph_execute_all(HEXOBJ hObj, INT mode);
void _flowgraph_clear_data(EX_FLOWGRAPH_DATA* pData);
BOOL _flowgraph_export_to_yaml(HEXOBJ hObj, LPCWSTR filePath);
BOOL _flowgraph_import_from_yaml(HEXOBJ hObj, LPCWSTR filePath);
INT _flowgraph_node_execution_complete(HEXOBJ hObj, EX_FLOWGRAPH_ASYNC_RESULT* result);
void _flowgraph_execute_chain_step(HEXOBJ hObj, EX_FLOWGRAPH_DATA* pData, EX_FLOWGRAPH_CHAIN_CTX* ctx);
EX_FLOWGRAPH_CHAIN_CTX* _flowgraph_find_chain_context(EX_FLOWGRAPH_DATA* pData, INT nodeId);
void _flowgraph_remove_chain_context(EX_FLOWGRAPH_DATA* pData, INT chainEndNode);
void _flowgraph_save_chain_context(EX_FLOWGRAPH_DATA* pData, EX_FLOWGRAPH_CHAIN_CTX* ctx);
void _flowgraph_topo_visit(EX_FLOWGRAPH_DATA* pData, INT nodeId, BOOL* visited, INT* order, INT* orderCount);
void _flowgraph_avoid_overlap(EX_FLOWGRAPH_DATA* pData, EX_FLOWGRAPH_NODE* newNode);
void _flowgraph_draw_node_tooltip(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData,
	EX_FLOWGRAPH_NODE* node, FLOAT mouseX, FLOAT mouseY, FLOAT canvasWidth, FLOAT canvasHeight);
EXARGB _flowgraph_get_status_color(INT status);
LPCWSTR _flowgraph_get_status_text(INT status);
LPCWSTR _flowgraph_get_cardtype_name(EX_FLOWGRAPH_DATA* pData, INT cardType);
void _flowgraph_invalidate_downstream(EX_FLOWGRAPH_DATA* pData, INT nodeId);
void _flowgraph_draw_sidebar(HEXOBJ hObj, HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, FLOAT canvasWidth, FLOAT canvasHeight);
void _flowgraph_draw_add_panel(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, FLOAT canvasWidth, FLOAT canvasHeight);
void _flowgraph_draw_chain_panel(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, FLOAT canvasWidth, FLOAT canvasHeight);
void _flowgraph_draw_help_tooltip(HEXOBJ hObj, HEXCANVAS hCanvas, FLOAT canvasWidth, FLOAT canvasHeight);
void _flowgraph_free_temp_node_data(EX_FLOWGRAPH_NODE* node);

// 视频相关
void _flowgraph_video_cleanup(EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo);
void _flowgraph_video_load(EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo, LPCWSTR filePath, BOOL bLoadOnly);
void _flowgraph_draw_video_widget(HEXCANVAS hCanvas, EX_FLOWGRAPH_NODE* node, EX_FLOWGRAPH_PORT* port, FLOAT zoom, INT scrollX, INT scrollY);
void _flowgraph_format_video_time(INT64 ms, WCHAR* buf, INT bufSize);
void _flowgraph_update_video_timers(HEXOBJ hObj, EX_FLOWGRAPH_DATA* pData);
EX_FLOWGRAPH_NODE_VIDEO_DATA* _flowgraph_create_video_data(libvlc_instance_t* libVlc, HEXOBJ hFlowGraphObj);
// VLC回调
unsigned int _flowgraph_video_format_cb(void** object, char* chroma, unsigned int* width, unsigned int* height, unsigned int* pitches, unsigned int* lines);
void* _flowgraph_video_lock_cb(void* object, void** planes);
void _flowgraph_video_unlock_cb(void* object, void* picture, void* const* planes);
void _flowgraph_video_display_cb(void* object, void* picture);
void _flowgraph_video_event_cb(const libvlc_event_t* event, void* object);
INT _flowgraph_set_custom_items(HEXOBJ hObj, EX_FLOWGRAPH_CUSTOM_ITEMS* pItems);
void _flowgraph_draw_custom_panel(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, FLOAT canvasWidth, FLOAT canvasHeight);

INT _flowgraph_register_card_type(HEXOBJ hObj, EX_FLOWGRAPH_CARD_DESCRIPTOR* pDesc);
INT _flowgraph_unregister_card_type(HEXOBJ hObj, INT cardType);
INT _flowgraph_create_custom_node(HEXOBJ hObj, EX_FLOWGRAPH_CUSTOM_NODE_CREATE* p);
EX_FLOWGRAPH_CARD_DESCRIPTOR* _flowgraph_find_card_descriptor(EX_FLOWGRAPH_DATA* pData, INT cardType);
LPVOID _flowgraph_copy_widget_data(INT widgetType, LPVOID srcData);
void _flowgraph_free_card_descriptor(EX_FLOWGRAPH_CARD_DESCRIPTOR* desc);
void _flowgraph_adjust_image_size(EX_FLOWGRAPH_PORT* port);

INT _flowgraph_remove_dynamic_port(HEXOBJ hObj, INT nodeId);
INT _flowgraph_add_dynamic_port(HEXOBJ hObj, INT nodeId);
EX_FLOWGRAPH_NODE* _flowgraph_find_topmost_node_at(EX_FLOWGRAPH_DATA* pData, FLOAT virtualX, FLOAT virtualY);

void _flowgraph_cancel_node(HEXOBJ hObj, INT nodeId);
BOOL _flowgraph_is_connection_protected_by_busy(EX_FLOWGRAPH_DATA* pData, INT connId);
BOOL _flowgraph_is_node_protected_by_busy(EX_FLOWGRAPH_DATA* pData, INT nodeId);
BOOL _flowgraph_is_upstream_of(EX_FLOWGRAPH_DATA* pData, INT nodeId, INT targetId);
BOOL _flowgraph_is_upstream_of_internal(EX_FLOWGRAPH_DATA* pData, INT nodeId, INT targetId, BOOL* visited);
void _flowgraph_draw_cancel_panel(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, FLOAT canvasWidth, FLOAT canvasHeight);

void _flowgraph_update_edit_panel(HEXOBJ hObj);
void _flowgraph_draw_edit_panel(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, HEXOBJ hObj, FLOAT canvasWidth, FLOAT canvasHeight);
void _flowgraph_editpanel_submit(HEXOBJ hObj);
void _flowgraph_get_edit_panel_rect(HEXOBJ hObj, FLOAT* outX, FLOAT* outY, FLOAT* outW, FLOAT* outH);

BOOL _flowgraph_card_type_has_edit_panel(EX_FLOWGRAPH_NODE* node);
void _flowgraph_get_edit_panel_rect(HEXOBJ hObj, FLOAT* outX, FLOAT* outY, FLOAT* outW, FLOAT* outH);
void _flowgraph_update_edit_panel(HEXOBJ hObj);
void _flowgraph_editpanel_submit(HEXOBJ hObj);
void _flowgraph_draw_edit_panel(HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData, HEXOBJ hObj, FLOAT canvasWidth, FLOAT canvasHeight);

LRESULT CALLBACK _flowgraph_edit_at_triggered(HEXOBJ hObj, INT nID, INT nCode, WPARAM wParam, LPARAM lParam);

INT _flowgraph_add_dynamic_port2(HEXOBJ hObj, INT nodeId);
INT _flowgraph_remove_dynamic_port2(HEXOBJ hObj, INT nodeId);
void _flowgraph_draw_context_menu(HEXOBJ hObj, HEXCANVAS hCanvas, EX_FLOWGRAPH_DATA* pData);
void _flowgraph_clear_node_outputs(HEXOBJ hObj, EX_FLOWGRAPH_NODE* node);


void _flowgraph_update_selection_rect(EX_FLOWGRAPH_DATA* pData);
BOOL _flowgraph_is_node_selected(EX_FLOWGRAPH_DATA* pData, INT nodeId);
void _flowgraph_add_to_selection(EX_FLOWGRAPH_DATA* pData, INT nodeId);
void _flowgraph_clear_selection(EX_FLOWGRAPH_DATA* pData);

HEXIMAGE _flowgraph_load_image_no_lock(LPCWSTR filePath);