#include <napi.h>
#include "stdafx.h"
#include "imageUtilities.h"
#include "UtilsCaptureDevice.h"
#include "uiautomationclient.h"
#include "atlcomcli.h"
#include <codecvt>
#include <winuser.h>

struct {
	IDirect3DDevice9 *pDevice = nullptr;
	IDirect3DSurface9 *pSurface = nullptr;
	UINT32 uiWidth = 0;
	UINT32 uiHeight = 0;
} ssVars;

std::string _getFileName(TCHAR *filepath)
{
	char fname[_MAX_FNAME] = { 0 };
	errno_t err;

	std::string path = std::string(filepath);

	err = _splitpath_s(path.c_str(), NULL, 0, NULL, 0, &fname[0], _MAX_FNAME, NULL, 0);
	return std::string(fname);
}

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
//	hwnd: focused window
//	processName: application name
//	type: output desired.  1 for title, 2 for URL
std::wstring _getBrowserURL(HWND hwnd, std::string processName, int type)
{
	while (true)
	{
		//if (!hwnd)
		//	break;
		//if (!IsWindowVisible(hwnd))
		//	continue;

		CComQIPtr<IUIAutomation> uia;
		if (FAILED(uia.CoCreateInstance(CLSID_CUIAutomation)) || !uia)
			break;

		CComPtr<IUIAutomationElement> root;
		if (FAILED(uia->ElementFromHandle(hwnd, &root)) || !root)
			break;

		CComPtr<IUIAutomationCondition> condition;

		if (type == 1)
			uia->CreatePropertyCondition(UIA_ControlTypePropertyId,
				CComVariant(0xC363), &condition);
		else if (type == 2)
			uia->CreatePropertyCondition(UIA_ControlTypePropertyId,
				CComVariant(0xC354), &condition);

		CComPtr<IUIAutomationElement> edit;
		if (FAILED(root->FindFirst(TreeScope_Descendants, condition, &edit))
			|| !edit)
			continue;

		CComVariant url;
		if (type == 1)
			edit->GetCurrentPropertyValue(UIA_NamePropertyId, &url);
		else if (type == 2)
			edit->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &url);
		return std::wstring(url.bstrVal);
		break;
	}
	return S_OK;
}

// input:
//	pszExeFileName: process executable file path
// output:
//	imageBuffer: contains the icon in RGB32
//	width, height, size of the icon
//
HRESULT _getProcessIcon(LPCSTR pszExeFileName, BYTE **imageBuffer, DWORD *width, DWORD *height, DWORD *imageSize)
{
	HICON icon = ExtractIconA(GetModuleHandle(NULL), pszExeFileName, 0);
	auto iconInfo = imageUtilities::_MyGetIconInfo(icon);
	auto hbmp = imageUtilities::_HICONtoHBITMAP(icon, iconInfo.nWidth, iconInfo.nWidth);

	//create
	HDC hdcScreen = GetDC(NULL);
	HDC hdc = CreateCompatibleDC(hdcScreen);
	SelectObject(hdc, hbmp);

	BYTE *imgBuffer = new BYTE[iconInfo.nWidth * iconInfo.nHeight * 4];

	auto x = imageUtilities::_bitmapToBytes(hbmp, (int&)*width, (int&)*height);
	memcpy(imgBuffer, &x[0], (*width)*(*height) * 4);

	*imageBuffer = imgBuffer;
	*imageSize = x.size();

	//release
	DeleteDC(hdc);
	DeleteObject(hbmp);
	ReleaseDC(NULL, hdcScreen);
	return S_OK;
}


HRESULT _getScreenshot(BYTE **imageBuffer, DWORD *width, DWORD *height, DWORD *imageSize)
{
	HRESULT hr = ssVars.pDevice->GetFrontBufferData(0, ssVars.pSurface);

	D3DLOCKED_RECT rc;
	if (SUCCEEDED(hr))
	{
		hr = ssVars.pSurface->LockRect(&rc, NULL, 0);
	}

	BYTE *imgBuffer = new BYTE[ssVars.uiWidth * ssVars.uiHeight * 4];

	if (SUCCEEDED(hr))
	{
		memcpy(imgBuffer, rc.pBits, ssVars.uiWidth * ssVars.uiHeight * 4);
	}

	ssVars.pSurface->UnlockRect();

	if (SUCCEEDED(hr))
	{
		*imageSize = ssVars.uiWidth * ssVars.uiHeight * 4;
		*height = ssVars.uiHeight;
		*width = ssVars.uiWidth;
		*imageBuffer = imgBuffer;
	}
	else
	{
		printf("_getScreenshot error: 0x%X\n", hr);
	}
	return hr;
}

// input:
//	filename
// output:
//	true if the filename is a browser executable
//
bool _isItBrowser(TCHAR *filepath)
{
	std::string filename = _getFileName(filepath);

	if (filename == "msedge" || filename == "chrome" || filename == "brave" || filename == "opera" || filename == "firefox")
		return true;
	else
		return false;
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

	BYTE *imageBuffer = nullptr; DWORD imageSize = 0, snapshotWidth = 0, snapshotHeight = 0;
	//_getWindowSnapshot(focusedWindow, rc, &imageBuffer, &snapshotWidth, &snapshotHeight, &imageSize);
	_getScreenshot(&imageBuffer, &snapshotWidth, &snapshotHeight, &imageSize);

	// get icon of the process

	BYTE *iconBuffer = nullptr; DWORD iconSize = 0, iconWidth = 0, iconHeight = 0;
	_getProcessIcon(filename, &iconBuffer, &iconWidth, &iconHeight, &iconSize);
	
	obj.Set(Napi::String::New(env, "filename"), filename);
	obj.Set(Napi::String::New(env, "snapshot"), Napi::Buffer<uint8_t>::New(env, imageBuffer, imageSize));
	obj.Set(Napi::String::New(env, "snapshotWidth"), snapshotWidth);
	obj.Set(Napi::String::New(env, "snapshotHeight"), snapshotHeight);
	obj.Set(Napi::String::New(env, "icon"), Napi::Buffer<uint8_t>::New(env, iconBuffer, iconSize));
	obj.Set(Napi::String::New(env, "iconWidth"), iconWidth);
	obj.Set(Napi::String::New(env, "iconHeight"), iconHeight);

	// get last user activity (idle time) on the application

	LASTINPUTINFO lii;
	lii.cbSize = sizeof(LASTINPUTINFO);
	GetLastInputInfo(&lii);
	auto idleTime = (GetTickCount() - lii.dwTime)/1000.0;
	obj.Set(Napi::String::New(env, "userIdleTime"), idleTime);

	
	if (_isItBrowser(filename))
	{
		using convert_type = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_type, wchar_t> converter;
		std::string stringURL = converter.to_bytes(_getBrowserURL(focusedWindow, "", 2));
		obj.Set(Napi::String::New(env, "browserURL"), stringURL.c_str());

		std::string stringTitle = converter.to_bytes(_getBrowserURL(focusedWindow, "", 1));
		obj.Set(Napi::String::New(env, "browserTitle"), stringTitle.c_str());
	}
	return obj;
}

// input: none
// output: object with keys
//	
Napi::Object getFocusedDetail(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();
	Napi::Object obj = Napi::Object::New(env);
	uint8_t *x = new uint8_t[4];
	return obj;
}

// input: none
// output: true (microphone active) / false (not active)
//
Napi::Boolean isMicrophoneActive(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();

	// audio device
	bool active = UtilsCaptureDevice::isCaptureDeviceActive(true);

	return Napi::Boolean::New(env, active);
}

// input: none
// output: true (webcam active) / false (not active)
//
Napi::Boolean isWebcamActive(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();

	// not audio device (i.e., webcam)
	bool active = UtilsCaptureDevice::isCaptureDeviceActive(false);

	return Napi::Boolean::New(env, active);
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

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr))
	{
		hr = MFStartup(MF_VERSION);
	}

	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
	int result = SetProcessDPIAware();

	hr = imageUtilities::InitializeDirect3D9(&ssVars.pDevice, &ssVars.pSurface, 
		ssVars.uiWidth, ssVars.uiHeight);
	if (FAILED(hr))
	{
		printf("InitializeDirect3D9 failed, hr: 0x%X\n", hr);
	}
	return exports;
}

NODE_API_MODULE(hello, Init)
