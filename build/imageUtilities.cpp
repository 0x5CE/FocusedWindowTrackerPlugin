/*
	Utility functions to manipulate win32 images, icons, and stuff;
*/

#include "imageUtilities.h"

namespace imageUtilities
{
	// converts HICON to HBITMAP format
	// input:
	//	hIcon, x (width), y (height)
	// output:
	//	resulting HBITmAP
	//
	HBITMAP _HICONtoHBITMAP(HICON hIcon, int x, int y)
	{
		HDC hDC = GetDC(NULL);
		HDC hMemDC = CreateCompatibleDC(hDC);
		HBITMAP hMemBmp = CreateCompatibleBitmap(hDC, x, y);
		HBITMAP hResultBmp = NULL;
		HGDIOBJ hOrgBMP = SelectObject(hMemDC, hMemBmp);

		DrawIconEx(hMemDC, 0, 0, hIcon, x, y, 0, NULL, DI_NORMAL);

		hResultBmp = hMemBmp;
		hMemBmp = NULL;

		SelectObject(hMemDC, hOrgBMP);
		DeleteDC(hMemDC);
		ReleaseDC(NULL, hDC);
		DestroyIcon(hIcon);
		return hResultBmp;
	}

	// input:
	//	BitmapHandle
	// output:
	//	width, height of the snapshot
	//	byte map of the snapshot
	//
	std::vector<unsigned char> _bitmapToBytes(HBITMAP BitmapHandle, int &width, int &height)
	{
		BITMAP Bmp = { 0 };
		BITMAPINFO Info = { 0 };
		std::vector<unsigned char> Pixels = std::vector<unsigned char>();

		HDC DC = CreateCompatibleDC(NULL);
		std::memset(&Info, 0, sizeof(BITMAPINFO));
		HBITMAP OldBitmap = (HBITMAP)SelectObject(DC, BitmapHandle);
		GetObject(BitmapHandle, sizeof(Bmp), &Bmp);

		Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		Info.bmiHeader.biWidth = width = Bmp.bmWidth;
		Info.bmiHeader.biHeight = height = Bmp.bmHeight;
		Info.bmiHeader.biHeight = -Info.bmiHeader.biHeight;
		Info.bmiHeader.biPlanes = 1;
		Info.bmiHeader.biBitCount = Bmp.bmBitsPixel;
		Info.bmiHeader.biCompression = BI_RGB;
		Info.bmiHeader.biSizeImage = ((width * Bmp.bmBitsPixel + 31) / 32) * 4 * height;

		Pixels.resize(Info.bmiHeader.biSizeImage);
		GetDIBits(DC, BitmapHandle, 0, height, &Pixels[0], &Info, DIB_RGB_COLORS);
		SelectObject(DC, OldBitmap);

		height = std::abs(height);
		DeleteDC(DC);
		return Pixels;
	}

	// get icon info
	// input:
	//	hIcon
	// output:
	//	MYICON_INFO struct containing icon width, height, bits per pixel
	//
	MYICON_INFO _MyGetIconInfo(HICON hIcon)
	{
		MYICON_INFO myinfo;
		ZeroMemory(&myinfo, sizeof(myinfo));

		ICONINFO info;
		ZeroMemory(&info, sizeof(info));

		BOOL bRes = FALSE;

		bRes = GetIconInfo(hIcon, &info);
		if (!bRes)
			return myinfo;

		BITMAP bmp;
		ZeroMemory(&bmp, sizeof(bmp));

		if (info.hbmColor)
		{
			const int nWrittenBytes = GetObject(info.hbmColor, sizeof(bmp), &bmp);
			if (nWrittenBytes > 0)
			{
				myinfo.nWidth = bmp.bmWidth;
				myinfo.nHeight = bmp.bmHeight;
				myinfo.nBitsPerPixel = bmp.bmBitsPixel;
			}
		}
		else if (info.hbmMask)
		{
			// Icon has no color plane, image data stored in mask
			const int nWrittenBytes = GetObject(info.hbmMask, sizeof(bmp), &bmp);
			if (nWrittenBytes > 0)
			{
				myinfo.nWidth = bmp.bmWidth;
				myinfo.nHeight = bmp.bmHeight / 2;
				myinfo.nBitsPerPixel = 1;
			}
		}

		if (info.hbmColor)
			DeleteObject(info.hbmColor);
		if (info.hbmMask)
			DeleteObject(info.hbmMask);

		return myinfo;
	}

}