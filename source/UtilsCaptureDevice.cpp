#include "UtilsCaptureDevice.h"

bool UtilsCaptureDevice::isCaptureDeviceActive(bool audioDevice)
{
	IMFMediaSource *pSource = nullptr; DWORD devCount; IMFSourceReader *pReader = nullptr; 	IMFSample *pSample = nullptr;
	HRESULT hr = UtilsCaptureDevice::StartStopCaptureDevice(0, true, &pSource, &devCount, false, audioDevice);
	if (SUCCEEDED(hr))
	{
		hr = MFCreateSourceReaderFromMediaSource(pSource, nullptr, &pReader);
	}
	if (SUCCEEDED(hr))
	{
		DWORD streamIndex;
		DWORD streamFlags;
		LONGLONG timeindex;
		DWORD sindex = audioDevice ? MF_SOURCE_READER_FIRST_AUDIO_STREAM : MF_SOURCE_READER_FIRST_VIDEO_STREAM;
		hr = pReader->ReadSample(sindex, 0, &streamIndex, &streamFlags, &timeindex, &pSample);
	}
	if (SUCCEEDED(hr))
	{
		SafeRelease(&pReader);
		SafeRelease(&pSample);
		SafeRelease(&pSource);
		hr = UtilsCaptureDevice::StartStopCaptureDevice(0, false, &pSource, &devCount, false, audioDevice);
		return false;
	}
	hr = UtilsCaptureDevice::StartStopCaptureDevice(0, false, &pSource, &devCount, false, audioDevice);
	SafeRelease(&pSource);
	return true;
}

HRESULT UtilsCaptureDevice::StartStopCaptureDevice(DWORD devID, bool start, IMFMediaSource **ppSource,
	DWORD *countDevices, bool verbose, bool audioDevice)
{
	if (ppSource)
		*ppSource = NULL;

	UINT32 count = 0;

	if (countDevices)
		*countDevices = 0;

	IMFAttributes *pConfig = NULL;
	IMFActivate **ppDevices = NULL;

	// Create an attribute store to hold the search criteria.
	HRESULT hr = MFCreateAttributes(&pConfig, 1);

	// Request video capture devices.
	if (SUCCEEDED(hr))
	{
		auto devType = audioDevice ? MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID :
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID;

		hr = pConfig->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, devType);
	}
	else
		return -1;

	// Enumerate the devices,
	if (SUCCEEDED(hr))
	{
		hr = MFEnumDeviceSources(pConfig, &ppDevices, &count);
	}
	else
		return -1;

	if (count < 1)
		return -1;

	if (countDevices)
		*countDevices = count;

	// Create a media source for the first device in the list.
	if (SUCCEEDED(hr))
	{
		if (count > devID)
		{
			WCHAR *szFriendlyName = NULL;

			// Try to get the display name.
			UINT32 cchName;

			hr = ppDevices[devID]->GetAllocatedString(
				MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
				&szFriendlyName, &cchName);

			if (start)
			{
				if (SUCCEEDED(hr))
				{
					if (verbose)
					{
						wprintf(L"Activating aud/video device ID: %d, Name: ", devID);
						wprintf(szFriendlyName);
					}
				}
				else
					return -1;

				hr = ppDevices[devID]->ActivateObject(IID_PPV_ARGS(ppSource));
			}
			else
			{
				if (SUCCEEDED(hr))
				{
					if (verbose)
					{
						wprintf(L"Shutting down aud/video device ID: %d, Name: ", devID);
						wprintf(szFriendlyName);
					}
				}
				hr = ppDevices[devID]->ShutdownObject();
				hr = ppDevices[devID]->DetachObject();

			}

			CoTaskMemFree(szFriendlyName);
		}
		else
		{
			hr = MF_E_NOT_FOUND;
		}
	}
	else
		return -1;

	for (DWORD i = 0; i < count; i++)
	{
		ppDevices[i]->Release();
	}
	CoTaskMemFree(ppDevices);
	SafeRelease(&pConfig);
	return hr;
}

