// XTvClient.cpp : 定义控制台应用程序的入口点。
//



// Moni.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>

#include <Shlwapi.h>

#pragma  comment (lib, "Shlwapi.LIB")


#include "objbase.h"



typedef LPVOID (WINAPI *VirtualAllocPtr)(
	__in_opt LPVOID lpAddress,
	__in     SIZE_T dwSize,
	__in     DWORD flAllocationType,
	__in     DWORD flProtect
);


VirtualAllocPtr TRUEVirtualAlloc = NULL;

BOOL StopService(PWSTR pszServiceName)
{
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS ssSvcStatus = {};
	BOOL bStop = FALSE;

	// Open the local default service control manager database 
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL)
	{
		wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
		goto Cleanup;
	}

	// Open the service with delete, stop, and query status permissions 
	schService = OpenService(schSCManager, pszServiceName, SERVICE_STOP |
		SERVICE_QUERY_STATUS | DELETE);
	if (schService == NULL)
	{
		wprintf(L"OpenService failed w/err 0x%08lx\n", GetLastError());
		goto Cleanup;
	}

	// Try to stop the service 
	if (ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus))
	{
		wprintf(L"Stopping %s.", pszServiceName);
		Sleep(500);

		while (QueryServiceStatus(schService, &ssSvcStatus))
		{
			if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING)
			{
				wprintf(L".");
				Sleep(500);
			}
			else
				break;
		}

		if (ssSvcStatus.dwCurrentState == SERVICE_STOPPED)
		{
			wprintf(L"\n%s is stopped.\n", pszServiceName);
			bStop = TRUE;
		}
		else
		{
			wprintf(L"\n%s failed to stop.\n", pszServiceName);
		}
	}

	wprintf(L"%s is removed.\n", pszServiceName);

Cleanup:
	// Centralized cleanup for all allocated resources. 
	if (schSCManager)
	{
		CloseServiceHandle(schSCManager);
		schSCManager = NULL;
	}
	if (schService)
	{
		CloseServiceHandle(schService);
		schService = NULL;
	}
	return bStop;
}


int EnableDebugPriv(const wchar_t* name)
{
	HANDLE hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken)) {
		printf("OpenProcessToken Error\n");
		return -8;
	}

	LUID luid;
	if (!LookupPrivilegeValue(NULL, name, &luid)) {
		printf("LookupPrivilegeValue Error\n");
		return -9;
	}

	TOKEN_PRIVILEGES tokenPri;
	tokenPri.PrivilegeCount = 1;
	tokenPri.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tokenPri.Privileges[0].Luid = luid;

	if (!AdjustTokenPrivileges(hToken, 0, &tokenPri, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
		printf("AdjustTokenPrivileges Error!\n");
		return -10;
	}

	return 0;
}


void StartApp(DWORD *pid, DWORD *tid, LPTSTR app)
{
	if (pid == NULL || tid == NULL || app == NULL) {
		return;
	}

	STARTUPINFO sinfo;
	PROCESS_INFORMATION pinfo;
	ZeroMemory(&sinfo, sizeof(sinfo));

	ZeroMemory(&pinfo, sizeof(pinfo));

	CreateProcess(NULL, app, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &sinfo, &pinfo);
	*pid = pinfo.dwProcessId;
	*tid = pinfo.dwThreadId;

	printf("Remote App Process ID:\t%d\n", *pid);
	printf("Remote App Thread  ID:\t%d\n", *tid);
}


int InjectDLL(DWORD dwRemoteProcessId, const char* dll)
{
	HANDLE hRemoteProcess;
	if ((hRemoteProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwRemoteProcessId)) == NULL) {
		printf("Error Code:%d\n", GetLastError());
		return -2;
	}

	LPVOID pRemoteDllPath = VirtualAllocEx(hRemoteProcess, NULL, strlen(dll) + 1, MEM_COMMIT, PAGE_READWRITE);
	if (pRemoteDllPath == NULL) {
		printf("VirtualAllocEx Error\n");
		return -3;
	}

	SIZE_T Size;
	if (WriteProcessMemory(hRemoteProcess, pRemoteDllPath, dll, strlen(dll) + 1, &Size) == 0)
	{
		printf("WriteProcessMemory Error\n");
		return -4;
	}

	LPTHREAD_START_ROUTINE pLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "LoadLibraryA");
	if (pLoadLibrary == NULL)
	{
		printf("GetProcAddress Error\n");
		return -5;
	}

	DWORD dwThreadId;
	HANDLE hThread;
	if ((hThread = CreateRemoteThread(hRemoteProcess, NULL, 0, pLoadLibrary, pRemoteDllPath, 0, &dwThreadId)) == NULL) {
		printf("CreateRemoteThread Error\n");
		return -6;
	}
	else {
		WaitForSingleObject(hThread, INFINITE);
		printf("Remote DLL Thread ID:\t%d\n", dwThreadId);
		printf("Inject is done.\n");
	}

	if (VirtualFreeEx(hRemoteProcess, pRemoteDllPath, 0, MEM_RELEASE) == 0) {
		printf("VitualFreeEx Error\n");
		return -7;
	}

	if (hThread != NULL) CloseHandle(hThread);
	if (hRemoteProcess != NULL) CloseHandle(hRemoteProcess);
	return 0;
}


void ResumeRemoteThread(DWORD tid)
{
	printf("Resume Remote Thread: %d\n", tid);
	HANDLE thread_handle = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);
	ResumeThread(thread_handle);
	CloseHandle(thread_handle);
}


void CloseTeamViewerService()
{
	StopService(L"TeamViewer");
}


int main(void)
{
	const char monitor[] = "XtvClientdll.dll";
	const WCHAR exepath[] = L"TeamViewer.exe";
	DWORD dwRemoteProcessId, dwRemoteThreadId;

	CloseTeamViewerService();

	CHAR szCurrentPathDll[MAX_PATH];
	GetModuleFileNameA(NULL, szCurrentPathDll, MAX_PATH);
	PathRemoveFileSpecA(szCurrentPathDll);
	PathAppendA(szCurrentPathDll, monitor);

	WCHAR szCurrentPathExe[MAX_PATH];
	GetModuleFileName(NULL, szCurrentPathExe, MAX_PATH);
	PathRemoveFileSpec(szCurrentPathExe);
	PathAppend(szCurrentPathExe, exepath);

	printf("Current Inject PID: %d\n", GetCurrentProcessId());

	EnableDebugPriv(SE_DEBUG_NAME);
	StartApp(&dwRemoteProcessId, &dwRemoteThreadId, szCurrentPathExe);
	InjectDLL(dwRemoteProcessId, szCurrentPathDll);
	ResumeRemoteThread(dwRemoteThreadId);

	return 0;
}


