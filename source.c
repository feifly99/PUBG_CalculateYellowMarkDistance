#include "head.h"

#define IS_YELLOW_PIXEL(r,g,b) (!(r > 200 && g > 200 && b < 130))

void ExFreeMem(
    PVOID* mem
)
{
    if (*mem == NULL)
    {
        printf("空指针传入.\n");
        return;
    }
    free(*mem);
    *mem = NULL;
    return;
}

void initializeInfoHeader(
    IN BITMAP bmpScreen,
    OUT BITMAPINFOHEADER* bi
)
{
    (*bi).biSize = sizeof(BITMAPINFOHEADER);
    (*bi).biWidth = bmpScreen.bmWidth;
    (*bi).biHeight = bmpScreen.bmHeight;
    (*bi).biPlanes = 1;
    (*bi).biBitCount = 32;
    (*bi).biCompression = BI_RGB;
    (*bi).biSizeImage = 0;
    (*bi).biXPelsPerMeter = 0;
    (*bi).biYPelsPerMeter = 0;
    (*bi).biClrUsed = 0;
    (*bi).biClrImportant = 0;
    return;
}

void initializeFileHeader(
    OUT PBITMAPFILEHEADER imgFileHeader,
    IN DWORD sizeHeaderAndInfo,
    IN DWORD wholeSize,
    IN WORD imgHeadMarkCode
)
{
    imgFileHeader->bfOffBits = sizeHeaderAndInfo;
    imgFileHeader->bfSize = wholeSize;
    imgFileHeader->bfType = 0x4D42;
    return;
}

void initializePixelHeader(
    OUT_PTR PVOID* imgPixelHeader,
    IN LONG imgPixelSize,
    IN HDC nakedScreenImage,
    IN HBITMAP hiddenImageInstance,
    IN BITMAP img,
    IN BITMAPINFO* imgInfoHeader
)
{
    *imgPixelHeader = malloc((SIZE_T)imgPixelSize);
    GetDIBits(
        nakedScreenImage,
        hiddenImageInstance,
        0,
        (UINT)img.bmHeight,
        *imgPixelHeader,
        imgInfoHeader,
        DIB_RGB_COLORS
    );
}

void createImageFile(
    IN WCHAR* w_filePath,
    OUT HANDLE* hFile
)
{
    *hFile = CreateFileW(
        w_filePath,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
}

SIZE_T getTotalBitMapSize(
    IN DWORD dwBmpSize
)
{
    return (SIZE_T)dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
}

LONG getBmpSize(
    IN BITMAP bmpScreen,
    IN BITMAPINFOHEADER bi
)
{
    return ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;
}

void saveImg2Disk(
    OUT HANDLE* hFile,
    CONST WCHAR* fatherLocWithNoMark,
    CONST WCHAR* imgName,
    IN PBITMAPFILEHEADER imgFileHeader,
    IN PBITMAPINFOHEADER imgInfoHeader,
    IN PVOID imgPixelHeader,
    IN DWORD wholeSize
)
{
    WCHAR FilePath[MAX_PATH];
    wsprintfW(FilePath, L"%s\\%s.png", fatherLocWithNoMark, imgName);

    createImageFile(FilePath, hFile);
    WriteFile(*hFile, (LPSTR)imgFileHeader, sizeof(BITMAPFILEHEADER), NULL, NULL);
    WriteFile(*hFile, (LPSTR)imgInfoHeader, sizeof(BITMAPINFOHEADER), NULL, NULL);
    WriteFile(*hFile, (LPSTR)imgPixelHeader, wholeSize, NULL, NULL);
    return;
}

void transScreenDC2Img(
    IN HDC screenDC,
    OUT BITMAP* img,
    IN Point lt,
    IN Point rb,
    OUT_PTR HDC** _hiddenImgCode,
    OUT_PTR HBITMAP** _hiddenImageInstance
)
{
    int w = (int)(rb.x - lt.x);
    int h = (int)(rb.y - lt.y);
    *_hiddenImgCode = (HDC*)malloc(sizeof(HDC));
    *_hiddenImageInstance = (HBITMAP*)malloc(sizeof(HBITMAP));
    **_hiddenImgCode = CreateCompatibleDC(screenDC);
    **_hiddenImageInstance = CreateCompatibleBitmap(screenDC, w, h);

    SelectObject(**_hiddenImgCode, **_hiddenImageInstance);
    BitBlt(
        **_hiddenImgCode,
        0, 0,
        w, h,
        screenDC,
        (int)lt.x, (int)lt.y,
        SRCCOPY
    );
    GetObjectW(**_hiddenImageInstance, sizeof(BITMAP), img);
    return;
}

PIMG makeImg(
    IN BITMAP img,
    IN_CLEAR_OUT HDC* screenDC,
    IN_CLEAR_OUT HDC** memDC,
    IN_CLEAR_OUT HBITMAP** memImg,
    IN BOOLEAN isSaveToDisk,
    IN_OPT CONST WCHAR* name
)
{
    PIMG ret = (PIMG)malloc(sizeof(IMG));

    PBITMAPINFOHEADER imgInfoHeaderPointer = (PBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER));
    PBITMAPFILEHEADER imgFileHeaderPointer = (PBITMAPFILEHEADER)malloc(sizeof(BITMAPFILEHEADER));
    PVOID imgPixelHeader = NULL;

    initializeInfoHeader(img, imgInfoHeaderPointer);

    LONG imgPixelSize = getBmpSize(img, *imgInfoHeaderPointer);
    initializePixelHeader(
        &imgPixelHeader,
        imgPixelSize,
        *screenDC,
        **memImg,
        img,
        (BITMAPINFO*)imgInfoHeaderPointer
    );

    initializeFileHeader(
        imgFileHeaderPointer,
        (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER),
        imgPixelSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER),
        0x4D42
    );

    if (name)
    {
        if (isSaveToDisk)
        {
            HANDLE hFile = NULL;
            saveImg2Disk(
                &hFile,
                L"E:\\desk",
                name,
                imgFileHeaderPointer,
                imgInfoHeaderPointer,
                imgPixelHeader,
                imgPixelSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
            );
            CloseHandle(hFile);
        }
        else
        {
            //printf("未选择保存中间结果.\n");
        }
    }
    else
    {
        printf("isSaveToDisk && name == 0, 检查输入.\n");
        exit(0xFFFFFFCF);
    }

    ret->filePointer = imgFileHeaderPointer;
    ret->infoPointer = imgInfoHeaderPointer;
    ret->pixelPointer = imgPixelHeader;

    DeleteObject(**memImg);
    ExFreeMem((PVOID*)&(*memImg));
    DeleteObject(**memDC);
    ExFreeMem((PVOID*)&(*memDC));
    ReleaseDC(NULL, *screenDC);
    return ret;
}

void makeImgRgbTensor(
    IN_CLEAR_OUT PIMG* img,
    PRGB_TENSOR* tensor
)
{
    if (*tensor != NULL)
    {
        printf("已经有值.\n");
        exit(0xEEFFEE);
    }
    *tensor = (PRGB_TENSOR)malloc(sizeof(RGB_TENSOR));
    (*tensor)->width = (*img)->infoPointer->biWidth;
    (*tensor)->height = (*img)->infoPointer->biHeight;
    (*tensor)->depth = 3;
    (*tensor)->RGBtensor = (UCHAR***)malloc((*tensor)->depth * sizeof(UCHAR**));
    fors(
        (*tensor)->depth,
        ((*tensor)->RGBtensor)[j] = (UCHAR**)malloc((*tensor)->height * sizeof(UCHAR*));
    );
    forss(
        (*tensor)->depth, (*tensor)->height,
        ((*tensor)->RGBtensor)[j][i] = (UCHAR*)malloc((*tensor)->width * sizeof(UCHAR));,
        );
    ULONG64 headerPixelPointer = (ULONG64)((*img)->pixelPointer) + (*tensor)->width * (*tensor)->height * ((*tensor)->depth + 1) - 4;
    forss(
        (*tensor)->height, (*tensor)->width,
        ((*tensor)->RGBtensor)[0][j][(*tensor)->width - i - 1] = *(UCHAR*)(headerPixelPointer + 2); //R
    ((*tensor)->RGBtensor)[1][j][(*tensor)->width - i - 1] = *(UCHAR*)(headerPixelPointer + 1); //G
    ((*tensor)->RGBtensor)[2][j][(*tensor)->width - i - 1] = *(UCHAR*)(headerPixelPointer + 0); //B
    headerPixelPointer -= 4; ,
        );
    ExFreeMem((PVOID*)&((*img)->filePointer));
    ExFreeMem((PVOID*)&((*img)->infoPointer));
    ExFreeMem((PVOID*)&((*img)->pixelPointer));
    ExFreeMem((PVOID*)&((*img)));
    return;
}

void createRgbTensor(
    IN size_t width,
    IN size_t height,
    IN size_t depth,
    IN_OUT PRGB_TENSOR* tensor
)
{
    if (*tensor != NULL)
    {
        printf("已经有值.\n");
        exit(0xEEFFEE);
    }
    *tensor = (PRGB_TENSOR)malloc(sizeof(RGB_TENSOR));
    (*tensor)->width = width;
    (*tensor)->height = height;
    (*tensor)->depth = depth;
    (*tensor)->RGBtensor = (UCHAR***)malloc((*tensor)->depth * sizeof(UCHAR**));
    RtlZeroMemory((*tensor)->RGBtensor, (*tensor)->depth * sizeof(UCHAR**));
    fors(
        (*tensor)->depth,
        ((*tensor)->RGBtensor)[j] = (UCHAR**)malloc((*tensor)->height * sizeof(UCHAR*));
    RtlZeroMemory(((*tensor)->RGBtensor)[j], (*tensor)->height * sizeof(UCHAR*));
        );
    forss(
        (*tensor)->depth, (*tensor)->height,
        ((*tensor)->RGBtensor)[j][i] = (UCHAR*)malloc((*tensor)->width * sizeof(UCHAR));
    RtlZeroMemory(((*tensor)->RGBtensor)[j][i], (*tensor)->width * sizeof(UCHAR));,
        );
    return;
}

void ExFreeRgbTensor(
    IN_CLEAR_OUT PRGB_TENSOR* tensor
)
{
    if (*tensor == NULL)
    {
        return;
    }
    forss(
        (*tensor)->depth, (*tensor)->height,
        ExFreeMem((PVOID*)&(*tensor)->RGBtensor[j][i]); ,
        ExFreeMem((PVOID*)&(*tensor)->RGBtensor[j]);
    );
    ExFreeMem((PVOID*)&(*tensor)->RGBtensor);
    ExFreeMem((PVOID*)&(*tensor));
    return;
}

double getDistanceToYellowMark(
    IN_OUT PRGB_TENSOR* rgbTensor
)
{
    size_t centerX = (*rgbTensor)->width / 2;
    size_t centerY = (*rgbTensor)->height / 2;
    size_t radius = 12;
    forss(
        (*rgbTensor)->width, (*rgbTensor)->height,
        size_t dx = centerX - j;
        size_t dy = centerY - i;
        if (sqrt((double)(dx * dx + dy * dy)) <= radius)
        {
            (*rgbTensor)->RGBtensor[0][i][j] = 0;
            (*rgbTensor)->RGBtensor[1][i][j] = 0;
            (*rgbTensor)->RGBtensor[2][i][j] = 0;
        },
    );
    forsss(
        (*rgbTensor)->depth, (*rgbTensor)->height, (*rgbTensor)->width,
        if (!((*rgbTensor)->RGBtensor[0][i][k] > 200 && (*rgbTensor)->RGBtensor[1][i][k] > 200 && (*rgbTensor)->RGBtensor[2][i][k] < 130))
        {
            (*rgbTensor)->RGBtensor[j][i][k] = 0;
        }
        , 1;, 1;
    );
    //transRgbTensor2ImgOnDisk(rgbTensor, L"E:\\desk\\sssss.png");
    int gap = 0;
    int row = 0;
    int col = 0;
    int dh = 0;
    int dw = 0;
    for (int j = (int)(*rgbTensor)->height - 1; j != 0; j--)
    {
        for (int i = 0; i < (int)(*rgbTensor)->width; i++)
        {
            if ((*rgbTensor)->RGBtensor[0][j][i] > 200 && (*rgbTensor)->RGBtensor[1][j][i] > 200 && (*rgbTensor)->RGBtensor[2][j][i] < 130)
            {
                row = j;
                col = i;
                goto ExitLoop;
            }
        }
    }
    return 9999.0;
ExitLoop:
    dh = row - (int)centerX;
    dw = col - (int)centerY;
    return (400.0 / 256.0) * sqrt((double)dh * dh + (double)dw * dw);
}

void modifyRgbTensor(
    IN_OUT PRGB_TENSOR* rgbTensor
)
{
    //getDistanceToYellowMark(rgbTensor);
    return;
}

void transRgbTensor2ImgOnDisk(
    IN_CLEAR_OUT PRGB_TENSOR* rgbTensor,
    IN CONST WCHAR* imgName
)
{
    PVOID img = malloc((*rgbTensor)->width * (*rgbTensor)->height * ((*rgbTensor)->depth + 1) + 0x36 + 0x36);
    *(USHORT*)((ULONG64)img) = 0x4D42;
    *(ULONG*)((ULONG64)img + 2) = (ULONG)((*rgbTensor)->width * (*rgbTensor)->height * ((*rgbTensor)->depth + 1) + 0x36);
    *(ULONG*)((ULONG64)img + 6) = 0xCDCDCDCD;
    *(ULONG*)((ULONG64)img + 10) = 0x00000036;
    *(ULONG*)((ULONG64)img + 14) = 0x00000028;
    *(ULONG*)((ULONG64)img + 18) = (ULONG)((*rgbTensor)->width);
    *(ULONG*)((ULONG64)img + 22) = (ULONG)((*rgbTensor)->height);
    *(USHORT*)((ULONG64)img + 26) = 0x0001;
    *(USHORT*)((ULONG64)img + 28) = 0x0020;
    *(ULONG*)((ULONG64)img + 30) = 0x00000000;
    *(ULONG*)((ULONG64)img + 34) = (ULONG)((*rgbTensor)->width * (*rgbTensor)->height * ((*rgbTensor)->depth + 1));
    RtlZeroMemory((PVOID)((ULONG64)img + 38), 16);
    size_t k = (*rgbTensor)->width * (*rgbTensor)->height * ((*rgbTensor)->depth + 1) - 4;
    for (size_t j = 0; j < ((*rgbTensor)->height); j++)
    {
        for (size_t i = 0; i < ((*rgbTensor)->width); i++)
        {
            *(UCHAR*)((ULONG64)img + 54 + k + 0) = (*rgbTensor)->RGBtensor[2][j][((*rgbTensor)->width) - i - 1];
            *(UCHAR*)((ULONG64)img + 54 + k + 1) = (*rgbTensor)->RGBtensor[1][j][((*rgbTensor)->width) - i - 1];
            *(UCHAR*)((ULONG64)img + 54 + k + 2) = (*rgbTensor)->RGBtensor[0][j][((*rgbTensor)->width) - i - 1];
            *(UCHAR*)((ULONG64)img + 54 + k + 3) = 0xFF;
            k -= 4;
        }
    }
    *(ULONG*)((ULONG64)img + 54 + (ULONG64)((*rgbTensor)->width * (*rgbTensor)->height * ((*rgbTensor)->depth + 1))) = 0xFDFDFDFD;
    RtlZeroMemory((PVOID)((ULONG64)img + 54 + (ULONG64)((*rgbTensor)->width * (*rgbTensor)->height * ((*rgbTensor)->depth + 1)) + 4), 0x32);
    HANDLE file = NULL;
    CONST WCHAR* path = (CONST WCHAR*)imgName;
    file = CreateFileW(
        path,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    WriteFile(file, img, (DWORD)((*rgbTensor)->width * (*rgbTensor)->height * ((*rgbTensor)->depth + 1) + 0x36 + 0x36), NULL, NULL);
    CloseHandle(file);
    ExFreeRgbTensor(rgbTensor);
    ExFreeMem((PVOID*)&img);
    return;
}

static void mallocMatrix(
    IN size_t row,
    IN size_t col,
    OUT PMatrix* matrix
)
{
    *matrix = (PMatrix)malloc(sizeof(Matrix));

    (*matrix)->row = row;
    (*matrix)->col = col;
    (*matrix)->matrix = (float**)malloc(row * sizeof(float*));

    fors(
        row,
        (*matrix)->matrix[j] = (float*)malloc(col * sizeof(float));
    );

    return;
}

static void freeMatrix(
    IN_OUT PMatrix* matrix
)
{
    fors(
        (*matrix)->row,
        ExFreeMem((PVOID*)&((*matrix)->matrix[j]));
        );

    ExFreeMem((PVOID*)&((*matrix)->matrix));
    ExFreeMem((PVOID*)&(*matrix));

    return;
}

myCudaError callCuda(
    IN float** leftMatrix,
    IN size_t leftMatrixRow,
    IN size_t leftMatrixCol,
    IN float** rightMatrix,
    IN size_t rightMatrixRow,
    IN size_t rightMatrixCol,
    IN myCudaCalculateFlag calculateFlag,
    OUT_HOST_PTR float*** result
)
{
    return CUDA_SUCCESS;
}