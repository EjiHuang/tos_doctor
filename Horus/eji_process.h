#pragma once

#include <iostream>
#include <Windows.h>
#include <WtsApi32.h>
#include <vector>
#include <atlstr.h>

#pragma comment(lib, "WtsApi32.lib")

struct _proc_info
{
	int pid;
	std::wstring process_name;
};

class eji_process
{
public:
	void get_process_list();

	std::vector<_proc_info> proc_infoes;

private:
	bool adjust_process_privileges();
};

/// <summary>
/// ��ȡ�����б�
/// </summary>
inline void eji_process::get_process_list()
{
	WTS_PROCESS_INFO* pWPIs = nullptr;
	unsigned long proc_count = 0;
	if (WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pWPIs, &proc_count))
	{
		// ������м������Ĺ���
		for (size_t i = 0; i < proc_count; i++)
		{
			proc_infoes.push_back(_proc_info());
			proc_infoes[i].pid = pWPIs[i].ProcessId;
			// proc_infoes[i].process_name = CW2A(pWPIs[i].pProcessName);
			proc_infoes[i].process_name = pWPIs[i].pProcessName;
		}
	}

	// �ͷ���Դ
	if (pWPIs)
	{
		WTSFreeMemory(pWPIs);
		pWPIs = nullptr;
	}
}

/// <summary>
/// ��������Ȩ��
/// </summary>
/// <returns></returns>
inline bool eji_process::adjust_process_privileges()
{
	HANDLE proc_token;
	LUID luid;
	TOKEN_PRIVILEGES tp;
	bool ret_value;
	unsigned long err;

	ret_value = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &proc_token);
	if (!ret_value)
	{
		err = GetLastError();
		std::cerr << "OpenProcessToken failed, err = " << err << ", err";
		goto exit;
	}

	ret_value = LookupPrivilegeValue(nullptr, SE_LOAD_DRIVER_NAME, &luid);
	if (!ret_value)
	{
		err = GetLastError();
		std::cerr << "OpenProcessToken failed, err = " << err << ", err";
		goto exit1;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	//
	// ��ʹ��������Ȩ��AdjustTokenPrivilegesҲ��ɹ���
	// ����������£�GetLastError����ERROR_NOT_ALL_ASSIGNED��
	//
	// ��ˣ������ڳɹ���ʧ������¶����GetLastError��
	//

	(void)AdjustTokenPrivileges(proc_token, false, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)nullptr, (PDWORD)nullptr);
	err = GetLastError();

	if (ERROR_SUCCESS != err)
	{
		ret_value = false;
		std::cerr << "OpenProcessToken failed, err = " << err << ", err";
		goto exit1;
	}

exit1:
	CloseHandle(proc_token);
exit:
	return ret_value;
}