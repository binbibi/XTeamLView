// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>
#include <atlstr.h>

#include <shellapi.h>
#include <shlwapi.h>
#include <strsafe.h>


#pragma comment(lib, "shlwapi.lib")
//#pragma comment(lib, "detours.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "strsafe.lib")



#pragma warning(disable: 4018)
#pragma warning(disable: 4258)
#pragma warning(disable: 4995)
#pragma warning(disable: 4996)
#pragma warning(disable: 4102)




// TODO: reference additional headers your program requires here
