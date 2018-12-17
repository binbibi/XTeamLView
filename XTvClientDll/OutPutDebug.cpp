#include "stdafx.h"
#include "outputdebug.h"
#include <strsafe.h>



void _cdecl MyAtlTraceW(LPCWSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	WCHAR szBuffer[0x500];
	WCHAR szOutText[0x500 + 20];

	HRESULT hr = StringCbVPrintfW(szBuffer, sizeof(szBuffer), lpszFormat, args);

	StringCbCopyW(szOutText, sizeof(szOutText), _MYDEBUGFLAG_W);

	StringCbCatW(szOutText, sizeof(szOutText), szBuffer);

	OutputDebugStringW(szOutText);
	va_end(args);
}

void _cdecl MyAtlTraceA(LPCSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	CHAR szBuffer[0x500];
	CHAR szOutText[0x500 + 20];

	HRESULT hr = StringCbVPrintfA(szBuffer, sizeof(szBuffer), lpszFormat, args);
	ATLASSERT(SUCCEEDED(hr)); 

	StringCbCopyA(szOutText, sizeof(szOutText), _MYDEBUGFLAG_A);

	StringCbCatA(szOutText, sizeof(szOutText), szBuffer);

	OutputDebugStringA(szOutText);
	va_end(args);
}



