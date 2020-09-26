#include <napi.h>
#include "build\stdafx.h"
#include <psapi.h>


// input: none
// output:
//	focusedWindow: handle of the focused windows
//	rc:	window rectangle coordinates
//	focusedPID: process ID of the window
//
HRESULT _getFocusedWindowInfo(HWND *focusedWindow, RECT *rc, DWORD *focusedPID)
{
	*focusedWindow = GetForegroundWindow();
	GetWindowRect(*focusedWindow, rc);
	GetWindowThreadProcessId(*focusedWindow, focusedPID);
	return S_OK;
}

// input:
//	BitmapHandle
// output:
//	width, height of the snapshot
//	byte map of the snapshot
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

// input:
//	focusedWindow, rc
// output:
//	imageBuffer: contains the snapshot of the window
//	width, height, size of the snapshot
//
HRESULT _getWindowSnapshot(HWND focusedWindow, RECT rc, BYTE **imageBuffer, DWORD *width, DWORD *height, DWORD *imageSize)
{
	//create
	HDC hdcScreen = GetDC(NULL);
	HDC hdc = CreateCompatibleDC(hdcScreen);
	HBITMAP hbmp = CreateCompatibleBitmap(hdcScreen,
		(rc.right - rc.left), (rc.bottom - rc.top));
	SelectObject(hdc, hbmp);

	auto imgSize = (rc.right - rc.left) * (rc.bottom - rc.top) * 4;
	BYTE *imgBuffer = new BYTE[(rc.right - rc.left) * (rc.bottom - rc.top) * 4];

	//Print to memory hdc
	PrintWindow(focusedWindow, hdc, PW_RENDERFULLCONTENT);

	//auto bitmapSize = bitmap.bmWidth * bitmap.bmHeight * bitmap.bmBitsPixel / 8;
	auto x = _bitmapToBytes(hbmp, (int&)*width, (int&)*height);
	memcpy(imgBuffer, &x[0], (*width)*(*height)*4);

	*imageBuffer = imgBuffer;
	*imageSize = x.size();

	//release
	DeleteDC(hdc);
	DeleteObject(hbmp);
	ReleaseDC(NULL, hdcScreen);
	return S_OK;
}

// input:
//	PID: process ID
// output:
//	filename: file path of the process
//
HRESULT _getProcessFilename(DWORD PID, TCHAR *filename)
{
	auto processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, PID);
	if (processHandle != NULL)
	{
		if (GetModuleFileNameEx(processHandle, NULL, filename, MAX_PATH) == 0)
		{
			printf("GetModuleFileName failed\n");
		}
		else
		{
			//	printf("Module filename is: %S\n", filename);
		}
		CloseHandle(processHandle);
	}
	else
	{
		printf("OpenProcess failed\n");
	}
	return S_OK;
}

// input: none
// output:	object with keys
//	filename (Napi::String): the path of the executable
//	
Napi::Object getFocusedImageAndDetail(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();
	Napi::Object obj = Napi::Object::New(env);

	// get  focused window info

	HWND focusedWindow;  RECT rc; DWORD focusedPID;
	_getFocusedWindowInfo(&focusedWindow, &rc, &focusedPID);

	// get file name of the program

	TCHAR filename[MAX_PATH];
	_getProcessFilename(focusedPID, filename);

	// get snapshot of the window
	BYTE *imageBuffer = nullptr; DWORD imageSize = 0, width = 0, height = 0;

	_getWindowSnapshot(focusedWindow, rc, &imageBuffer, &width, &height, &imageSize);

	obj.Set(Napi::String::New(env, "filename"), filename);
	obj.Set(Napi::String::New(env, "snapshot"), Napi::Buffer<uint8_t>::New(env, imageBuffer, imageSize));
	obj.Set(Napi::String::New(env, "snapshotWidth"), width);
	obj.Set(Napi::String::New(env, "snapshotHeight"), height);

	return obj;
}

// input: none
// output:	object with keys
//	
Napi::Object getFocusedDetail(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();
	Napi::Object obj = Napi::Object::New(env);
	uint8_t *x = new uint8_t[4];
	return obj;
}

// input: none
// output: true (microphone active) / false (not active)
Napi::Boolean isMicrophoneActive(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();
	return Napi::Boolean::New(env, false);
}

// input: none
// output: true (webcam active) / false (not active)
Napi::Boolean isWebcamActive(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();
	return Napi::Boolean::New(env, false);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
	exports.Set(Napi::String::New(env, "getFocusedImageAndDetail"),
		Napi::Function::New(env, getFocusedImageAndDetail));
	exports.Set(Napi::String::New(env, "getFocusedDetail"),
		Napi::Function::New(env, getFocusedDetail));
	exports.Set(Napi::String::New(env, "isMicrophoneActive"),
		Napi::Function::New(env, isMicrophoneActive));
	exports.Set(Napi::String::New(env, "isWebcamActive"),
		Napi::Function::New(env, isWebcamActive));

	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
	int result = SetProcessDPIAware();

	return exports;
}

NODE_API_MODULE(hello, Init)
