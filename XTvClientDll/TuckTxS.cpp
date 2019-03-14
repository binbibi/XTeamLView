#include "stdafx.h"
#include "tucktxs.h"
#include "OutPutDebug.h"


#include <strsafe.h>
#include <comdef.h>
#include <Wbemidl.h>
#include "detours\include\detours.h"
#include "objbase.h"

#pragma  comment (lib, "detours/lib.X86/detours.lib")

#include <stdlib.h>
#include <time.h>
#include <Shlobj.h>
#include <strsafe.h>
#include <atlstr.h>
#include <random>  


typedef HRESULT(STDMETHODCALLTYPE *CWbemObjectGet)(
	IWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName,
	/* [in] */ long lFlags,
	/* [unique][in][out] */ VARIANT *pVal,
	/* [unique][in][out] */ CIMTYPE *pType,
	/* [unique][in][out] */ long *plFlavor);


CWbemObjectGet TrueCWbemObjectGet = NULL;


typedef LSTATUS (__stdcall *Kernel32RegQueryValueExWPtr)(
	HKEY    hKey,
	LPCWSTR lpValueName,
	LPDWORD lpReserved,
	LPDWORD lpType,
	LPBYTE  lpData,
	LPDWORD lpcbData
);

Kernel32RegQueryValueExWPtr TrueKernel32RegQueryValueExW = NULL;


typedef LSTATUS(__stdcall *KernelBaseRegQueryValueExWPtr)(
	HKEY    hKey,
	LPCWSTR lpValueName,
	LPDWORD lpReserved,
	LPDWORD lpType,
	LPBYTE  lpData,
	LPDWORD lpcbData
	);

KernelBaseRegQueryValueExWPtr TrueKernelBaseRegQueryValueExW = NULL;


typedef BOOL (WINAPI *GetVolumeInformationWPtr)(
	__in_opt  LPCWSTR lpRootPathName,
	__out_ecount_opt(nVolumeNameSize) LPWSTR lpVolumeNameBuffer,
	__in      DWORD nVolumeNameSize,
	__out_opt LPDWORD lpVolumeSerialNumber,
	__out_opt LPDWORD lpMaximumComponentLength,
	__out_opt LPDWORD lpFileSystemFlags,
	__out_ecount_opt(nFileSystemNameSize) LPWSTR lpFileSystemNameBuffer,
	__in      DWORD nFileSystemNameSize
);

GetVolumeInformationWPtr TrueGetVolumeInformationW = NULL;


typedef HANDLE(WINAPI *FindFirstFileWPtr)(
	__in  LPCWSTR lpFileName,
	__out LPWIN32_FIND_DATAW lpFindFileData
	);

FindFirstFileWPtr TrueFindFirstFileW = NULL;


#include <string>
using namespace std;


std::string GuidToString(const GUID &guid)
{
	char buf[64*2] = { 0 };
	sprintf_s(buf, sizeof(buf), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	return std::string(buf);
}

std::string s;
DWORD g_VolumeSerialNumber = 0;
CString g_strPath;


HRESULT STDMETHODCALLTYPE ProxyCWbemObjectGet(
	IWbemClassObject * This,
	/* [string][in] */ LPCWSTR wszName,
	/* [in] */ long lFlags,
	/* [unique][in][out] */ VARIANT *pVal,
	/* [unique][in][out] */ CIMTYPE *pType,
	/* [unique][in][out] */ long *plFlavor)
{
	
	if (!wszName || !pVal)
	{
		return TrueCWbemObjectGet(This, wszName, lFlags, pVal, pType, plFlavor);
	}
	
	if (!wcsicmp(wszName, L"UUID")) //CPUID
	{

		VariantInit(pVal);

		pVal->vt = VT_BSTR;
		
		
		MyAtlTraceA("[%s] UUID 是%s \n", __FUNCTION__, s.c_str());

		V_BSTR(pVal) = SysAllocString(CA2W(s.c_str()));
		return 1;
	}
		
	
	else
		return TrueCWbemObjectGet(This, wszName, lFlags, pVal, pType, plFlavor);
}


LSTATUS WINAPI ProxyKernel32RegQueryValueExW(
	HKEY    hKey,
	LPCWSTR lpValueName,
	LPDWORD lpReserved,
	LPDWORD lpType,
	LPBYTE  lpData,
	LPDWORD lpcbData
)
{
	if (lpValueName && (
		(   0 == wcsicmp(lpValueName, L"ClientIC")) 
		|| (0 == wcsicmp(lpValueName, L"ClientID"))
		|| (0 == wcsicmp(lpValueName, L"MIDInitiativeGUID"))
		|| (0 == wcsicmp(lpValueName, L"MIDAttractionGUID"))
		))
	{
		MyAtlTraceA("[%s] 就是要Fuck %s\n", __FUNCTION__, lpValueName);
		return ERROR_FILE_NOT_FOUND;
	}
	
	else
	{
		return TrueKernel32RegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	}
}


LSTATUS WINAPI ProxyKernelBaseRegQueryValueExW(
	HKEY    hKey,
	LPCWSTR lpValueName,
	LPDWORD lpReserved,
	LPDWORD lpType,
	LPBYTE  lpData,
	LPDWORD lpcbData
)
{
	if (lpValueName && (
		(0 == wcsicmp(lpValueName, L"ClientIC"))
		|| (0 == wcsicmp(lpValueName, L"ClientID"))
		|| (0 == wcsicmp(lpValueName, L"MIDInitiativeGUID"))
		|| (0 == wcsicmp(lpValueName, L"MIDAttractionGUID"))
		)
		)
	{
		MyAtlTraceA("[%s] 就是要Fuck %s\n", __FUNCTION__, lpValueName);
		return ERROR_FILE_NOT_FOUND;
	}

	else
	{
		return TrueKernelBaseRegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	}
}


BOOL WINAPI ProxyGetVolumeInformationW(
	__in_opt  LPCWSTR lpRootPathName,
	__out_ecount_opt(nVolumeNameSize) LPWSTR lpVolumeNameBuffer,
	__in      DWORD nVolumeNameSize,
	__out_opt LPDWORD lpVolumeSerialNumber,
	__out_opt LPDWORD lpMaximumComponentLength,
	__out_opt LPDWORD lpFileSystemFlags,
	__out_ecount_opt(nFileSystemNameSize) LPWSTR lpFileSystemNameBuffer,
	__in      DWORD nFileSystemNameSize
)
{
	// 先获取下
	BOOL bRet = TrueGetVolumeInformationW(lpRootPathName, lpVolumeNameBuffer, nVolumeNameSize, lpVolumeSerialNumber, lpMaximumComponentLength, lpFileSystemFlags, lpFileSystemNameBuffer, nFileSystemNameSize);

	if (bRet != 0 && lpRootPathName 
		&& (0 == wcsicmp(L"C:\\", lpRootPathName))
		)
	{
		if (lpVolumeSerialNumber)
		{
			*lpVolumeSerialNumber = g_VolumeSerialNumber;
			MyAtlTraceA("[%s] 就是要Fuck %p\n", __FUNCTION__, g_VolumeSerialNumber);
		}
		
	}

	return bRet;
}


HANDLE WINAPI ProxyFindFirstFileW(
	__in  LPCWSTR lpFileName,
	__out LPWIN32_FIND_DATAW lpFindFileData
)
{
	HANDLE h = TrueFindFirstFileW(lpFileName, lpFindFileData);
	
	if (h != INVALID_HANDLE_VALUE && !g_strPath.IsEmpty() && lpFileName)
	{
	    if (g_strPath.CompareNoCase(lpFileName) == 0)
		{
			MyAtlTraceW(L"[%s] 就是要Fuck %s\n", __FUNCTIONW__, g_strPath);
			if (lpFindFileData)
			{
				lpFindFileData->ftCreationTime.dwHighDateTime = g_VolumeSerialNumber;
				lpFindFileData->ftCreationTime.dwLowDateTime = g_VolumeSerialNumber;
			}
	    }
	}
	return h;
}


BOOL GetPEVersion(LPCWSTR path, DWORD *msver, DWORD *lsver)
{
	BOOL status = FALSE;
	PVOID info = NULL;
	DWORD handle = 0;
	VS_FIXEDFILEINFO* vsinfo = NULL;
	UINT vsinfolen = 0;
	DWORD infolen = GetFileVersionInfoSizeW(path, &handle);

	if (infolen)
	{
		info = malloc(infolen);
		if (info)
		{
			if (GetFileVersionInfoW(path, handle, infolen, info))
			{
				if (VerQueryValue(info, _T("\\"), (void**)&vsinfo, &vsinfolen))
				{
					if (msver)
					{
						*msver = vsinfo->dwFileVersionMS;
					}

					if (lsver)
					{
						*lsver = vsinfo->dwFileVersionLS;
					}

					status = TRUE;
				}
			}

			free(info);
		}
	}

	return status;
}


VOID TuckMsg::Start()
{  
	_StartImpl();
}


VOID TuckMsg::_StartImpl()
{	
	// 开始执行Hook
	StartHook();	
}


VOID TuckMsg::StartHook()
{
	LoadLibrary(L"fastprox.dll");
	LoadLibrary(L"kernel32.dll");
	LoadLibrary(L"kernelbase.dll");

	g_strPath.Empty();

	MyAtlTraceW(L"[%s]进来了\n", __FUNCTIONW__);

	/// uuid
	GUID guid;
	CoCreateGuid(&guid);
	s = GuidToString(guid);
	
	// 序列号
	random_device rd;
	mt19937 gen(rd());

	g_VolumeSerialNumber = gen(); // 打印随机数 0 到 RAND_MAX


	// 特殊路径
	// CSIDL_APPDATA
	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, 0x26, NULL, 0, szPath)))
	{
		g_strPath = szPath;
	}

	TrueCWbemObjectGet = (CWbemObjectGet)DetourFindFunction("fastprox.dll", "?Get@CWbemObject@@UAGJPBGJPAUtagVARIANT@@PAJ2@Z");
	TrueKernel32RegQueryValueExW = (Kernel32RegQueryValueExWPtr)DetourFindFunction("kernel32.dll", "RegQueryValueExW");
	TrueGetVolumeInformationW = (GetVolumeInformationWPtr)DetourFindFunction("kernel32.dll", "GetVolumeInformationW");
	TrueFindFirstFileW = (FindFirstFileWPtr)DetourFindFunction("kernel32.dll", "FindFirstFileW");
	TrueKernelBaseRegQueryValueExW = (KernelBaseRegQueryValueExWPtr)DetourFindFunction("kernelbase.dll", "RegQueryValueExW");
	
	// 比较标准的用法
	if (NO_ERROR == DetourTransactionBegin())
	{
		if (NO_ERROR == DetourUpdateThread(GetCurrentThread()))
		{
			if (NO_ERROR == DetourAttach(&(PVOID&)TrueKernelBaseRegQueryValueExW, (LPBYTE)ProxyKernelBaseRegQueryValueExW))
			{
				if (NO_ERROR == DetourTransactionCommit())
				{
					MyAtlTraceA("Hook KernelBase RegQueryValueExW");
				}
				else
				{
					DetourTransactionAbort();
				}
			}
			else
			{
				DetourTransactionCommit();
			}
		}
		else
		{
			DetourTransactionCommit();
		}
	}
	
	if (NO_ERROR == DetourTransactionBegin())
	{
		if (NO_ERROR == DetourUpdateThread(GetCurrentThread()))
		{
			if (NO_ERROR == DetourAttach(&(PVOID&)TrueCWbemObjectGet, (LPBYTE)ProxyCWbemObjectGet))
			{
				if (NO_ERROR == DetourTransactionCommit())
				{
					MyAtlTraceA("Hook ?Get@CWbemObject@@UAGJPBGJPAUtagVARIANT@@PAJ2@Z");
				}
			}
			else
			{
				DetourTransactionCommit();
			}
		}
	}

	if (NO_ERROR == DetourTransactionBegin())
	{
		if (NO_ERROR == DetourUpdateThread(GetCurrentThread()))
		{
			if (NO_ERROR == DetourAttach(&(PVOID&)TrueKernel32RegQueryValueExW, (LPBYTE)ProxyKernel32RegQueryValueExW))
			{
				if (NO_ERROR == DetourTransactionCommit())
				{
					MyAtlTraceA("Hook Kernel32 RegQueryValueExW");
				}
			}
			else
			{
				DetourTransactionCommit();
			}
		}
	}

	if (NO_ERROR == DetourTransactionBegin())
	{
		if (NO_ERROR == DetourUpdateThread(GetCurrentThread()))
		{
			if (NO_ERROR == DetourAttach(&(PVOID&)TrueGetVolumeInformationW, (LPBYTE)ProxyGetVolumeInformationW))
			{
				if (NO_ERROR == DetourTransactionCommit())
				{
					MyAtlTraceA("Hook GetVolumeInformationW");
				}
			}
			else
			{
				DetourTransactionCommit();
			}
		}
	}


	if (NO_ERROR == DetourTransactionBegin())
	{
		if (NO_ERROR == DetourUpdateThread(GetCurrentThread()))
		{
			if (NO_ERROR == DetourAttach(&(PVOID&)TrueFindFirstFileW, (LPBYTE)ProxyFindFirstFileW))
			{
				if (NO_ERROR == DetourTransactionCommit())
				{
					MyAtlTraceA("Hook FindFirstFileW");
				}
			}
			else
			{
				DetourTransactionCommit();
			}
		}
	}

}
