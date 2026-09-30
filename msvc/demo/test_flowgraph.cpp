#include "test_flowgraph.h"
HEXOBJ hFlowGraph;
HWND hWnd_flowgraph;

static INT g_nodeIdLocalText;
static INT g_nodeIdLLMText;
static INT g_nodeIdTextRender;
static INT g_nodeIdT2I1;
static INT g_nodeIdT2I2;
static INT g_nodeIdLocalImg;
static INT g_nodeIdRefImg;
static INT g_nodeIdT2V;
static INT g_nodeIdRefV;
static INT g_nodeIdLocalAudio;

// 辅助：std::vector<std::wstring> 转 LPCWSTR* 数组
LPCWSTR* VecToLPCWSTRArray(const std::vector<std::wstring>& vec) {
	LPCWSTR* arr = (LPCWSTR*)Ex_MemAlloc(sizeof(LPCWSTR) * vec.size());
	for (size_t i = 0; i < vec.size(); i++) arr[i] = vec[i].c_str();
	return arr;
}

void CreateNodes()
{
	// 1. 本地文本
	EX_FLOWGRAPH_CUSTOM_NODE_CREATE localText = { 100, 100, L"本地文本节点", FLOWGRAPH_CARD_TYPE_LOCAL_TEXT };
	g_nodeIdLocalText = (INT)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_CREATE_CUSTOM_NODE, 0, (LPARAM)&localText);

	// 2. 大模型文本
	EX_FLOWGRAPH_CUSTOM_NODE_CREATE llmText = { 100, 400, L"大模型文本节点", FLOWGRAPH_CARD_TYPE_LLM_TEXT };
	g_nodeIdLLMText = (INT)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_CREATE_CUSTOM_NODE, 0, (LPARAM)&llmText);

	// 3. 文本显示
	EX_FLOWGRAPH_CUSTOM_NODE_CREATE textRender = { 500, 200, L"文本显示节点", FLOWGRAPH_CARD_TYPE_TEXT_RENDER };
	g_nodeIdTextRender = (INT)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_CREATE_CUSTOM_NODE, 0, (LPARAM)&textRender);

	// 4. 文生图1
	EX_FLOWGRAPH_CUSTOM_NODE_CREATE t2i1 = { 500, 450, L"文生图节点", FLOWGRAPH_CARD_TYPE_TEXT_TO_IMAGE };
	g_nodeIdT2I1 = (INT)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_CREATE_CUSTOM_NODE, 0, (LPARAM)&t2i1);

	// 5. 文生图2
	EX_FLOWGRAPH_CUSTOM_NODE_CREATE t2i2 = { 900, 400, L"文生图节点", FLOWGRAPH_CARD_TYPE_TEXT_TO_IMAGE };
	g_nodeIdT2I2 = (INT)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_CREATE_CUSTOM_NODE, 0, (LPARAM)&t2i2);

	// 6. 本地图
	EX_FLOWGRAPH_CUSTOM_NODE_CREATE localImg = { 900, 750, L"本地图节点", FLOWGRAPH_CARD_TYPE_LOCAL_IMAGE };
	g_nodeIdLocalImg = (INT)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_CREATE_CUSTOM_NODE, 0, (LPARAM)&localImg);

	// 7. 参考生图
	EX_FLOWGRAPH_CUSTOM_NODE_CREATE refImg = { 1300, 300, L"参考生图节点", FLOWGRAPH_CARD_TYPE_REF_IMAGE };
	g_nodeIdRefImg = (INT)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_CREATE_CUSTOM_NODE, 0, (LPARAM)&refImg);

	// 8. 文生视频
	EX_FLOWGRAPH_CUSTOM_NODE_CREATE t2v = { 500, 750, L"文生视频节点", FLOWGRAPH_CARD_TYPE_TEXT_TO_VIDEO };
	g_nodeIdT2V = (INT)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_CREATE_CUSTOM_NODE, 0, (LPARAM)&t2v);

	// 9. 参考生视频
	EX_FLOWGRAPH_CUSTOM_NODE_CREATE refV = { 900, 750, L"参考生视频节点", FLOWGRAPH_CARD_TYPE_REF_IMAGE_TO_VIDEO };
	g_nodeIdRefV = (INT)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_CREATE_CUSTOM_NODE, 0, (LPARAM)&refV);

    // 10. 本地音频
    EX_FLOWGRAPH_CUSTOM_NODE_CREATE localAudio = { 1300, 750, L"本地音频节点", FLOWGRAPH_CARD_TYPE_LOCAL_AUDIO };
    g_nodeIdLocalAudio = (INT)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_CREATE_CUSTOM_NODE, 0, (LPARAM)&localAudio);
}
void AddConnections()
{
	EX_FLOWGRAPH_CONNECTION conn = { 0 };

	// 本地文本 → 文生图1 (output[1]→input[0])
	conn.fromNode = g_nodeIdLocalText; conn.fromSlot = 1; conn.toNode = g_nodeIdT2I1; conn.toSlot = 0;
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_ADD_CONNECTION, 0, (LPARAM)&conn);

	// 本地文本 → 大模型文本(输入文本端口) (output[1]→input[0])
	conn.fromNode = g_nodeIdLocalText; conn.fromSlot = 1; conn.toNode = g_nodeIdLLMText; conn.toSlot = 0;
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_ADD_CONNECTION, 0, (LPARAM)&conn);

	// 大模型文本 → 文本显示节点 (output[3]→input[0])
	conn.fromNode = g_nodeIdLLMText; conn.fromSlot = 3; conn.toNode = g_nodeIdTextRender; conn.toSlot = 0;
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_ADD_CONNECTION, 0, (LPARAM)&conn);

	// 文本显示 → 文生图2 (output[3]→input[0])
	conn.fromNode = g_nodeIdTextRender; conn.fromSlot = 3; conn.toNode = g_nodeIdT2I2; conn.toSlot = 0;
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_ADD_CONNECTION, 0, (LPARAM)&conn);

	// 文本显示 → 参考生图(文本端口[0])
	conn.fromNode = g_nodeIdTextRender; conn.fromSlot = 3; conn.toNode = g_nodeIdRefImg; conn.toSlot = 0;
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_ADD_CONNECTION, 0, (LPARAM)&conn);

	// 本地图 → 参考生图(参考图片0) ★ toSlot=7 (FIXED_PORTS=7)
	conn.fromNode = g_nodeIdLocalImg; conn.fromSlot = 2; conn.toNode = g_nodeIdRefImg; conn.toSlot = 7;
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_ADD_CONNECTION, 0, (LPARAM)&conn);

	// 本地文本 → 文生视频 (output[1]→input[0])
	conn.fromNode = g_nodeIdLocalText; conn.fromSlot = 1; conn.toNode = g_nodeIdT2V; conn.toSlot = 0;
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_ADD_CONNECTION, 0, (LPARAM)&conn);

	// 文生图1 → 参考生视频(参考图片0) ★ fromSlot=6(T2I output), toSlot=8(REF_VIDEO FIXED_PORTS=8)
	conn.fromNode = g_nodeIdT2I1; conn.fromSlot = 6; conn.toNode = g_nodeIdRefV; conn.toSlot = 8;
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_ADD_CONNECTION, 0, (LPARAM)&conn);

	// 文本显示 → 参考生视频(文本端口) toSlot=0
	conn.fromNode = g_nodeIdTextRender; conn.fromSlot = 3; conn.toNode = g_nodeIdRefV; conn.toSlot = 0;
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_ADD_CONNECTION, 0, (LPARAM)&conn);

    // ★ 给参考生视频手动增加1个音频端口 (因为Min=0，默认没有)
    Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_ADD_DYNAMIC_PORT2, g_nodeIdRefV, 0);
    // 本地音频 → 参考生视频(参考音频0) 
    // 注意：此时参考生视频的端口布局为：0文本, 1图控制, 2音频控制, 3模型, 4分辨率, 5内容, 6视频, 7保存, 8图片0, 9音频0, 10输出
    conn.fromNode = g_nodeIdLocalAudio; conn.fromSlot = 2; conn.toNode = g_nodeIdRefV; conn.toSlot = 9;
    Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_ADD_CONNECTION, 0, (LPARAM)&conn);
}

// ==================== 调试辅助：打印端口IO数据 ====================
void DebugPrintIOData(LPCWSTR tag, EX_FLOWGRAPH_NODE* node, EX_FLOWGRAPH_NODE_IO_DATA* arr, INT count)
{
    for (INT i = 0; i < count; i++)
    {
        // ★ 查找对应端口的 widgetType，以正确解析 data 指针
        INT widgetType = 0;
        if (node) {
            for (INT j = 0; j < node->portCount; j++) {
                if (node->ports[j].id == arr[i].portId) {
                    widgetType = node->ports[j].widgetType;
                    break;
                }
            }
        }

        switch (arr[i].dataType)
        {
        case FLOWGRAPH_DATATYPE_STRING:
        {
            // ★ 如果是按钮组件，data 是结构体指针，需解析 caption
            if (widgetType == FLOWGRAPH_NODEDATA_TYPE_BUTTON) {
                EX_FLOWGRAPH_NODE_BUTTON_DATA* btn = (EX_FLOWGRAPH_NODE_BUTTON_DATA*)arr[i].data;
                LPCWSTR caption = (btn && btn->caption) ? btn->caption : L"(null)";
                OUTPUTW(tag, L"  [", i, L"] portId=", arr[i].portId,
                    L", type=STRING(BUTTON), caption=\"", caption, L"\"");
            }
            else if (widgetType == FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON) {
                EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA* dual = (EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA*)arr[i].data;
                LPCWSTR cap1 = (dual && dual->caption1) ? dual->caption1 : L"(null)";
                LPCWSTR cap2 = (dual && dual->caption2) ? dual->caption2 : L"(null)";
                OUTPUTW(tag, L"  [", i, L"] portId=", arr[i].portId,
                    L", type=STRING(DUAL_BTN), cap1=\"", cap1, L"\", cap2=\"", cap2, L"\"");
            }
            else {
                LPCWSTR str = arr[i].data ? (LPCWSTR)arr[i].data : L"(null)";
                OUTPUTW(tag, L"  [", i, L"] portId=", arr[i].portId,
                    L", type=STRING, data=\"", str, L"\"");
            }
            break;
        }
        case FLOWGRAPH_DATATYPE_IMAGE:
        {
            OUTPUTW(tag, L"  [", i, L"] portId=", arr[i].portId,
                L", type=IMAGE, data=0x", (LPVOID)arr[i].data);
            break;
        }
        case FLOWGRAPH_DATATYPE_COMBO:
        {
            if (arr[i].data)
            {
                EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)arr[i].data;
                OUTPUTW(tag, L"  [", i, L"] portId=", arr[i].portId,
                    L", type=COMBO, count=", combo->count,
                    L", current=", combo->current);
                for (INT k = 0; k < combo->count; k++)
                {
                    OUTPUTW(tag, L"    options[", k, L"]=\"",
                        combo->options[k] ? combo->options[k] : L"(null)", L"\"");
                }
            }
            else
            {
                OUTPUTW(tag, L"  [", i, L"] portId=", arr[i].portId,
                    L", type=COMBO, data=(null)");
            }
            break;
        }
        case FLOWGRAPH_DATATYPE_ANY:
        {
            OUTPUTW(tag, L"  [", i, L"] portId=", arr[i].portId,
                L", type=ANY, data=0x", (LPVOID)arr[i].data);
            break;
        }
        case FLOWGRAPH_DATATYPE_VIDEO:
        {
            OUTPUTW(tag, L"  [", i, L"] portId=", arr[i].portId,
                L", type=VIDEO, data=0x", (LPVOID)arr[i].data);
            break;
        }
        default:
        {
            OUTPUTW(tag, L"  [", i, L"] portId=", arr[i].portId,
                L", type=UNKNOWN(", arr[i].dataType, L"), data=0x", (LPVOID)arr[i].data);
            break;
        }
        }
    }
}

// ==================== 按卡片类型分别处理事件 ====================
LRESULT CALLBACK FlowGraphNotifyProc(HEXOBJ hObj, INT nID, INT nCode, WPARAM wParam, LPARAM lParam)
{
	if (nID != 200) return 0;
	switch (nCode)
	{
    case FLOWGRAPH_EVENT_EXECUTE_NODE:
    {
        EX_FLOWGRAPH_EXECUTE_PARAMS* params = (EX_FLOWGRAPH_EXECUTE_PARAMS*)lParam;
        EX_FLOWGRAPH_NODE* node = (EX_FLOWGRAPH_NODE*)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_FIND_NODE, 0, params->nodeId);
        // ===== 通用打印：输入/输出端口概览 =====
        OUTPUTW(L"══════════════════════════════════════════");
        OUTPUTW(L"[执行事件] nodeId=", params->nodeId,
            L", cardType=", params->cardType);
        OUTPUTW(L"── 输入端口 (count=", params->inputCount, L") ──");
        DebugPrintIOData(L"[INPUT]", node, params->inputs, params->inputCount); // ★ 传入 node
        OUTPUTW(L"── 输出端口 (count=", params->outputCount, L") ──");
        DebugPrintIOData(L"[OUTPUT]", node, params->outputs, params->outputCount);

        switch (params->cardType)
        {
            // ==================== 本地文本 ====================
        case FLOWGRAPH_CARD_TYPE_LOCAL_TEXT:
        {
            OUTPUTW(L"[本地文本] 端口布局: [0]INTERMEDIATE/TEXT(内容) [1]OUTPUT/STRING(输出文本)");

            // inputs[0] = INTERMEDIATE/TEXT("内容")
            LPCWSTR text = (LPCWSTR)params->inputs[0].data;
            OUTPUTW(L"[本地文本] inputs[0].data(内容)=\"", text ? text : L"(null)", L"\"");

            // outputs[0] = OUTPUT/STRING("输出文本")
            params->outputs[0].data = text ? (LPVOID)StrDupW(text) : NULL;
            OUTPUTW(L"[本地文本] outputs[0].data(输出)=\"",
                params->outputs[0].data ? (LPCWSTR)params->outputs[0].data : L"(null)", L"\"");
            break;
        }

        // ==================== 大模型文本 ====================
        case FLOWGRAPH_CARD_TYPE_LLM_TEXT:
        {
            OUTPUTW(L"[大模型文本] 端口布局: [0]INPUT/STRING(输入文本) [1]INTERMEDIATE/COMBO(模型) [2]INTERMEDIATE/TEXT(提示词) [3]OUTPUT/STRING(输出文本)");

            // inputs[0] = INPUT/STRING("输入文本")
            LPCWSTR inputText = (LPCWSTR)params->inputs[0].data;
            OUTPUTW(L"[大模型文本] inputs[0].data(输入文本)=\"", inputText ? inputText : L"(null)", L"\"");

            // inputs[1] = INTERMEDIATE/COMBO("模型")
            EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)params->inputs[1].data;
            if (combo)
            {
                OUTPUTW(L"[大模型文本] inputs[1].data(模型) count=", combo->count,
                    L", current=", combo->current,
                    L", value=\"", combo->options[combo->current] ? combo->options[combo->current] : L"(null)", L"\"");
                for (INT k = 0; k < combo->count; k++)
                {
                    OUTPUTW(L"  options[", k, L"]=\"", combo->options[k] ? combo->options[k] : L"(null)", L"\"");
                }
            }
            else
            {
                OUTPUTW(L"[大模型文本] inputs[1].data(模型)=(null)");
            }

            // inputs[2] = INTERMEDIATE/TEXT("提示词")
            LPCWSTR prompt = (LPCWSTR)params->inputs[2].data;
            OUTPUTW(L"[大模型文本] inputs[2].data(提示词)=\"", prompt ? prompt : L"(null)", L"\"");

            // 拼接完整提示
            std::wstring fullPrompt = (inputText ? inputText : L"");
            fullPrompt += L"\n";
            fullPrompt += (prompt ? prompt : L"");
            OUTPUTW(L"[大模型文本] 拼接后完整提示=\"", fullPrompt.c_str(), L"\"");

            if (!fullPrompt.empty())
            {
                Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_MARK_ASYNC_NODE, params->nodeId, 0);
                INT nodeId = params->nodeId;
                std::thread([nodeId, fullPrompt]() {
                    Sleep(3000);
                    EX_FLOWGRAPH_ASYNC_RESULT result = { 0 };
                    result.nodeId = nodeId;
                    result.cardType = FLOWGRAPH_CARD_TYPE_LLM_TEXT;
                    result.executionResult = FLOWGRAPH_EXEC_RESULT_SUCCESS;
                    result.outputText = L"大模型生成的内容大模型生成的内容大模型生成的内容大模型生成的内容大模型生成的内容";
                    Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_NODE_EXECUTION_COMPLETE, 0, (LPARAM)&result);
                    }).detach();
                OUTPUTW(L"[大模型文本] 已标记异步执行，等待回调...");
            }
            else
            {
                params->executionResult = FLOWGRAPH_EXEC_RESULT_FAILED;
                OUTPUTW(L"[大模型文本] 执行失败：完整提示为空");
            }
            break;
        }

        // ==================== 文生图 ====================
        case FLOWGRAPH_CARD_TYPE_TEXT_TO_IMAGE:
        {
            OUTPUTW(L"[文生图] 端口布局: [0]INPUT/STRING(输入文本) [1]INTERMEDIATE/COMBO(模型) [2]INTERMEDIATE/COMBO(分辨率) [3]INTERMEDIATE/TEXT(内容) [4]INTERMEDIATE/IMAGE(预览) [5]INTERMEDIATE/BUTTON(保存) [6]OUTPUT/IMAGE(输出图片)");

            // inputs[0] = INPUT/STRING("输入文本")
            LPCWSTR inputPrompt = (LPCWSTR)params->inputs[0].data;
            OUTPUTW(L"[文生图] inputs[0].data(输入文本)=\"", inputPrompt ? inputPrompt : L"(null)", L"\"");

            // inputs[1] = INTERMEDIATE/COMBO("模型")
            EX_FLOWGRAPH_NODE_COMBO_DATA* modelCombo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)params->inputs[1].data;
            if (modelCombo)
            {
                OUTPUTW(L"[文生图] inputs[1].data(模型) current=", modelCombo->current,
                    L", value=\"", modelCombo->options[modelCombo->current] ? modelCombo->options[modelCombo->current] : L"(null)", L"\"");
            }
            else
            {
                OUTPUTW(L"[文生图] inputs[1].data(模型)=(null)");
            }

            // inputs[2] = INTERMEDIATE/COMBO("分辨率")
            EX_FLOWGRAPH_NODE_COMBO_DATA* resCombo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)params->inputs[2].data;
            if (resCombo)
            {
                OUTPUTW(L"[文生图] inputs[2].data(分辨率) current=", resCombo->current,
                    L", value=\"", resCombo->options[resCombo->current] ? resCombo->options[resCombo->current] : L"(null)", L"\"");
            }
            else
            {
                OUTPUTW(L"[文生图] inputs[2].data(分辨率)=(null)");
            }

            // inputs[3] = INTERMEDIATE/TEXT("内容")
            LPCWSTR contentText = (LPCWSTR)params->inputs[3].data;
            OUTPUTW(L"[文生图] inputs[3].data(内容)=\"", contentText ? contentText : L"(null)", L"\"");

            // outputs[0] = OUTPUT/IMAGE("输出图片") - 空槽位，异步填充
            OUTPUTW(L"[文生图] outputs[0].data(输出图片) 待异步填充");

            if (inputPrompt && lstrlenW(inputPrompt) > 0)
            {
                Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_MARK_ASYNC_NODE, params->nodeId, 0);
                INT nodeId = params->nodeId;
                std::thread([nodeId]() {
                    Sleep(2000);
                    HEXIMAGE hImg = NULL;
                    _img_createfromfile(L"res/rotateimgbox.jpg", &hImg);
                    EX_FLOWGRAPH_ASYNC_RESULT result = { 0 };
                    result.nodeId = nodeId;
                    result.cardType = FLOWGRAPH_CARD_TYPE_TEXT_TO_IMAGE;
                    result.outputImage = hImg;
                    Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_NODE_EXECUTION_COMPLETE, 0, (LPARAM)&result);
                    }).detach();
                OUTPUTW(L"[文生图] 已标记异步执行，等待回调...");
            }
            else
            {
                params->executionResult = FLOWGRAPH_EXEC_RESULT_FAILED;
                OUTPUTW(L"[文生图] 执行失败：输入文本为空");
            }
            break;
        }

        // ==================== 本地图 ====================
        case FLOWGRAPH_CARD_TYPE_LOCAL_IMAGE:
        {
            OUTPUTW(L"[本地图] 端口布局: [0]INTERMEDIATE/BUTTON(选择图片) [1]INTERMEDIATE/IMAGE(预览) [2]OUTPUT/IMAGE(输出图片)");
            OUTPUTW(L"[本地图] 无输入端口(通过按钮选择图片)，默认透传");
            EX_FLOWGRAPH_NODE* node = (EX_FLOWGRAPH_NODE*)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_FIND_NODE, 0, params->nodeId);
            // inputs: 无INPUT端口，inputs为空或只有INTERMEDIATE
            // outputs: OUTPUT/IMAGE由widgetData携带
            OUTPUTW(L"[本地图] inputs count=", params->inputCount);
            DebugPrintIOData(L"[本地图 INPUT]", node, params->inputs, params->inputCount);
            break;
        }

        // ==================== 参考生图 ====================
        case FLOWGRAPH_CARD_TYPE_REF_IMAGE:
        {
            EX_FLOWGRAPH_NODE* node = (EX_FLOWGRAPH_NODE*)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_FIND_NODE, 0, params->nodeId);
            if (!node) { params->executionResult = FLOWGRAPH_EXEC_RESULT_FAILED; break; }

            // ★ 端口布局: [0]INPUT/STRING [1]INTERMEDIATE/DUAL_BUTTON [2]INTERMEDIATE/COMBO(模型)
            //   [3]INTERMEDIATE/COMBO(分辨率) [4]INTERMEDIATE/TEXT(内容) [5]INTERMEDIATE/IMAGE(预览)
            //   [6]INTERMEDIATE/BUTTON(保存) [7..7+N-1]INPUT/IMAGE(参考图) [last]OUTPUT/IMAGE
            OUTPUTW(L"[参考生图] 端口布局: [0]INPUT/STRING(文本) [1]DUAL_BUTTON(控制) [2]COMBO(模型) [3]COMBO(分辨率) [4]TEXT(内容) [5]IMAGE(预览) [6]BUTTON(保存) [7..]INPUT/IMAGE(参考图) [last]OUTPUT/IMAGE");
            OUTPUTW(L"[参考生图] 节点总端口数 portCount=", node->portCount);

            // 逐个端口打印
            for (INT i = 0; i < node->portCount; i++)
            {
                EX_FLOWGRAPH_PORT& p = node->ports[i];
                WCHAR portTypeStr[16] = { 0 };
                switch (p.portType) {
                case FLOWGRAPH_PORTTYPE_INPUT:       lstrcpyW(portTypeStr, L"INPUT"); break;
                case FLOWGRAPH_PORTTYPE_OUTPUT:      lstrcpyW(portTypeStr, L"OUTPUT"); break;
                case FLOWGRAPH_PORTTYPE_INTERMEDIATE: lstrcpyW(portTypeStr, L"INTER"); break;
                default: lstrcpyW(portTypeStr, L"?"); break;
                }

                WCHAR widgetTypeStr[16] = { 0 };
                switch (p.widgetType) {
                case 0: lstrcpyW(widgetTypeStr, L"NONE"); break;
                case FLOWGRAPH_NODEDATA_TYPE_EDIT:        lstrcpyW(widgetTypeStr, L"EDIT"); break;
                case FLOWGRAPH_NODEDATA_TYPE_TEXT:        lstrcpyW(widgetTypeStr, L"TEXT"); break;
                case FLOWGRAPH_NODEDATA_TYPE_COMBO:       lstrcpyW(widgetTypeStr, L"COMBO"); break;
                case FLOWGRAPH_NODEDATA_TYPE_IMAGE:       lstrcpyW(widgetTypeStr, L"IMAGE"); break;
                case FLOWGRAPH_NODEDATA_TYPE_BUTTON:      lstrcpyW(widgetTypeStr, L"BUTTON"); break;
                case FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON: lstrcpyW(widgetTypeStr, L"DUALBTN"); break;
                case FLOWGRAPH_NODEDATA_TYPE_VIDEO:       lstrcpyW(widgetTypeStr, L"VIDEO"); break;
                default: lstrcpyW(widgetTypeStr, L"?"); break;
                }

                // 按数据类型打印widgetData内容
                if (p.portType == FLOWGRAPH_PORTTYPE_INPUT || p.portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE)
                {
                    if (p.dataType == FLOWGRAPH_DATATYPE_STRING && p.widgetData)
                    {
                        OUTPUTW(L"  [", i, L"] ", portTypeStr, L"/", widgetTypeStr,
                            L" name=\"", p.name, L"\" portId=", p.id,
                            L" data=\"", (LPCWSTR)p.widgetData, L"\"",
                            L" connected=", p.isConnected);
                    }
                    else if (p.dataType == FLOWGRAPH_DATATYPE_IMAGE)
                    {
                        OUTPUTW(L"  [", i, L"] ", portTypeStr, L"/", widgetTypeStr,
                            L" name=\"", p.name, L"\" portId=", p.id,
                            L" data=0x", (LPVOID)p.widgetData,
                            L" connected=", p.isConnected);
                    }
                    else if (p.dataType == FLOWGRAPH_DATATYPE_COMBO && p.widgetData)
                    {
                        EX_FLOWGRAPH_NODE_COMBO_DATA* c = (EX_FLOWGRAPH_NODE_COMBO_DATA*)p.widgetData;
                        OUTPUTW(L"  [", i, L"] ", portTypeStr, L"/", widgetTypeStr,
                            L" name=\"", p.name, L"\" portId=", p.id,
                            L" count=", c->count, L" current=", c->current,
                            L" value=\"", c->options[c->current] ? c->options[c->current] : L"(null)", L"\"",
                            L" connected=", p.isConnected);
                    }
                    else if (p.dataType == FLOWGRAPH_DATATYPE_ANY)
                    {
                        OUTPUTW(L"  [", i, L"] ", portTypeStr, L"/", widgetTypeStr,
                            L" name=\"", p.name, L"\" portId=", p.id,
                            L" dataType=ANY",
                            L" connected=", p.isConnected);
                    }
                    else
                    {
                        OUTPUTW(L"  [", i, L"] ", portTypeStr, L"/", widgetTypeStr,
                            L" name=\"", p.name, L"\" portId=", p.id,
                            L" dataType=", p.dataType,
                            L" data=0x", (LPVOID)p.widgetData,
                            L" connected=", p.isConnected);
                    }
                }
                else
                {
                    // OUTPUT端口
                    OUTPUTW(L"  [", i, L"] ", portTypeStr, L"/", widgetTypeStr,
                        L" name=\"", p.name, L"\" portId=", p.id,
                        L" dataType=", p.dataType);
                }
            }

            // 提取关键参数
            LPCWSTR inputPrompt = node->ports[0].widgetData ? (LPCWSTR)node->ports[0].widgetData : L"";
            EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)node->ports[2].widgetData;
            EX_FLOWGRAPH_NODE_COMBO_DATA* resCombo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)node->ports[3].widgetData;
            LPCWSTR contentText = node->ports[4].widgetData ? (LPCWSTR)node->ports[4].widgetData : L"";

            INT lastIdx = node->portCount - 1;
            INT refCount = lastIdx - FLOWGRAPH_REF_IMAGE_FIXED_PORTS;

            OUTPUTW(L"[参考生图] FIXED_PORTS=", FLOWGRAPH_REF_IMAGE_FIXED_PORTS,
                L", refCount=", refCount,
                L", lastIdx=", lastIdx);

            // 统计参考图
            INT imgCount = 0;
            if (refCount > 0)
            {
                for (INT i = 0; i < refCount; i++)
                {
                    INT portIdx = FLOWGRAPH_REF_IMAGE_FIXED_PORTS + i;
                    BOOL hasData = (node->ports[portIdx].widgetData != NULL);
                    OUTPUTW(L"[参考生图] 参考图[", i, L"] portIdx=", portIdx,
                        L" name=\"", node->ports[portIdx].name, L"\"",
                        L" hasData=", hasData,
                        L" isConnected=", node->ports[portIdx].isConnected);
                    if (hasData) imgCount++;
                }
            }

            LPCWSTR modelName = (combo && combo->count > 0) ? combo->options[combo->current] : L"";
            LPCWSTR resolutionName = (resCombo && resCombo->count > 0) ? resCombo->options[resCombo->current] : L"";

            OUTPUTW(L"[参考生图] 输入文本=\"", inputPrompt, L"\"");
            OUTPUTW(L"[参考生图] 模型=\"", modelName, L"\"");
            OUTPUTW(L"[参考生图] 分辨率=\"", resolutionName, L"\"");
            OUTPUTW(L"[参考生图] 内容=\"", contentText, L"\"");
            OUTPUTW(L"[参考生图] 参考图数=", refCount, L", 有数据=", imgCount);

            // 打印inputs数组
            OUTPUTW(L"[参考生图] params inputs:");
            DebugPrintIOData(L"[参考生图 IN]", node, params->inputs, params->inputCount);

            if (lstrlenW(inputPrompt) > 0 && imgCount == refCount && refCount > 0)
            {
                Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_MARK_ASYNC_NODE, params->nodeId, 0);
                INT nodeId = params->nodeId;
                std::thread([nodeId]() {
                    Sleep(3000);
                    HEXIMAGE hImg = NULL;
                    _img_createfromfile(L"res/rotateimgbox.jpg", &hImg);
                    EX_FLOWGRAPH_ASYNC_RESULT result = { 0 };
                    result.nodeId = nodeId;
                    result.cardType = FLOWGRAPH_CARD_TYPE_REF_IMAGE;
                    result.outputImage = hImg;
                    Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_NODE_EXECUTION_COMPLETE, 0, (LPARAM)&result);
                    }).detach();
                OUTPUTW(L"[参考生图] 已标记异步执行，等待回调...");
            }
            else
            {
                params->executionResult = FLOWGRAPH_EXEC_RESULT_FAILED;
                OUTPUTW(L"[参考生图] 执行失败：输入文本长度=", lstrlenW(inputPrompt),
                    L", imgCount=", imgCount, L", refCount=", refCount);
            }
            break;
        }

        // ==================== 文本显示 ====================
        case FLOWGRAPH_CARD_TYPE_TEXT_RENDER:
        {
            OUTPUTW(L"[文本显示] 端口布局: [0]INPUT/STRING(输入文本) [1]INTERMEDIATE/TEXT(显示文本) [2]INTERMEDIATE/BUTTON(复制) [3]OUTPUT/STRING(输出文本)");

            // inputs[0] = INPUT/STRING("输入文本")
            LPCWSTR inputText = (LPCWSTR)params->inputs[0].data;
            OUTPUTW(L"[文本显示] inputs[0].data(输入文本)=\"", inputText ? inputText : L"(null)", L"\"");

            // 更新INTERMEDIATE/TEXT显示
            EX_FLOWGRAPH_NODE* node = (EX_FLOWGRAPH_NODE*)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_FIND_NODE, 0, params->nodeId);
            if (node)
            {
                for (INT i = 0; i < node->portCount; i++)
                {
                    if (node->ports[i].portType == FLOWGRAPH_PORTTYPE_INTERMEDIATE &&
                        node->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT)
                    {
                        OUTPUTW(L"[文本显示] 找到TEXT端口 index=", i,
                            L", portId=", node->ports[i].id,
                            L", name=\"", node->ports[i].name, L"\"");
                        EX_FLOWGRAPH_PORT newData = { 0 };
                        newData.id = node->ports[i].id;
                        newData.widgetType = FLOWGRAPH_NODEDATA_TYPE_TEXT;
                        newData.widgetData = (LPVOID)StrDupW(inputText ? inputText : L"");
                        Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_UPDATE_NODEDATA, params->nodeId, (LPARAM)&newData);
                        OUTPUTW(L"[文本显示] 已更新TEXT端口内容=\"", inputText ? inputText : L"(null)", L"\"");
                        break;
                    }
                }
            }

            // outputs[0] = OUTPUT/STRING("输出文本")
            params->outputs[0].data = inputText ? (LPVOID)StrDupW(inputText) : NULL;
            OUTPUTW(L"[文本显示] outputs[0].data(输出)=\"",
                params->outputs[0].data ? (LPCWSTR)params->outputs[0].data : L"(null)", L"\"");
            break;
        }

        // ==================== 文生视频 ====================
        case FLOWGRAPH_CARD_TYPE_TEXT_TO_VIDEO:
        {
            OUTPUTW(L"[文生视频] 端口布局: [0]INPUT/STRING(输入文本) [1]INTERMEDIATE/COMBO(模型) [2]INTERMEDIATE/COMBO(分辨率) [3]INTERMEDIATE/TEXT(内容) [4]INTERMEDIATE/VIDEO(视频预览) [5]INTERMEDIATE/BUTTON(保存) [6]OUTPUT/STRING(输出视频路径)");

            // inputs[0] = INPUT/STRING("输入文本")
            LPCWSTR inputPrompt = (LPCWSTR)params->inputs[0].data;
            OUTPUTW(L"[文生视频] inputs[0].data(输入文本)=\"", inputPrompt ? inputPrompt : L"(null)", L"\"");

            // inputs[1] = INTERMEDIATE/COMBO("模型")
            EX_FLOWGRAPH_NODE_COMBO_DATA* modelCombo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)params->inputs[1].data;
            if (modelCombo)
            {
                OUTPUTW(L"[文生视频] inputs[1].data(模型) current=", modelCombo->current,
                    L", value=\"", modelCombo->options[modelCombo->current] ? modelCombo->options[modelCombo->current] : L"(null)", L"\"");
                for (INT k = 0; k < modelCombo->count; k++)
                {
                    OUTPUTW(L"  options[", k, L"]=\"", modelCombo->options[k] ? modelCombo->options[k] : L"(null)", L"\"");
                }
            }
            else
            {
                OUTPUTW(L"[文生视频] inputs[1].data(模型)=(null)");
            }

            // inputs[2] = INTERMEDIATE/COMBO("分辨率")
            EX_FLOWGRAPH_NODE_COMBO_DATA* resCombo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)params->inputs[2].data;
            if (resCombo)
            {
                OUTPUTW(L"[文生视频] inputs[2].data(分辨率) current=", resCombo->current,
                    L", value=\"", resCombo->options[resCombo->current] ? resCombo->options[resCombo->current] : L"(null)", L"\"");
                for (INT k = 0; k < resCombo->count; k++)
                {
                    OUTPUTW(L"  options[", k, L"]=\"", resCombo->options[k] ? resCombo->options[k] : L"(null)", L"\"");
                }
            }
            else
            {
                OUTPUTW(L"[文生视频] inputs[2].data(分辨率)=(null)");
            }

            // inputs[3] = INTERMEDIATE/TEXT("内容")
            LPCWSTR contentText = (LPCWSTR)params->inputs[3].data;
            OUTPUTW(L"[文生视频] inputs[3].data(内容)=\"", contentText ? contentText : L"(null)", L"\"");

            // outputs[0] = OUTPUT/STRING("输出视频路径")
            OUTPUTW(L"[文生视频] outputs[0].data(输出视频路径) 待异步填充");

            if (inputPrompt && lstrlenW(inputPrompt) > 0)
            {
                Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_MARK_ASYNC_NODE, params->nodeId, 0);
                INT nodeId = params->nodeId;
                std::thread([nodeId]() {
                    Sleep(3000);
                    EX_FLOWGRAPH_ASYNC_RESULT result = { 0 };
                    result.nodeId = nodeId;
                    result.outputText = L"./res/test.mp4";
                    Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_NODE_EXECUTION_COMPLETE, 0, (LPARAM)&result);
                    }).detach();
                OUTPUTW(L"[文生视频] 已标记异步执行，等待回调...");
            }
            else
            {
                params->executionResult = FLOWGRAPH_EXEC_RESULT_FAILED;
                OUTPUTW(L"[文生视频] 执行失败：输入文本为空");
            }
            break;
        }

        // ==================== 参考生视频 ====================
        case FLOWGRAPH_CARD_TYPE_REF_IMAGE_TO_VIDEO:
        {
            EX_FLOWGRAPH_NODE* node = (EX_FLOWGRAPH_NODE*)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_FIND_NODE, 0, params->nodeId);
            if (!node) { params->executionResult = FLOWGRAPH_EXEC_RESULT_FAILED; break; }

            OUTPUTW(L"[参考生视频] 端口布局: [0]文本 [1]图控制 [2]音频控制 [3]模型 [4]分辨率 [5]内容 [6]视频 [7]保存 [8..]图片 [..]音频 [last]输出");
            OUTPUTW(L"[参考生视频] 节点总端口数 portCount=", node->portCount);

            // 逐个端口打印 (保持原样，动态遍历不会出错)
            for (INT i = 0; i < node->portCount; i++)
            {
                EX_FLOWGRAPH_PORT& p = node->ports[i];
                WCHAR portTypeStr[16] = { 0 };
                switch (p.portType) {
                case FLOWGRAPH_PORTTYPE_INPUT:       lstrcpyW(portTypeStr, L"INPUT"); break;
                case FLOWGRAPH_PORTTYPE_OUTPUT:      lstrcpyW(portTypeStr, L"OUTPUT"); break;
                case FLOWGRAPH_PORTTYPE_INTERMEDIATE: lstrcpyW(portTypeStr, L"INTER"); break;
                default: lstrcpyW(portTypeStr, L"?"); break;
                }

                WCHAR widgetTypeStr[16] = { 0 };
                switch (p.widgetType) {
                case 0: lstrcpyW(widgetTypeStr, L"NONE"); break;
                case FLOWGRAPH_NODEDATA_TYPE_EDIT:        lstrcpyW(widgetTypeStr, L"EDIT"); break;
                case FLOWGRAPH_NODEDATA_TYPE_TEXT:        lstrcpyW(widgetTypeStr, L"TEXT"); break;
                case FLOWGRAPH_NODEDATA_TYPE_COMBO:       lstrcpyW(widgetTypeStr, L"COMBO"); break;
                case FLOWGRAPH_NODEDATA_TYPE_IMAGE:       lstrcpyW(widgetTypeStr, L"IMAGE"); break;
                case FLOWGRAPH_NODEDATA_TYPE_BUTTON:      lstrcpyW(widgetTypeStr, L"BUTTON"); break;
                case FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON: lstrcpyW(widgetTypeStr, L"DUALBTN"); break;
                case FLOWGRAPH_NODEDATA_TYPE_VIDEO:       lstrcpyW(widgetTypeStr, L"VIDEO"); break;
                default: lstrcpyW(widgetTypeStr, L"?"); break;
                }

                if (p.dataType == FLOWGRAPH_DATATYPE_STRING && p.widgetData && (p.widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT || p.widgetType == FLOWGRAPH_NODEDATA_TYPE_EDIT))
                {
                    OUTPUTW(L"  [", i, L"] ", portTypeStr, L"/", widgetTypeStr, L" name=\"", p.name, L"\" portId=", p.id, L" data=\"", (LPCWSTR)p.widgetData, L"\"", L" connected=", p.isConnected);
                }
                else if (p.dataType == FLOWGRAPH_DATATYPE_COMBO && p.widgetData)
                {
                    EX_FLOWGRAPH_NODE_COMBO_DATA* c = (EX_FLOWGRAPH_NODE_COMBO_DATA*)p.widgetData;
                    OUTPUTW(L"  [", i, L"] ", portTypeStr, L"/", widgetTypeStr, L" name=\"", p.name, L"\" portId=", p.id, L" count=", c->count, L" current=", c->current, L" value=\"", c->options[c->current] ? c->options[c->current] : L"(null)", L"\"");
                }
                else if (p.dataType == FLOWGRAPH_DATATYPE_IMAGE)
                {
                    OUTPUTW(L"  [", i, L"] ", portTypeStr, L"/", widgetTypeStr, L" name=\"", p.name, L"\" portId=", p.id, L" data=0x", (LPVOID)p.widgetData, L" connected=", p.isConnected);
                }
                else
                {
                    OUTPUTW(L"  [", i, L"] ", portTypeStr, L"/", widgetTypeStr, L" name=\"", p.name, L"\" portId=", p.id, L" dataType=", p.dataType, L" connected=", p.isConnected);
                }
            }

      
            LPCWSTR inputPrompt = node->ports[0].widgetData ? (LPCWSTR)node->ports[0].widgetData : L"";
            EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)node->ports[3].widgetData;     
            EX_FLOWGRAPH_NODE_COMBO_DATA* resCombo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)node->ports[4].widgetData; 
            LPCWSTR contentText = node->ports[5].widgetData ? (LPCWSTR)node->ports[5].widgetData : L"";         

           
            const int FLOWGRAPH_REF_VIDEO_FIXED_PORTS = 8;
            INT imgCount = node->dynamicPortCount;
            INT audioCount = node->dynamicPortCount2;

            OUTPUTW(L"[参考生视频] 固定端口数=", FLOWGRAPH_REF_VIDEO_FIXED_PORTS, L", 动态图片数=", imgCount, L", 动态音频数=", audioCount);

            // 统计参考图
            INT validImgCount = 0;
            if (imgCount > 0)
            {
                for (INT i = 0; i < imgCount; i++)
                {
                    INT portIdx = FLOWGRAPH_REF_VIDEO_FIXED_PORTS + i;
                    BOOL hasData = (node->ports[portIdx].widgetData != NULL);
                    OUTPUTW(L"[参考生视频] 参考图[", i, L"] portIdx=", portIdx,
                        L" name=\"", node->ports[portIdx].name, L"\"",
                        L" hasData=", hasData,
                        L" isConnected=", node->ports[portIdx].isConnected);
                    if (hasData) validImgCount++;
                }
            }

            LPCWSTR modelName = (combo && combo->count > 0) ? combo->options[combo->current] : L"";
            LPCWSTR resolutionName = (resCombo && resCombo->count > 0) ? resCombo->options[resCombo->current] : L"";

            OUTPUTW(L"[参考生视频] 输入文本=\"", inputPrompt, L"\"");
            OUTPUTW(L"[参考生视频] 模型=\"", modelName, L"\"");
            OUTPUTW(L"[参考生视频] 分辨率=\"", resolutionName, L"\"");
            OUTPUTW(L"[参考生视频] 内容=\"", contentText, L"\"");
            OUTPUTW(L"[参考生视频] 参考图总数=", imgCount, L", 有数据=", validImgCount);

            // 打印inputs数组
            OUTPUTW(L"[参考生视频] params inputs:");
            DebugPrintIOData(L"[参考生视频 IN]", node, params->inputs, params->inputCount);

            // 打印音频路径
            INT imgStartIdx = FLOWGRAPH_REF_VIDEO_FIXED_PORTS;
            INT audioStartIdx = imgStartIdx + imgCount;
            for (INT i = 0; i < audioCount; i++) {
                INT portIdx = audioStartIdx + i;
                LPCWSTR audioPath = node->ports[portIdx].widgetData ? (LPCWSTR)node->ports[portIdx].widgetData : L"(null)";
                OUTPUTW(L"[参考生视频] 参考音频[", i, L"] path=\"", audioPath, L"\"");
            }

            if (inputPrompt && lstrlenW(inputPrompt) > 0)
            {
                Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_MARK_ASYNC_NODE, params->nodeId, 0);
                INT nodeId = params->nodeId;
                std::thread([nodeId]() {
                    Sleep(3000);
                    EX_FLOWGRAPH_ASYNC_RESULT result = { 0 };
                    result.nodeId = nodeId;
                    result.outputText = L"./res/test.mp4";
                    Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_NODE_EXECUTION_COMPLETE, 0, (LPARAM)&result);
                    }).detach();
                OUTPUTW(L"[参考生视频] 已标记异步执行，等待回调...");
            }
            else
            {
                params->executionResult = FLOWGRAPH_EXEC_RESULT_FAILED;
                OUTPUTW(L"[参考生视频] 执行失败：输入文本为空");
            }
            break;
        }
        // ==================== 本地音频 ====================
        case FLOWGRAPH_CARD_TYPE_LOCAL_AUDIO:
        {
            OUTPUTW(L"[本地音频] 端口布局: [0]INTERMEDIATE/BUTTON(选择) [1]INTERMEDIATE/TEXT(路径) [2]OUTPUT/AUDIO(输出)");
            // 默认透传 widgetData (路径字符串)
            if (node && node->portCount > 2) {
                LPCWSTR path = node->ports[1].widgetData ? (LPCWSTR)node->ports[1].widgetData : L"";
                params->outputs[0].data = (LPVOID)StrDupW(path);
                OUTPUTW(L"[本地音频] 输出音频路径=\"", path, L"\"");
            }
            break;
         }
        default:
            OUTPUTW(L"[未知卡片] cardType=", params->cardType, L", 设置执行失败");
            params->executionResult = FLOWGRAPH_EXEC_RESULT_FAILED;
            break;
        }

        OUTPUTW(L"[执行事件] 返回结果 executionResult=", params->executionResult);
        OUTPUTW(L"══════════════════════════════════════════");
        break;
    }
    case FLOWGRAPH_EVENT_NODE_CANCELED:
    {
        INT canceledNodeId = (INT)wParam;
        OUTPUTW(L"══════════════════════════════════════════");
        OUTPUTW(L"[取消事件] 节点执行被强制取消, nodeId = ", canceledNodeId);

     

        if (canceledNodeId == g_nodeIdLLMText) {
            OUTPUTW(L"[取消事件] 正在终止大模型文本节点的后台请求...");

        }
        else if (canceledNodeId == g_nodeIdT2I1 || canceledNodeId == g_nodeIdRefImg) {
            OUTPUTW(L"[取消事件] 正在终止生图节点的后台任务...");
        }

        OUTPUTW(L"══════════════════════════════════════════");
        break;
    }
	case FLOWGRAPH_EVENT_BUTTON_CLICKED:
	{
		// 按钮点击事件（本地图节点的"选择图片"按钮）
		EX_FLOWGRAPH_BUTTON_CLICK_INFO* info = (EX_FLOWGRAPH_BUTTON_CLICK_INFO*)lParam;
		OUTPUTW(L"[按钮点击] nodeId = ", info->nodeId, L"cardType = ", info->cardType);
		if (info->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_IMAGE) {
			// 打开文件对话框选择图片
			OPENFILENAMEW ofn = { 0 };
			WCHAR szFile[MAX_PATH] = { 0 };
			ofn.lStructSize = sizeof(ofn);
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFilter = L"图片文件\0*.jpg;*.jpeg;*.png;*.bmp\0所有文件\0*.*\0";
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
			if (GetOpenFileNameW(&ofn)) {
				HEXIMAGE hImg = NULL;
				if (_img_createfromfile(szFile, &hImg)) {
					// ★ 通过查找节点获取实际端口ID
					EX_FLOWGRAPH_NODE* node = (EX_FLOWGRAPH_NODE*)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_FIND_NODE, 0, info->nodeId);
					if (node) {
						EX_FLOWGRAPH_PORT newData = { 0 };
						newData.id = node->ports[1].id;   // ★ 使用自动生成的端口ID (port[1]=预览)
						newData.widgetType = FLOWGRAPH_NODEDATA_TYPE_IMAGE;
						newData.widgetData = (LPVOID)hImg;
                        newData.imagePath = szFile;
						Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_UPDATE_NODEDATA, info->nodeId, (LPARAM)&newData);

						// ★ 同时更新输出端口（关键！）
						HEXIMAGE hOutputCopy = NULL;
						_img_copy(hImg, &hOutputCopy);
						if (hOutputCopy) {
							EX_FLOWGRAPH_PORT outData = { 0 };
							outData.id = node->ports[2].id;
							outData.widgetType = FLOWGRAPH_NODEDATA_TYPE_IMAGE;
							outData.widgetData = (LPVOID)hOutputCopy;
                            outData.imagePath = szFile;
							Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_UPDATE_NODEDATA,
								info->nodeId, (LPARAM)&outData);
						}
					}
				}
			}
		}
		else if (info->cardType == FLOWGRAPH_CARD_TYPE_TEXT_TO_IMAGE || info->cardType == FLOWGRAPH_CARD_TYPE_REF_IMAGE) {
			// 保存图片到本地
			EX_FLOWGRAPH_NODE* node = (EX_FLOWGRAPH_NODE*)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_FIND_NODE, 0, info->nodeId);

			if (node) {
				// 查找预览图片端口(INTERMEDIATE/IMAGE)
				HEXIMAGE hImg = NULL;
				for (INT i = 0; i < node->portCount; i++) {
					if (node->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_IMAGE && node->ports[i].widgetData) {
						hImg = (HEXIMAGE)node->ports[i].widgetData;
						break;
					}
				}
				if (hImg) {
					OPENFILENAMEW ofn = { 0 };
					WCHAR szFile[MAX_PATH] = { 0 };
					lstrcpyW(szFile, L"output.png");
					ofn.lStructSize = sizeof(ofn);
					ofn.lpstrFile = szFile;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = L"PNG图片\0*.png\0JPEG图片\0*.jpg\0BMP图片\0*.bmp\0所有文件\0*.*\0";
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
					ofn.lpstrDefExt = L"png";
					if (GetSaveFileNameW(&ofn)) {
						_img_savetofile(hImg, szFile);
					}
				}
			}
		}
		else if (info->cardType == FLOWGRAPH_CARD_TYPE_TEXT_TO_VIDEO || info->cardType == FLOWGRAPH_CARD_TYPE_REF_IMAGE_TO_VIDEO)
		{
			EX_FLOWGRAPH_NODE* node = (EX_FLOWGRAPH_NODE*)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_FIND_NODE, 0, info->nodeId);
			if (node)
			{
				// 查找视频数据
				EX_FLOWGRAPH_NODE_VIDEO_DATA* pVideo = NULL;

				for (INT k = 0; k < node->portCount; k++) {
					if (node->ports[k].widgetType == FLOWGRAPH_NODEDATA_TYPE_VIDEO && node->ports[k].widgetData) {
						pVideo = (EX_FLOWGRAPH_NODE_VIDEO_DATA*)node->ports[k].widgetData;
						break;
					}
				}
				if (pVideo && pVideo->videoPath && lstrlenW(pVideo->videoPath) > 0) {
					OPENFILENAMEW ofn = { 0 };
					WCHAR szFile[MAX_PATH] = { 0 };
					lstrcpyW(szFile, L"output_video.mp4");
					ofn.lStructSize = sizeof(ofn);
					ofn.lpstrFile = szFile;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = L"MP4视频\0*.mp4\0AVI视频\0*.avi\0所有文件\0*.*\0";
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
					ofn.lpstrDefExt = L"mp4";
					if (GetSaveFileNameW(&ofn)) {
						CopyFileW(pVideo->videoPath, szFile, FALSE);
					}
				}
			}
		}
		else if (info->cardType == FLOWGRAPH_CARD_TYPE_TEXT_RENDER) {
			// 复制文本到剪贴板
			EX_FLOWGRAPH_NODE* node = (EX_FLOWGRAPH_NODE*)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_FIND_NODE, 0, info->nodeId);
			if (node) {
				for (INT i = 0; i < node->portCount; i++) {
					if (node->ports[i].widgetType == FLOWGRAPH_NODEDATA_TYPE_TEXT && node->ports[i].widgetData) {
						LPCWSTR text = (LPCWSTR)node->ports[i].widgetData;
						if (OpenClipboard(NULL)) {
							EmptyClipboard();
							SIZE_T size = (lstrlenW(text) + 1) * sizeof(WCHAR);
							HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
							if (hMem) {
								memcpy(GlobalLock(hMem), text, size);
								GlobalUnlock(hMem);
								SetClipboardData(CF_UNICODETEXT, hMem);
							}
							CloseClipboard();
						}
						break;
					}
				}
			}
		}
        else if (info->cardType == FLOWGRAPH_CARD_TYPE_LOCAL_AUDIO) {
            OPENFILENAMEW ofn = { 0 };
            WCHAR szFile[MAX_PATH] = { 0 };
            ofn.lStructSize = sizeof(ofn);
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"音频文件\0*.mp3;*.wav;*.aac\0所有文件\0*.*\0";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
            if (GetOpenFileNameW(&ofn)) {
                EX_FLOWGRAPH_NODE* node = (EX_FLOWGRAPH_NODE*)Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_FIND_NODE, 0, info->nodeId);
                if (node) {
                    // 更新 TEXT 端口 (索引1)
                    EX_FLOWGRAPH_PORT textData = { 0 };
                    textData.id = node->ports[1].id;
                    textData.widgetType = FLOWGRAPH_NODEDATA_TYPE_TEXT;
                    textData.widgetData = (LPVOID)szFile;
                    Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_UPDATE_NODEDATA, info->nodeId, (LPARAM)&textData);

                    // 更新 OUTPUT/AUDIO 端口 (索引2)
                    EX_FLOWGRAPH_PORT outData = { 0 };
                    outData.id = node->ports[2].id;
                    outData.widgetType = 0; // OUTPUT端口无widget
                    outData.widgetData = (LPVOID)StrDupW(szFile); // 传递路径字符串
                    Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_UPDATE_NODEDATA, info->nodeId, (LPARAM)&outData);
                }
            }
        }
		break;
	}

	case FLOWGRAPH_EVENT_NODEDATA_COMBO_CHANGED:
	{
		EX_FLOWGRAPH_PORT* port = (EX_FLOWGRAPH_PORT*)lParam;
		EX_FLOWGRAPH_NODE_COMBO_DATA* combo = (EX_FLOWGRAPH_NODE_COMBO_DATA*)port->widgetData;
		OUTPUTW(L"[组合框改变] nodeId = ", (INT)wParam, L",选中 = ", combo->options[combo->current]);
		break;
	}
	case FLOWGRAPH_EVENT_SIDEBAR_ADD_CARD:
	{
		INT cardType = (INT)wParam;

		// 根据卡片类型设置默认标题
		LPCWSTR defaultTitle = L"自定义节点";
		switch (cardType) {
		case FLOWGRAPH_CARD_TYPE_LOCAL_TEXT:        defaultTitle = L"本地文本节点"; break;
		case FLOWGRAPH_CARD_TYPE_LLM_TEXT:          defaultTitle = L"大模型文本节点"; break;
		case FLOWGRAPH_CARD_TYPE_TEXT_TO_IMAGE:     defaultTitle = L"文生图节点"; break;
		case FLOWGRAPH_CARD_TYPE_LOCAL_IMAGE:       defaultTitle = L"本地图节点"; break;
		case FLOWGRAPH_CARD_TYPE_REF_IMAGE:         defaultTitle = L"参考生图节点"; break;
		case FLOWGRAPH_CARD_TYPE_TEXT_RENDER:       defaultTitle = L"文本显示节点"; break;
		case FLOWGRAPH_CARD_TYPE_TEXT_TO_VIDEO:     defaultTitle = L"文生视频节点"; break;
		case FLOWGRAPH_CARD_TYPE_REF_IMAGE_TO_VIDEO:defaultTitle = L"参考生视频节点"; break;
		default: break;
		}

		// ★ 统一使用 CREATE_CUSTOM_NODE 消息，组件会自动从注册表克隆模板
		EX_FLOWGRAPH_CUSTOM_NODE_CREATE createParam = { 0 };
		createParam.x = FLOWGRAPH_AUTO_POSITION;
		createParam.y = FLOWGRAPH_AUTO_POSITION;
		createParam.title = defaultTitle;
		createParam.cardType = cardType;

		Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_CREATE_CUSTOM_NODE, 0, (LPARAM)&createParam);
		break;
	}
	case FLOWGRAPH_EVENT_CUSTOM_ITEM_CLICKED:
	{
		INT itemIndex = (INT)wParam;       // 点击的条目索引
		LPCWSTR itemName = (LPCWSTR)lParam; // 条目名称
		OUTPUTW(L"[自定义条目点击] index = ", itemIndex, L", name = ", itemName);
		break;
	}
	}
	return 0;
}
// 统一注册所有事件
void RegisterAllEvents(HEXOBJ hObj)
{
	Ex_ObjHandleEvent(hObj, FLOWGRAPH_EVENT_EXECUTE_NODE, FlowGraphNotifyProc);
	Ex_ObjHandleEvent(hObj, FLOWGRAPH_EVENT_BUTTON_CLICKED, FlowGraphNotifyProc);
	Ex_ObjHandleEvent(hObj, FLOWGRAPH_EVENT_NODEDATA_COMBO_CHANGED, FlowGraphNotifyProc);
	Ex_ObjHandleEvent(hObj, FLOWGRAPH_EVENT_SIDEBAR_ADD_CARD, FlowGraphNotifyProc);
	Ex_ObjHandleEvent(hObj, FLOWGRAPH_EVENT_CUSTOM_ITEM_CLICKED, FlowGraphNotifyProc);
    Ex_ObjHandleEvent(hObj, FLOWGRAPH_EVENT_NODE_CANCELED, FlowGraphNotifyProc);
}


LRESULT CALLBACK OnFlowGraphWndMsgProc(HWND hWnd, HEXDUI hExDui, INT uMsg, WPARAM wParam,
	LPARAM lParam, LRESULT* lpResult)
{
	if (uMsg == WM_SIZE) {
		INT  width = LOWORD(lParam);
		INT  height = HIWORD(lParam);
		Ex_ObjMove(hFlowGraph, 0, Ex_Scale(30), width, height - Ex_Scale(50), FALSE);
	}
	return 0;
}

void RegisterBuiltinCardTypes()
{
	// 公共组合框数据
	EX_FLOWGRAPH_NODE_COMBO_DATA llmComboData = { 0 };
	LPCWSTR llmOptions[] = { L"deepseek", L"豆包" };
	llmComboData.options = llmOptions; llmComboData.count = 2; llmComboData.current = 0;

	EX_FLOWGRAPH_NODE_COMBO_DATA imgComboData = { 0 };
	LPCWSTR imgOptions[] = { L"豆包" };
	imgComboData.options = imgOptions; imgComboData.count = 1; imgComboData.current = 0;

	EX_FLOWGRAPH_NODE_COMBO_DATA resComboData = { 0 };
	LPCWSTR resOptions[] = { L"2K", L"4K" };
	resComboData.options = resOptions; resComboData.count = 2; resComboData.current = 0;

	EX_FLOWGRAPH_NODE_COMBO_DATA videoComboData = { 0 };
	LPCWSTR videoOptions[] = { L"可灵", L"runway" };
	videoComboData.options = videoOptions; videoComboData.count = 2; videoComboData.current = 0;

	EX_FLOWGRAPH_NODE_BUTTON_DATA btnSelectImg = { L"选择图片" };
	EX_FLOWGRAPH_NODE_BUTTON_DATA btnSaveImg = { L"保存图片到本地" };
	EX_FLOWGRAPH_NODE_BUTTON_DATA btnSaveVideo = { L"保存视频到本地" };
	EX_FLOWGRAPH_NODE_BUTTON_DATA btnCopy = { L"复制文本到剪贴板" };
	EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA dualBtn = { L"+参考图", L"-参考图" };
    EX_FLOWGRAPH_NODE_BUTTON_DATA btnSelectAudio = { L"选择音频" };
    EX_FLOWGRAPH_NODE_DUAL_BUTTON_DATA dualBtnAudio = { L"+参考音频", L"-参考音频" };

	// ==================== 1. 本地文本(EDIT→TEXT) ====================
	EX_FLOWGRAPH_PORT_DESC ltPorts[2] = { 0 };
	ltPorts[0] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"内容",
		FLOWGRAPH_NODEDATA_TYPE_TEXT, 0, 300, 0, (LPVOID)L"" };  
	ltPorts[1] = { FLOWGRAPH_PORTTYPE_OUTPUT, FLOWGRAPH_DATATYPE_STRING, L"输出文本", 0, 0, 0, 0, NULL };
	EX_FLOWGRAPH_CARD_DESCRIPTOR ltDesc = { FLOWGRAPH_CARD_TYPE_LOCAL_TEXT, L"本地文本",
		ExARGB(80,130,80,255), 2, ltPorts };
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_REGISTER_CARD_TYPE, 0, (LPARAM)&ltDesc);

	// ==================== 2. 大模型文本(EDIT→TEXT) ====================
	EX_FLOWGRAPH_PORT_DESC llmPorts[4] = { 0 };
	llmPorts[0] = { FLOWGRAPH_PORTTYPE_INPUT, FLOWGRAPH_DATATYPE_STRING, L"输入文本", 0, 0, 0, 0, NULL };
	llmPorts[1] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_COMBO, L"模型",
		FLOWGRAPH_NODEDATA_TYPE_COMBO, 0, 300, 30, &llmComboData };
	llmPorts[2] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"提示词",
		FLOWGRAPH_NODEDATA_TYPE_TEXT, 0, 500, 0, (LPVOID)L"" };  
	llmPorts[3] = { FLOWGRAPH_PORTTYPE_OUTPUT, FLOWGRAPH_DATATYPE_STRING, L"输出文本", 0, 0, 0, 0, NULL };
	EX_FLOWGRAPH_CARD_DESCRIPTOR llmDesc = { FLOWGRAPH_CARD_TYPE_LLM_TEXT, L"大模型文本",
		ExARGB(80,80,180,255), 4, llmPorts };
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_REGISTER_CARD_TYPE, 0, (LPARAM)&llmDesc);

	// ==================== 3. 文生图(增加TEXT端口) ====================
	EX_FLOWGRAPH_PORT_DESC t2iPorts[7] = { 0 };
	t2iPorts[0] = { FLOWGRAPH_PORTTYPE_INPUT, FLOWGRAPH_DATATYPE_STRING, L"输入文本", 0, 0, 0, 0, NULL };
	t2iPorts[1] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_COMBO, L"模型",
		FLOWGRAPH_NODEDATA_TYPE_COMBO, 0, 300, 30, &imgComboData };
	t2iPorts[2] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_COMBO, L"分辨率",
		FLOWGRAPH_NODEDATA_TYPE_COMBO, 0, 300, 30, &resComboData };
	t2iPorts[3] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"内容",
		FLOWGRAPH_NODEDATA_TYPE_TEXT, 0, 300, 0, (LPVOID)L"" };  
	t2iPorts[4] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_IMAGE, L"预览",
		FLOWGRAPH_NODEDATA_TYPE_IMAGE, 0, 300, 200, NULL };
	t2iPorts[5] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"保存图片",
		FLOWGRAPH_NODEDATA_TYPE_BUTTON, 0, 300, 30, &btnSaveImg };
	t2iPorts[6] = { FLOWGRAPH_PORTTYPE_OUTPUT, FLOWGRAPH_DATATYPE_IMAGE, L"输出图片", 0, 0, 0, 0, NULL };
	EX_FLOWGRAPH_CARD_DESCRIPTOR t2iDesc = { FLOWGRAPH_CARD_TYPE_TEXT_TO_IMAGE, L"文生图",
		ExARGB(180,80,80,255), 7, t2iPorts };
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_REGISTER_CARD_TYPE, 0, (LPARAM)&t2iDesc);

	// ==================== 4. 本地图(不变) ====================
	EX_FLOWGRAPH_PORT_DESC liPorts[3] = { 0 };
	liPorts[0] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"选择图片",
		FLOWGRAPH_NODEDATA_TYPE_BUTTON, 0, 300, 30, &btnSelectImg };
	liPorts[1] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_IMAGE, L"预览",
		FLOWGRAPH_NODEDATA_TYPE_IMAGE, 0, 300, 200, NULL };
	liPorts[2] = { FLOWGRAPH_PORTTYPE_OUTPUT, FLOWGRAPH_DATATYPE_IMAGE, L"输出图片", 0, 0, 0, 0, NULL };
	EX_FLOWGRAPH_CARD_DESCRIPTOR liDesc = { FLOWGRAPH_CARD_TYPE_LOCAL_IMAGE, L"本地图",
		ExARGB(80,150,130,255), 3, liPorts };
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_REGISTER_CARD_TYPE, 0, (LPARAM)&liDesc);

	// ==================== 5. 参考生图(增加TEXT端口) ====================
	EX_FLOWGRAPH_PORT_DESC riAllPorts[9] = { 0 };
	riAllPorts[0] = { FLOWGRAPH_PORTTYPE_INPUT, FLOWGRAPH_DATATYPE_STRING, L"文本", 0, 0, 0, 0, NULL };
	riAllPorts[1] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_ANY, L"参考图控制",
		FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON, FLOWGRAPH_WIDGET_ID_REF_CONTROL, 300, 30, &dualBtn };
	riAllPorts[2] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_COMBO, L"模型",
		FLOWGRAPH_NODEDATA_TYPE_COMBO, 0, 300, 30, &imgComboData };
	riAllPorts[3] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_COMBO, L"分辨率",
		FLOWGRAPH_NODEDATA_TYPE_COMBO, 0, 300, 30, &resComboData };
	riAllPorts[4] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"内容",
		FLOWGRAPH_NODEDATA_TYPE_TEXT, 0, 300, 0, (LPVOID)L"" };  
	riAllPorts[5] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_IMAGE, L"预览",
		FLOWGRAPH_NODEDATA_TYPE_IMAGE, 0, 300, 200, NULL };
	riAllPorts[6] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"保存图片",
		FLOWGRAPH_NODEDATA_TYPE_BUTTON, 0, 300, 30, &btnSaveImg };
	riAllPorts[7] = { FLOWGRAPH_PORTTYPE_INPUT, FLOWGRAPH_DATATYPE_IMAGE, L"参考图片0", 0, 0, 0, 0, NULL };
	riAllPorts[8] = { FLOWGRAPH_PORTTYPE_OUTPUT, FLOWGRAPH_DATATYPE_IMAGE, L"输出图片", 0, 0, 0, 0, NULL };

	EX_FLOWGRAPH_CARD_DESCRIPTOR riDesc = { 0 };
	riDesc.cardType = FLOWGRAPH_CARD_TYPE_REF_IMAGE;
	riDesc.typeName = L"参考生图";
	riDesc.tagColor = ExARGB(150, 80, 150, 255);
	riDesc.portCount = 9;
	riDesc.ports = riAllPorts;
	riDesc.dynamicPortBaseIndex = 7;   
	riDesc.dynamicPortMinCount = 1;
	riDesc.dynamicPortMaxCount = 10;
	riDesc.dynamicPortDataType = FLOWGRAPH_DATATYPE_IMAGE;
	riDesc.dynamicPortNamePrefix = L"参考图片";
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_REGISTER_CARD_TYPE, 0, (LPARAM)&riDesc);

	// ==================== 6. 文本显示(不变) ====================
	EX_FLOWGRAPH_PORT_DESC trPorts[4] = { 0 };
	trPorts[0] = { FLOWGRAPH_PORTTYPE_INPUT, FLOWGRAPH_DATATYPE_STRING, L"输入文本", 0, 0, 0, 0, NULL };
	trPorts[1] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"显示文本",
		FLOWGRAPH_NODEDATA_TYPE_TEXT, 0, 500, 0, (LPVOID)L"" };
	trPorts[2] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"复制文本",
		FLOWGRAPH_NODEDATA_TYPE_BUTTON, 0, 300, 30, &btnCopy };
	trPorts[3] = { FLOWGRAPH_PORTTYPE_OUTPUT, FLOWGRAPH_DATATYPE_STRING, L"输出文本", 0, 0, 0, 0, NULL };
	EX_FLOWGRAPH_CARD_DESCRIPTOR trDesc = { FLOWGRAPH_CARD_TYPE_TEXT_RENDER, L"文本显示",
		ExARGB(60,120,160,255), 4, trPorts };
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_REGISTER_CARD_TYPE, 0, (LPARAM)&trDesc);

	// ==================== 7. 文生视频(增加分辨率+TEXT端口) ====================
	EX_FLOWGRAPH_PORT_DESC t2vPorts[7] = { 0 };
	t2vPorts[0] = { FLOWGRAPH_PORTTYPE_INPUT, FLOWGRAPH_DATATYPE_STRING, L"输入文本", 0, 0, 0, 0, NULL };
	t2vPorts[1] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_COMBO, L"模型",
		FLOWGRAPH_NODEDATA_TYPE_COMBO, 0, 300, 30, &videoComboData };
	t2vPorts[2] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_COMBO, L"分辨率",
		FLOWGRAPH_NODEDATA_TYPE_COMBO, 0, 300, 30, &resComboData };  
	t2vPorts[3] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"内容",
		FLOWGRAPH_NODEDATA_TYPE_TEXT, 0, 300, 0, (LPVOID)L"" };  
	t2vPorts[4] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_VIDEO, L"视频预览",
		FLOWGRAPH_NODEDATA_TYPE_VIDEO, 0, 300, 200, NULL };
	t2vPorts[5] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"保存视频",
		FLOWGRAPH_NODEDATA_TYPE_BUTTON, 0, 300, 30, &btnSaveVideo };
	t2vPorts[6] = { FLOWGRAPH_PORTTYPE_OUTPUT, FLOWGRAPH_DATATYPE_STRING, L"输出视频路径", 0, 0, 0, 0, NULL };
	EX_FLOWGRAPH_CARD_DESCRIPTOR t2vDesc = { FLOWGRAPH_CARD_TYPE_TEXT_TO_VIDEO, L"文生视频",
		ExARGB(180,130,50,255), 7, t2vPorts };
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_REGISTER_CARD_TYPE, 0, (LPARAM)&t2vDesc);

	// ==================== 8. 参考生视频(增加分辨率+TEXT端口) ====================
    // 布局: 0文本, 1图控制, 2音频控制, 3模型, 4分辨率, 5内容, 6视频, 7保存, 8图片0(动态1), 9输出
    EX_FLOWGRAPH_PORT_DESC rvPorts[10] = { 0 };
    rvPorts[0] = { FLOWGRAPH_PORTTYPE_INPUT, FLOWGRAPH_DATATYPE_STRING, L"文本", 0, 0, 0, 0, NULL };
    rvPorts[1] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_ANY, L"参考图控制", FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON, FLOWGRAPH_WIDGET_ID_REF_CONTROL, 300, 30, &dualBtn };
    rvPorts[2] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_ANY, L"参考音频控制", FLOWGRAPH_NODEDATA_TYPE_DUAL_BUTTON, FLOWGRAPH_WIDGET_ID_REF_AUDIO_CONTROL, 300, 30, &dualBtnAudio }; 
    rvPorts[3] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_COMBO, L"模型", FLOWGRAPH_NODEDATA_TYPE_COMBO, 0, 300, 30, &videoComboData };
    rvPorts[4] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_COMBO, L"分辨率", FLOWGRAPH_NODEDATA_TYPE_COMBO, 0, 300, 30, &resComboData };
    rvPorts[5] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"内容", FLOWGRAPH_NODEDATA_TYPE_TEXT, 0, 300, 0, (LPVOID)L"" };
    rvPorts[6] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_VIDEO, L"视频预览", FLOWGRAPH_NODEDATA_TYPE_VIDEO, 0, 300, 200, NULL };
    rvPorts[7] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"保存视频", FLOWGRAPH_NODEDATA_TYPE_BUTTON, 0, 300, 30, &btnSaveVideo };
    rvPorts[8] = { FLOWGRAPH_PORTTYPE_INPUT, FLOWGRAPH_DATATYPE_IMAGE, L"参考图片0", 0, 0, 0, 0, NULL }; // 动态1 (Min=1)
    // 动态2 (音频 Min=0，不占位)
    rvPorts[9] = { FLOWGRAPH_PORTTYPE_OUTPUT, FLOWGRAPH_DATATYPE_STRING, L"输出视频路径", 0, 0, 0, 0, NULL };

    EX_FLOWGRAPH_CARD_DESCRIPTOR rvDesc = { 0 };
    rvDesc.cardType = FLOWGRAPH_CARD_TYPE_REF_IMAGE_TO_VIDEO;
    rvDesc.typeName = L"参考生视频";
    rvDesc.tagColor = ExARGB(130, 50, 180, 255);
    rvDesc.portCount = 10;
    rvDesc.ports = rvPorts;
    // 第一组动态端口 (图片)
    rvDesc.dynamicPortBaseIndex = 8;
    rvDesc.dynamicPortMinCount = 1;
    rvDesc.dynamicPortMaxCount = 3;
    rvDesc.dynamicPortDataType = FLOWGRAPH_DATATYPE_IMAGE;
    rvDesc.dynamicPortNamePrefix = L"参考图片";
    // ★ 第二组动态端口 (音频)
    rvDesc.dynamicPort2BaseIndex = 0; // 紧跟在图片后面
    rvDesc.dynamicPort2MinCount = 0;  // 最少0个
    rvDesc.dynamicPort2MaxCount = 3;  // 最多3个
    rvDesc.dynamicPort2DataType = FLOWGRAPH_DATATYPE_AUDIO;
    rvDesc.dynamicPort2NamePrefix = L"参考音频";
    Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_REGISTER_CARD_TYPE, 0, (LPARAM)&rvDesc);

    // ==================== 9. 本地音频 (新增) ====================
    EX_FLOWGRAPH_PORT_DESC laPorts[3] = { 0 };
    laPorts[0] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"选择音频", FLOWGRAPH_NODEDATA_TYPE_BUTTON, 0, 300, 30, &btnSelectAudio };
    laPorts[1] = { FLOWGRAPH_PORTTYPE_INTERMEDIATE, FLOWGRAPH_DATATYPE_STRING, L"音频路径", FLOWGRAPH_NODEDATA_TYPE_TEXT, 0, 300, 0, (LPVOID)L"" };
    laPorts[2] = { FLOWGRAPH_PORTTYPE_OUTPUT, FLOWGRAPH_DATATYPE_AUDIO, L"输出音频", 0, 0, 0, 0, NULL };

    EX_FLOWGRAPH_CARD_DESCRIPTOR laDesc = { FLOWGRAPH_CARD_TYPE_LOCAL_AUDIO, L"本地音频", ExARGB(200, 180, 50, 255), 3, laPorts };
    Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_REGISTER_CARD_TYPE, 0, (LPARAM)&laDesc);
}

void test_flowgraph(HWND hWnd)
{
	hWnd_flowgraph = Ex_WndCreate(hWnd, L"Ex_DirectUI", L"测试画布", 0, 0, 1400, 1000, 0, 0);
	auto hExDui_flowgraph = Ex_DUIBindWindowEx(hWnd_flowgraph, 0,
		WINDOW_STYLE_NOINHERITBKG | WINDOW_STYLE_BUTTON_CLOSE |
		WINDOW_STYLE_BUTTON_MIN | WINDOW_STYLE_BUTTON_MAX | WINDOW_STYLE_MOVEABLE |
		WINDOW_STYLE_CENTERWINDOW | WINDOW_STYLE_TITLE |
		WINDOW_STYLE_HASICON, 0, OnFlowGraphWndMsgProc);
	Ex_DUISetLong(hExDui_flowgraph, ENGINE_LONG_CRBKG, ExARGB(80, 80, 90, 255));
	hFlowGraph = Ex_ObjCreateEx(-1, L"FlowGraph", L"", -1, 10, 30, 1380, 900,
		hExDui_flowgraph, 200, DT_VCENTER | DT_CENTER, 0, 0, NULL);
	RegisterBuiltinCardTypes();
	CreateNodes();
	AddConnections();
	RegisterAllEvents(hFlowGraph);
	LPCWSTR items[] = { L"生成报告", L"发送邮件", L"导出数据", L"刷新缓存" };
	EX_FLOWGRAPH_CUSTOM_ITEMS customItems;
	customItems.count = 4;
	customItems.items = items;
	Ex_ObjSendMessage(hFlowGraph, FLOWGRAPH_MESSAGE_SET_CUSTOM_ITEMS, 0, (LPARAM)&customItems);
	Ex_DUIShowWindow(hExDui_flowgraph, SW_SHOWNORMAL);
}