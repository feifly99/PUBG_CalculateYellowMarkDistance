#include "draw.h"

int initializeD3D()
{

	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return 0;

	// 创建D3D设备
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	if (IDirect3D9_CreateDevice(g_pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 辅助窗口句柄, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return 0;

	if (pLine == NULL)
		D3DXCreateLine(g_pd3dDevice, &pLine);

	D3DXCreateFontW(g_pd3dDevice, 16, 0, FW_DONTCARE, D3DX_DEFAULT, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"Vernada", &Font);

	return 1;
}

void createTransparentWindow(HWND 游戏窗口句柄, Draw 绘制函数)
{
	if (绘制函数 == NULL || 游戏窗口句柄 == 0) return;

	GameHwnd = 游戏窗口句柄;
	Render = 绘制函数;

	//初始化窗口类
	wClass.cbClsExtra = 0;
	wClass.cbSize = sizeof(WNDCLASSEX);
	wClass.cbWndExtra = 0;
	wClass.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
	wClass.hCursor = LoadCursor(0, IDC_ARROW);
	wClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	wClass.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wClass.hInstance = GetModuleHandle(NULL);
	wClass.lpfnWndProc = (WNDPROC)WinProc;
	wClass.lpszClassName = L" ";
	wClass.lpszMenuName = L" ";
	wClass.style = CS_VREDRAW | CS_HREDRAW;

	//注册窗口
	if (RegisterClassEx(&wClass) == 0)
	{
		MessageBox(NULL, L"创建窗口出错！", L"提示！", 0);
		exit(1);
	}

	//创建窗口
	GetWindowRect(GameHwnd, &窗口矩形);
	窗口宽 = 窗口矩形.right - 窗口矩形.left;
	窗口高 = 窗口矩形.bottom - 窗口矩形.top;
	辅助窗口句柄 = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, L" ", L" ", WS_POPUP, 1, 1, 窗口宽, 窗口高, 0, 0, 0, 0);

	//显示窗口
	SetLayeredWindowAttributes(辅助窗口句柄, 0, RGB(0, 0, 0), LWA_COLORKEY);
	ShowWindow(辅助窗口句柄, SW_SHOW);

	initializeD3D();
}

void windowMsgLoop()
{
	while (1)
	{
		//使辅助窗口一直盖在游戏窗口上
		if (GameHwnd)
		{
			GetWindowRect(GameHwnd, &窗口矩形);
			窗口宽 = 窗口矩形.right - 窗口矩形.left;
			窗口高 = 窗口矩形.bottom - 窗口矩形.top;
			DWORD dwStyle = GetWindowLong(GameHwnd, GWL_STYLE);
			if (dwStyle & WS_BORDER)
			{
				窗口矩形.top += 23;
				窗口高 -= 23;
			}
			MoveWindow(辅助窗口句柄, 窗口矩形.left, 窗口矩形.top, 窗口宽, 窗口高, 1);
		}

		//处理窗口消息
		MSG Message;
		ZeroMemory(&Message, sizeof(Message));
		if (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
		{
			DispatchMessage(&Message);
			TranslateMessage(&Message);
		}
		Sleep(20);
	}


	if (g_pd3dDevice) { IDirect3DDevice9_Release(g_pd3dDevice); g_pd3dDevice = NULL; }
	if (g_pD3D) { IDirect3DDevice9_Release(g_pd3dDevice); g_pD3D = NULL; }
	CloseWindow(辅助窗口句柄);

	UnregisterClass(wClass.lpszClassName, wClass.hInstance);
}

LRESULT WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_PAINT:
		{
			if (g_pd3dDevice)Render(); //就是这里调用了我们的画框画线之类的函数
			break;
		}
		case WM_CREATE:
		{
			DwmExtendFrameIntoClientArea(hWnd, &Margin);
			break;
		}
		case WM_DESTROY:
		{
			IDirect3D9_Release(g_pD3D);
			IDirect3DDevice9_Release(g_pd3dDevice);
			exit(1);
			return 0;
		}
		default:
		{
			return DefWindowProc(hWnd, Message, wParam, lParam);
			break;
		}
	}
	return 0;
}

void drawLine(D3DCOLOR Color, float X1, float Y1, float X2, float Y2, float Width)
{
	D3DXVECTOR2 Vertex[2] = { {X1, Y1}, {X2, Y2} };

	if (pLine && pLine->lpVtbl) {
		pLine->lpVtbl->SetWidth(pLine, Width);
		pLine->lpVtbl->Draw(pLine, Vertex, 2, Color);
	}
}

void drawText(float X, float Y, const char* Str, D3DCOLOR Color)
{
	RECT Rect = { (LONG)X, (LONG)Y, 0, 0 };

	if (Font && Font->lpVtbl) {
		Font->lpVtbl->DrawTextA(Font, NULL, Str, -1, &Rect, DT_CALCRECT, Color);
		Font->lpVtbl->DrawTextA(Font, NULL, Str, -1, &Rect, DT_LEFT, Color);
	}
}

void drawBox(float X, float Y, float W, float H, float Width, D3DCOLOR Color)
{
	D3DXVECTOR2 Vertex[5] = {
		{X, Y},
		{X + W, Y},
		{X + W, Y + H},
		{X, Y + H},
		{X, Y}
	};

	if (pLine && pLine->lpVtbl) {
		pLine->lpVtbl->SetWidth(pLine, Width);
		pLine->lpVtbl->Draw(pLine, Vertex, 5, Color);
	}
}

void beginDraw()
{
	if (g_pd3dDevice && g_pd3dDevice->lpVtbl) {
		g_pd3dDevice->lpVtbl->Clear(g_pd3dDevice, 0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);
		g_pd3dDevice->lpVtbl->BeginScene(g_pd3dDevice);
	}
}

void endDraw()
{
	if (g_pd3dDevice && g_pd3dDevice->lpVtbl) {
		g_pd3dDevice->lpVtbl->EndScene(g_pd3dDevice);
		g_pd3dDevice->lpVtbl->Present(g_pd3dDevice, NULL, NULL, NULL, NULL);
	}
}
