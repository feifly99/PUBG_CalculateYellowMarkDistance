#include "draw.h"
#include "head.h"

HWND window = NULL;

static double distance = 0.0;

void support()
{
    beginDraw();

    CONST CHAR buffer[32] = { 0 };
    CHAR* CONST buf = (CHAR * CONST)buffer;
    sprintf_s(buf, sizeof(buffer), "%d", (int)distance);
    drawText(768, 432, buffer, D3DCOLOR_ARGB(255, 255, 0, 0));

    endDraw();

    return;
}


DWORD enterDrawLoop(PVOID arg)
{
    UNREFERENCED_PARAMETER(arg);
    createTransparentWindow(GetDesktopWindow(), support);
    windowMsgLoop();
    return 0;
}

int main()
{
    window = GetDesktopWindow();
    HANDLE hThread = CreateThread(NULL, 0, enterDrawLoop, NULL, 0, NULL);
    Point imageLeftTop = { 1630, 793 };
    Point imageRightBottom = { 1886, 1049 };
    while (1)
    {
        HDC screenDC = GetDC(NULL);
        BITMAP img0 = { 0 };
        HDC* memDC = NULL;
        HBITMAP* memImg = NULL;
        transScreenDC2Img(screenDC, &img0, imageLeftTop, imageRightBottom, &memDC, &memImg);
        PIMG img = makeImg(img0, &screenDC, &memDC, &memImg, FALSE, L"EXEx");
        PRGB_TENSOR rgbTensor = NULL;
        makeImgRgbTensor(&img, &rgbTensor);
        distance = getDistanceToYellowMark(&rgbTensor);
        printf("%.2lf\n", distance);
        ExFreeRgbTensor(&rgbTensor);
        Sleep(50);
    }
    return 0;
}