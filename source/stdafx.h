// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

#include <psapi.h>
#include <shellapi.h>
#include <vector>

// Media Foundation

#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "Mf.lib")

// reference additional headers your program requires here
#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")
#pragma once

template <class T> void SafeRelease(T **ppT)
{

	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}