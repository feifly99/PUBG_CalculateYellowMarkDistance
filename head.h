#ifndef __GDI_GRAB_SCREEN__
#define __GDI_GRAB_SCREEN__

#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <math.h>

#pragma warning(disable:6387)
#pragma warning(disable:6385)
#pragma warning(disable:6011)
#pragma warning(disable:28183)

#ifndef YES
#define YES TRUE
#endif

#ifndef NO
#define NO FALSE
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef IN_OUT
#define IN_OUT
#endif

#ifndef IN_OPT
#define IN_OPT
#endif

#ifndef OUT_HOST_PTR
#define OUT_HOST_PTR
#endif

#define OUT_PTR OUT_HOST_PTR

#ifndef OUT_DEVICE_PTR
#define OUT_DEVICE_PTR
#endif

#define MAX_DIMENSION_SIZE 3

#define CLAMP(x, low, high) ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))

#define QAQ printf("\n")

#define ckFloatValueOnly(sen) printf("%.3f\n", (float)(sen))
#define ckFloatValueWithExplain(sen) printf("%s -> %.3f\n", (const char*)#sen, (float)(sen))

#define ckSizeTValueOnly(sen) printf("%zu\n", (SIZE_T)(sen))
#define ckSizeTValueWithExplain(sen) printf("%s -> %zu\n", (const char*)#sen, (SIZE_T)(sen))

#define IN_CLEAR_OUT
#define SHOW_LOG

#ifndef ASSERT
#define ASSERT(p) (p) ? 1:exit(0xFFF);
#endif

#ifndef NANO_TYPE
#define NANO_TYPE UCHAR
#endif

#define fors(times, sentence) \
do\
{\
	for(size_t j = 0; j < (times); j++)\
	{\
		sentence\
	}\
}while(0)

#define forss(times1, times2, sentenceInner, sentenceOuter) \
do\
{\
	for(size_t j = 0; j < (times1); j++)\
	{\
		for(size_t i = 0; i < (times2); i++)\
		{\
			sentenceInner\
		}\
		sentenceOuter\
	}\
}while(0)

#define forsss(times1, times2, times3, sentenceInnerMost, sentenceInnerSecond, sentenceOuter) \
do\
{\
	for(size_t j = 0; j < (times1); j++)\
	{\
		for(size_t i = 0; i < (times2); i++)\
		{\
			for(size_t k = 0; k < (times3); k++)\
		    {\
			    sentenceInnerMost\
		    }\
            sentenceInnerSecond\
		}\
		sentenceOuter\
	}\
}while(0)

typedef struct _Point
{
    double x;
    double y;
}Point, * PPoint;

typedef struct _vector
{
    double a;
    double b;
}V, * PV, ** PPV;

typedef struct _img
{
    PBITMAPFILEHEADER filePointer;
    PBITMAPINFOHEADER infoPointer;
    PVOID pixelPointer;
}IMG, * PIMG;

typedef struct _pixelRGB
{
    UCHAR R;
    UCHAR G;
    UCHAR B;
}pxlRGB, * ppxlRGB;

typedef struct _RGB_TENSOR
{
    size_t width;
    size_t height;
    size_t depth;
    UCHAR*** RGBtensor;
}RGB_TENSOR, * PRGB_TENSOR;

void ExFreeMem(
    PVOID* mem
);

/*CPU begin*/

void initializeInfoHeader(
    IN BITMAP bmpScreen,
    OUT BITMAPINFOHEADER* bi
);

void initializeFileHeader(
    OUT PBITMAPFILEHEADER imgFileHeader,
    IN DWORD sizeHeaderAndInfo,
    IN DWORD wholeSize,
    IN WORD imgHeadMarkCode
);

void initializePixelHeader(
    OUT_PTR PVOID* imgPixelHeader,
    IN LONG imgPixelSize,
    IN HDC nakedScreenImage,
    IN HBITMAP hiddenImageInstance,
    IN BITMAP img,
    IN BITMAPINFO* imgInfoHeader
);

void createImageFile(
    IN WCHAR* w_filePath,
    OUT HANDLE* hFile
);

SIZE_T getTotalBitMapSize(
    IN DWORD dwBmpSize
);

LONG getBmpSize(
    IN BITMAP bmpScreen,
    IN BITMAPINFOHEADER bi
);

void saveImg2Disk(
    OUT HANDLE* hFile,
    CONST WCHAR* fatherLocWithNoMark,
    CONST WCHAR* imgName,
    IN PBITMAPFILEHEADER imgFileHeader,
    IN PBITMAPINFOHEADER imgInfoHeader,
    IN PVOID imgPixelHeader,
    IN DWORD wholeSize
);

void transScreenDC2Img(
    IN HDC screenDC,
    OUT BITMAP* img,
    IN Point lt,
    IN Point rb,
    OUT_PTR HDC** _hiddenImgCode,
    OUT_PTR HBITMAP** _hiddenImageInstance
);

PIMG makeImg(
    IN BITMAP img,
    IN_CLEAR_OUT HDC* screenDC,
    IN_CLEAR_OUT HDC** memDC,
    IN_CLEAR_OUT HBITMAP** memImg,
    IN BOOLEAN isSaveToDisk,
    IN_OPT CONST WCHAR* name
);

void makeImgRgbTensor(
    IN_CLEAR_OUT PIMG* img,
    PRGB_TENSOR* tensor
);

void createRgbTensor(
    IN size_t width,
    IN size_t height,
    IN size_t depth,
    IN_OUT PRGB_TENSOR* tensor
);

void ExFreeRgbTensor(
    IN_CLEAR_OUT PRGB_TENSOR* tensor
);

void ConvertToGrayscale(
    IN_OUT PRGB_TENSOR* rgbTensor
);

double getDistanceToYellowMark(
    IN_OUT PRGB_TENSOR* rgbTensor
);

void modifyRgbTensor(
    IN_OUT PRGB_TENSOR* rgbTensor
);

void transRgbTensor2ImgOnDisk(
    IN_CLEAR_OUT PRGB_TENSOR* rgbTensor,
    IN CONST WCHAR* imgName
);

/*CPU end*/

/*GPU begin*/

typedef enum _myCudaError
{
    CUDA_SUCCESS = 0,
    CUDA_INVALID_OPERAND_TYPE = 0x01,
    CUDA_INVALID_CALCULATION_TYPE = 0x02,
    CUDA_DATA_DIMENSION_MISMATCH = 0x04,
    CUDA_MEMORY_ALLOC_FAILED = 0x08
}myCudaError;

typedef enum _myCudaCalculateFlag
{
    //所有的输入数据皆视为矩阵
    //设左操作数为A，右操作数为B
    //设A有Ar行，Ac列；B有Br行，Bc列.
    CAUCULATE_ADD = 0x1000, //*保证Ar == Br && Ac == Bc* -> 返回标准矩阵相加，左边的A(i, j)加右边的B(i, j)为C(i, j);
    CAUCULATE_DIRECT_MULTIPLE = 0x2000,//*保证Ac == Br* -> 返回标准矩阵相乘，左边的A的第i行乘右边的B的第j列为C(i, j);
    CAUCULATE_HARDMARD_MULTIPLE = 0x4000, //*保证Ar == Br && Ac == Bc* -> 返回哈达积，左边的A(i, j)乘右边的B(i, j)为C(i, j);
}myCudaCalculateFlag;

typedef struct _matrix
{
    size_t row;
    size_t col;
    float** matrix;
}Matrix, * PMatrix;

void mallocMatrix(
    IN size_t row,
    IN size_t col,
    OUT PMatrix* matrix
);

void freeMatrix(
    IN_OUT PMatrix* matrix
);

myCudaError callCuda(
    IN float** leftMatrix,
    IN size_t leftMatrixRow,
    IN size_t leftMatrixCol,
    IN float** rightMatrix,
    IN size_t rightMatrixRow,
    IN size_t rightMatrixCol,
    IN myCudaCalculateFlag calculateFlag,
    OUT_HOST_PTR float*** result
);

/*GPU end*/

#endif