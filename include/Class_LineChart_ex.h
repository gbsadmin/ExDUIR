#pragma once

// ����_����ͼ_����ָ��
#define LINECHART_LONG_DATA 0


void _linechart_register();
LRESULT CALLBACK _linechart_proc(HWND hWnd, HEXOBJ hObj, INT uMsg, WPARAM wParam, LPARAM lParam);
void _linechart_paint(HEXOBJ hObj);
void _linechart_update_polygon(EX_POLYGON* ptr, int newY);