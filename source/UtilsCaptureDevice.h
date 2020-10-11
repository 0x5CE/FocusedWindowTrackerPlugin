#pragma once
#include "stdafx.h"

namespace UtilsCaptureDevice
{
	HRESULT StartStopCaptureDevice(DWORD devID, bool start, IMFMediaSource **ppSource, DWORD *countDevices, bool verbose, bool audioDevice);
	bool isCaptureDeviceActive(bool audioDevice);

};