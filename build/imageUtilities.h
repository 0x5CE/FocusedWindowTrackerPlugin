#pragma once
#include "stdafx.h"

namespace imageUtilities
{
	struct MYICON_INFO
	{
		int     nWidth;
		int     nHeight;
		int     nBitsPerPixel;
	};

	HBITMAP _HICONtoHBITMAP(HICON hIcon, int x, int y);
	imageUtilities::MYICON_INFO _MyGetIconInfo(HICON hIcon);
	std::vector<unsigned char> _bitmapToBytes(HBITMAP BitmapHandle, int &width, int &height);
}