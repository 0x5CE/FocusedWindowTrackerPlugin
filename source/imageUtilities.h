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
	HRESULT InitializeDirect3D9(IDirect3DDevice9** ppDevice, IDirect3DSurface9** ppSurface, UINT32& uiWidth, UINT32& uiHeight);
}