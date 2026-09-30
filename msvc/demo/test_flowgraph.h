#pragma once
#include "ExDUIR_Func.h"
#include <thread>



void test_flowgraph(HWND hWnd);
LPCWSTR* VecToLPCWSTRArray(const std::vector<std::wstring>& vec);
void CreateNodes();
void AddConnections();
LRESULT CALLBACK FlowGraphNotifyProc(HEXOBJ hObj, INT nID, INT nCode, WPARAM wParam, LPARAM lParam);
void RegisterAllEvents(HEXOBJ hObj);
void DebugPrintIOData(LPCWSTR tag, EX_FLOWGRAPH_NODE_IO_DATA* arr, INT count);