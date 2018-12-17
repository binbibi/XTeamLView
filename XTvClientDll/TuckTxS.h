#ifndef __DEF_TUCKTXS_MON__
#define __DEF_TUCKTXS_MON__

#include "windows.h"

#include <vector> 
using namespace std;
#include <atlstr.h>

class TuckMsg
{
public:	
	VOID Start();
   

public:
	static TuckMsg* Instance()
	{
		static TuckMsg s_Instance;
		return &s_Instance;
	}
	
private:
	TuckMsg::TuckMsg() :
		// 原始模块句柄
		m_hProcess(NULL),
		m_exe_msver(0),
		m_exe_lsver(0),
		m_Init(0),
		m_exe_size(0),
		m_FirstHitTime(0)
	{
		;
	}


	VOID _StartImpl();
	VOID StartHook();
    


private:
	HMODULE m_hProcess;        // 注入的模块的地址
	DWORD m_exe_msver;         // 主版本号
	DWORD m_exe_lsver;         // 次版本号
	BOOL  m_Init;              // HOOK环境初始化Ok
	DWORD m_exe_size;          // 注入的进程的文件大小
	CString m_cs_filename;
	DWORD m_FirstHitTime;
};






#endif