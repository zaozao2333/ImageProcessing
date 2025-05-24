// ImageProcessing.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <direct.h>
#include <commdlg.h>

#define MAX_LOADSTRING 100

BOOL ReadImage(LPSTR, char *, int, int); //��ȡͼ����Ϣ��������Image[][]��
void ShowImage(char *, int, int, int, int);
BOOL ReadBmpImage(LPSTR, char *);
void ShowBmpImage(char *, int, int, int, int);
void OpenImageFileDlg(char *);

void ImageMaskProcessing(char* oImage, char* nImage, int wImage, int hImage,
	int* Mask, int MaskWH, int MaskCoff);
void ImageMaskProcessingBMP(char* oImage, char* nImage, int wImage, int hImage,
	int* Mask, int MaskWH, int MaskCoff);

void AverageMaskProcessing(char* oImage, char* nImage, int wImage, int hImage);
void AverageMaskProcessingBMP(char* oImage, char* nImage, int wImage, int hImage);

void GuassAverageMaskProcessing(char* oImage, char* nImage, int wImage, int hImage);
void GuassAverageMaskProcessingBMP(char* oImage, char* nImage, int wImage, int hImage);

void MaxFilterProcessing(char* OrgImage, char* NewImage, int wImage, int hImage);
void addRandomNoise(char* oImage, int wImage, int hImage, int noiseNum);

void MiddleFilterProcessing(char* OrgImage, char* NewImage, int wImage, int hImage, int windowSize);
void MiddleFilterProcessingBMP(char* OrgImage, char* NewImage, int wImage, int hImage, int windowSize);

void QuickSort(BYTE arr[], int size);

void LaplaceEdgeProcessing(char* oImage, char* nImage, int wImage, int hImage);
void LaplaceEdgeProcessingBMP(char* oImage, char* nImage, int wImage, int hImage);

HDC  hWinDC;
int ImageWidth, ImageHeight;
char ImgDlgFileName[MAX_PATH];
char ImgDlgFileDir[MAX_PATH];
char OrgImage[1024*1024];
#define IMAGEWIDTH	256
#define IMAGEHEIGHT	256
#define XPOS		100
#define YPOS		100

enum ImageType { IMAGE_RAW, IMAGE_BMP };
ImageType currentImageType = IMAGE_RAW; // Ĭ����ΪRAW

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];								// The title bar text

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_IMAGEPROCESSING, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_IMAGEPROCESSING);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_IMAGEPROCESSING);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_IMAGEPROCESSING;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
char NewImage[1024 * 1024 * 3];
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR szHello[MAX_LOADSTRING];
	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	switch (message) 
	{
		case WM_CREATE:
			hWinDC = GetWindowDC(hWnd);
			break;

		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_SHOWRAWIMAGE:
					OpenImageFileDlg("��ͼ���ļ�");
					ReadImage(ImgDlgFileName, OrgImage, IMAGEWIDTH, IMAGEHEIGHT);
					ShowImage(OrgImage, IMAGEWIDTH, IMAGEHEIGHT, XPOS, YPOS);
				   break;
				case IDM_SHOWBMPIMAGE:
					OpenImageFileDlg("��ͼ���ļ�");
					ReadBmpImage(ImgDlgFileName, OrgImage);
					ShowBmpImage(OrgImage, ImageWidth, ImageHeight, XPOS, YPOS);
				   break;
				case IDM_AVERAGEFILTER:				//ƽ���˲���
					if (currentImageType == IMAGE_RAW) {
						AverageMaskProcessing(OrgImage, NewImage, IMAGEWIDTH, IMAGEHEIGHT);
						ShowImage(NewImage, IMAGEWIDTH, IMAGEHEIGHT, XPOS, YPOS + 300);
					}
					else {
						AverageMaskProcessingBMP(OrgImage, NewImage, ImageWidth, ImageHeight);
						ShowBmpImage(NewImage, ImageWidth, ImageHeight, XPOS, YPOS + 600);
					}
					break;
				case IDM_GAUSSFILTER:				//��˹�˲���
					GuassAverageMaskProcessing(OrgImage, NewImage, IMAGEWIDTH, IMAGEHEIGHT);
					ShowImage(NewImage, IMAGEWIDTH, IMAGEHEIGHT, XPOS, YPOS + 300); 
					break;
				case IDM_MAXFILTER:				//���ֵ�˲�
					MaxFilterProcessing(OrgImage, NewImage, IMAGEWIDTH, IMAGEHEIGHT);
					ShowImage(NewImage, IMAGEWIDTH, IMAGEHEIGHT, XPOS, YPOS + 300); 
					break;
				case IDM_RANDOMNOISE:
					addRandomNoise(OrgImage, ImageWidth, ImageHeight, 5000);
					ShowBmpImage(OrgImage, ImageWidth, ImageHeight, XPOS, YPOS);
					break;
				case IDM_MIDDLEFILTER_3:
					if (currentImageType == IMAGE_RAW) {
						MiddleFilterProcessing(OrgImage, NewImage, IMAGEWIDTH, IMAGEHEIGHT, 3);
						ShowImage(NewImage, IMAGEWIDTH, IMAGEHEIGHT, XPOS, YPOS + 300);
					}
					else {
						MiddleFilterProcessingBMP(OrgImage, NewImage, ImageWidth, ImageHeight, 3);
						ShowBmpImage(NewImage, ImageWidth, ImageHeight, XPOS, YPOS + 600);
					}
					break;
				case IDM_MIDDLEFILTER_5:
					if (currentImageType == IMAGE_RAW) {
						MiddleFilterProcessing(OrgImage, NewImage, IMAGEWIDTH, IMAGEHEIGHT, 5);
						ShowImage(NewImage, IMAGEWIDTH, IMAGEHEIGHT, XPOS, YPOS + 300);
					}
					else {
						MiddleFilterProcessingBMP(OrgImage, NewImage, ImageWidth, ImageHeight, 5);
						ShowBmpImage(NewImage, ImageWidth, ImageHeight, XPOS, YPOS + 600);
					}
					break;
				case IDM_MIDDLEFILTER_7:
					if (currentImageType == IMAGE_RAW) {
						MiddleFilterProcessing(OrgImage, NewImage, IMAGEWIDTH, IMAGEHEIGHT, 7);
						ShowImage(NewImage, IMAGEWIDTH, IMAGEHEIGHT, XPOS, YPOS + 300);
					}
					else {
						MiddleFilterProcessingBMP(OrgImage, NewImage, ImageWidth, ImageHeight, 7);
						ShowBmpImage(NewImage, ImageWidth, ImageHeight, XPOS, YPOS + 600);
					}
					break;
				case IDM_LAPLACE:
					if (currentImageType == IMAGE_RAW) {
						LaplaceEdgeProcessing(OrgImage, NewImage, IMAGEWIDTH, IMAGEHEIGHT);
						ShowImage(NewImage, IMAGEWIDTH, IMAGEHEIGHT, XPOS, YPOS + 300);
					}
					else {
						LaplaceEdgeProcessingBMP(OrgImage, NewImage, ImageWidth, ImageHeight);
						ShowBmpImage(NewImage, ImageWidth, ImageHeight, XPOS, YPOS + 600);
					}
					break;
				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			RECT rt;
			GetClientRect(hWnd, &rt);
			DrawText(hdc, szHello, strlen(szHello), &rt, DT_CENTER);
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}




/************************************************************************************************
*																								*
*   ���޸�ʽRAWͼ���ļ�																			*
*																								*
************************************************************************************************/
BOOL ReadImage(LPSTR ImageFileName, char *oImage, int wImage, int hImage) //��ȡͼ����Ϣ��������Image[][]��
{
	OFSTRUCT of;
	HFILE Image_fp;

	Image_fp = OpenFile(ImageFileName, &of, OF_READ);
	if (Image_fp == HFILE_ERROR) 
	{
		MessageBox(NULL, ImageFileName, "���ļ�������Ϣ", MB_OK);
		return FALSE;
	}

	_lread(Image_fp, oImage, wImage*hImage);
	_lclose(Image_fp);

	currentImageType = IMAGE_RAW;

	return TRUE;
}

/************************************************************************************************
*																								*
*   ��ʾRAWͼ��																					*
*																								*
************************************************************************************************/
void ShowImage(char *Image, int wImage, int hImage, int xPos, int yPos)
{
	int i,j;

	for (i=0; i<hImage; i++) {
		for (j=0; j<wImage; j++) {
			SetPixel(hWinDC, j+yPos, i+xPos, RGB(Image[i*wImage+j],Image[i*wImage+j],Image[i*wImage+j]));
		}
	}
}

/************************************************************************************************
*																								*
*   ��BMPͼ���У�����ͼ��ͷ��Ϣ����Ҫ����ͼ�񳤶ȺͿ��											*
*																								*
************************************************************************************************/
BOOL ReadBmpImage(LPSTR ImageFileName, char *oImage)
{ 
	BITMAPFILEHEADER bfImage;
	BITMAPINFOHEADER biImage;

	OFSTRUCT of;
	HFILE Image_fp;

	Image_fp = OpenFile(ImageFileName, &of, OF_READ);
	if (Image_fp == HFILE_ERROR) 
	{
		MessageBox(NULL, ImageFileName, "���ļ�������Ϣ", MB_OK);
		return FALSE;
	}

	_llseek(Image_fp, 0, 0);
	_lread(Image_fp, &bfImage, sizeof(BITMAPFILEHEADER));

	if ((bfImage.bfType != 0x4d42)) {		// "BM"
		MessageBox(NULL, NULL, "��bmp�ļ�������Ϣ", MB_OK);
		return FALSE;
	}

	_lread(Image_fp, &biImage, sizeof(BITMAPINFOHEADER));

	ImageWidth = biImage.biWidth;
	ImageHeight= biImage.biHeight;
	if (biImage.biBitCount != 24) {		// 24λ��ɫͼ��
		MessageBox(NULL, NULL, "����24λbmpͼ��", MB_OK);
		return FALSE;
	}

	_llseek(Image_fp, bfImage.bfOffBits, 0);
	_lread(Image_fp, oImage, biImage.biWidth*biImage.biHeight*3);
	_lclose(Image_fp);

	currentImageType = IMAGE_BMP;

	return TRUE;
}

/************************************************************************************************
*																								*
*   ��ʾBMPͼ��																					*
*																								*
************************************************************************************************/
void ShowBmpImage(char *Image, int wImage, int hImage, int xPos, int yPos)
{
	//�����·�������ʾBMP��ʽͼ��������
	int i, j;
	for (i = 0; i < hImage; i++) {
		for (j = 0; j < wImage; j++) {
			// �����������ݵ�λ�ã�ʹ��(hImage - 1 - i)����ת��˳��
			int index = ((hImage - i - 1) * wImage + j) * 3; // �ײ������ȴ洢

			BYTE blue = Image[index];   // B
			BYTE green = Image[index + 1]; // G
			BYTE red = Image[index + 2];  // R

			SetPixel(hWinDC, j + yPos, i + xPos,
				RGB(red, green, blue));
		}
	}
}
/************************************************************************************************
*																								*
*   �򿪶��ļ���ϵͳ�Ի���																		*
*																								*
************************************************************************************************/
void OpenImageFileDlg(char *OPDLTitle)
{
	char FileTitle[100];
	OPENFILENAME ofn;

	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=NULL;
	ofn.hInstance=NULL;
	ofn.lpstrFilter=TEXT("Supported Files\0*.bmp;*.raw\0\0");
	ofn.lpstrCustomFilter=NULL;
	ofn.nMaxCustFilter=0;
	ofn.nFilterIndex=1;
	ofn.lpstrFile=ImgDlgFileName;
	ofn.nMaxFile=MAX_PATH;
	ofn.lpstrFileTitle=FileTitle;
	ofn.nMaxFileTitle=99;
	ofn.lpstrInitialDir=ImgDlgFileDir;
	ofn.lpstrTitle=OPDLTitle;
	ofn.Flags=OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt="raw";
	ofn.lCustData=NULL;
	ofn.lpfnHook=NULL;
	ofn.lpTemplateName=NULL;
	ImgDlgFileName[0]='\0';
	GetOpenFileName(&ofn); 

	getcwd(ImgDlgFileDir, MAX_PATH);
}

void LaplaceEdgeProcessing(char* oImage, char* nImage,
	int wImage, int hImage)
{
	int Mask[9] = { 0, 1, 0, 			//Laplace��Ե���ģ��
			   1,-4, 1,
			   0, 1, 0 };

	ImageMaskProcessing(oImage, nImage, wImage, hImage, Mask, 3, 1);
}

void LaplaceEdgeProcessingBMP(char* oImage, char* nImage,
	int wImage, int hImage)
{
	int Mask[9] = { 0, 1, 0, 			//Laplace��Ե���ģ��
			   1,-4, 1,
			   0, 1, 0 };

	ImageMaskProcessingBMP(oImage, nImage, wImage, hImage, Mask, 3, 1);
}

void ImageMaskProcessing(char* oImage, char* nImage, int wImage, int hImage,
	int* Mask, int MaskWH, int MaskCoff)
{
	int Coff;	int i, j, m, n;	int k;
	k = (MaskWH - 1) / 2;
	for (i = k; i < hImage - k; i++) {
		for (j = k; j < wImage - k; j++) {
			Coff = 0;
			for (m = -k; m <= k; m++) {
				for (n = -k; n <= k; n++) {
					Coff += (BYTE)oImage[(i + m) * wImage + (j + n)] *
						Mask[(m + k) * MaskWH + (n + k)];
				}
			}
			nImage[i * wImage + j] = (unsigned char)(Coff / MaskCoff);
			if (Coff < 0) nImage[i * wImage + j] = 0;
		}
	}
}

void ImageMaskProcessingBMP(char* oImage, char* nImage, int wImage, int hImage,
	int* Mask, int MaskWH, int MaskCoff)
{
	int Coff;
	int i, j, m, n;
	int k = (MaskWH - 1) / 2;

	for (i = k; i < hImage - k; i++) {
		for (j = k; j < wImage - k; j++) {
			// ����ÿ����ɫͨ�� (B, G, R)
			for (int channel = 0; channel < 3; channel++) {
				Coff = 0;
				for (m = -k; m <= k; m++) {
					for (n = -k; n <= k; n++) {
						// ����ԭʼͼ���е�����λ��
						int srcIndex = ((hImage - (i + m) - 1) * wImage + (j + n)) * 3 + channel;
						Coff += (BYTE)oImage[srcIndex] * Mask[(m + k) * MaskWH + (n + k)];
					}
				}
				// ���㴦���ͼ���е�����λ��
				int dstIndex = ((hImage - i - 1) * wImage + j) * 3 + channel;
				nImage[dstIndex] = (unsigned char)(Coff / MaskCoff);
				if (Coff < 0) nImage[dstIndex] = 0;
			}
		}
	}
}


void AverageMaskProcessing(char* oImage, char* nImage,	int wImage, int hImage)
{
	int Mask[9] = { 1, 1, 1,
			   1, 1, 1,
			   1, 1, 1 };

	ImageMaskProcessing(oImage, nImage, wImage, hImage, Mask, 3, 9);
}

void GuassAverageMaskProcessing(char* oImage, char* nImage, int wImage, int hImage)
{
	int Mask[9] = { 1, 2, 1,
			   2, 4, 2,
			   1, 2, 1 };

	ImageMaskProcessing(oImage, nImage, wImage, hImage, Mask, 3, 16);
}

void AverageMaskProcessingBMP(char* oImage, char* nImage, int wImage, int hImage)
{
	int Mask[9] = { 1, 1, 1,
					1, 1, 1,
					1, 1, 1 };

	ImageMaskProcessingBMP(oImage, nImage, wImage, hImage, Mask, 3, 9);
}

void GuassAverageMaskProcessingBMP(char* oImage, char* nImage, int wImage, int hImage)
{
	int Mask[9] = { 1, 2, 1,
					2, 4, 2,
					1, 2, 1 };

	ImageMaskProcessingBMP(oImage, nImage, wImage, hImage, Mask, 3, 16);
}

void MaxFilterProcessing(char* OrgImage, char* NewImage, int wImage, int hImage) {
    int i, j, m, n; 
    int k = 1;       
    int maxVal;      

    for (i = k; i < hImage - k; i++) {
        for (j = k; j < wImage - k; j++) {
            maxVal = 0;  
            for (m = -k; m <= k; m++) {
                for (n = -k; n <= k; n++) {
                    int pixelValue = (BYTE)OrgImage[(i + m) * wImage + (j + n)];
                    if (pixelValue > maxVal) {
                        maxVal = pixelValue;
                    }
                }
            }

            NewImage[i * wImage + j] = (unsigned char)maxVal;
        }
    }
}

void addRandomNoise(char* oImage, int wImage, int hImage, int noiseNum) {
	int i;
	int x, y;
	int index;

	for (i = 0; i < noiseNum; i++) {
		x = rand() % wImage;
		y = rand() % hImage;

		index = ((hImage - y - 1) * wImage + x) * 3;
		oImage[index] = 255;
		oImage[index + 1] = 255;
		oImage[index + 2] = 255;
	}
}

void MiddleFilterProcessing(char* OrgImage, char* NewImage, int wImage, int hImage, int windowSize)
{

	int i, j, m, n, k;
	int halfSize = windowSize / 2;
	int windowArea = windowSize * windowSize;

	// ��̬������ʱ�������ڴ洢��������ֵ
	BYTE* window = new BYTE[windowArea];

	for (i = 1; i < hImage - 1; i++) {
		for (j = 1; j < wImage - 1; j++) {
			int count = 0;

			// �ռ������ڵ�����ֵ
			for (m = -halfSize; m <= halfSize; m++) {
				for (n = -halfSize; n <= halfSize; n++) {
					window[count] = (BYTE)OrgImage[(i + m) * wImage + (j + n)];
					count++;
				}
			}

			QuickSort(window, windowArea); 

			// ȡ��ֵ(�������м�Ԫ��)
			int medianPos = window[windowArea / 2];

			// ���㴦���ͼ���е�����λ��
			NewImage[i * wImage + j] = (unsigned char)medianPos;
		}
	}

	// �ͷ���ʱ����
	delete[] window;
}

// BMPͼ����ֵ�˲�������
void MiddleFilterProcessingBMP(char* OrgImage, char* NewImage, int wImage, int hImage, int windowSize)
{

	int i, j, m, n, k;
	k = (windowSize - 1) / 2;
	int windowArea = windowSize * windowSize;

	// ��̬������ʱ�������ڴ洢��������ֵ
	BYTE* windowR = new BYTE[windowArea];
	BYTE* windowG = new BYTE[windowArea];
	BYTE* windowB = new BYTE[windowArea];

	for (i = k; i < hImage - k; i++) {
		for (j = k; j < wImage - k; j++) {
			int count = 0;

			// �ռ������ڵ�����ֵ
			for (m = -k; m <= k; m++) {
				for (n = -k; n <= k; n++) {
					// ����ԭʼͼ���е�����λ��
					int srcIndex = ((hImage - (i + m) - 1) * wImage + (j + n)) * 3;

					windowB[count] = (BYTE)OrgImage[srcIndex];     // ��ɫͨ��
					windowG[count] = (BYTE)OrgImage[srcIndex + 1]; // ��ɫͨ��
					windowR[count] = (BYTE)OrgImage[srcIndex + 2]; // ��ɫͨ��
					count++;
				}
			}

			// ��ÿ��ͨ��������ֵ��������
			QuickSort(windowB, windowArea); // ��ɫͨ������
			QuickSort(windowG, windowArea); // ��ɫͨ������
			QuickSort(windowR, windowArea); // ��ɫͨ������

			// ȡ��ֵ(�������м�Ԫ��)
			int medianPos = windowArea / 2;

			// ���㴦���ͼ���е�����λ��
			int dstIndex = ((hImage - i - 1) * wImage + j) * 3;

			NewImage[dstIndex] = windowB[medianPos]; // ��ɫͨ����ֵ
			NewImage[dstIndex + 1] = windowG[medianPos]; // ��ɫͨ����ֵ
			NewImage[dstIndex + 2] = windowR[medianPos]; // ��ɫͨ����ֵ
		}
	}

	// �ͷ���ʱ����
	delete[] windowR;
	delete[] windowG;
	delete[] windowB;
}

// ���������㷨ʵ��(��ð���������Ч���ر��Ƕ��ڴ󴰿�)
void QuickSort(BYTE arr[], int size)
{
	if (size <= 1) return;

	BYTE pivot = arr[size / 2];
	int i = 0, j = size - 1;

	while (i <= j) {
		while (arr[i] < pivot) i++;
		while (arr[j] > pivot) j--;
		if (i <= j) {
			BYTE temp = arr[i];
			arr[i] = arr[j];
			arr[j] = temp;
			i++;
			j--;
		}
	}

	if (j > 0) QuickSort(arr, j + 1);
	if (i < size) QuickSort(arr + i, size - i);
}


