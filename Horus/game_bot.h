#include <Windows.h>
#include "eji_vision.h"

namespace game_bot 
{
	struct handle_data
	{
		unsigned long pid;
		HWND hWnd;
	};

	inline BOOL CALLBACK enum_windows_callback(HWND hWnd, LPARAM lparam)
	{
		auto& data = *reinterpret_cast<handle_data*>(lparam);

		unsigned long pid = 0;
		GetWindowThreadProcessId(hWnd, &pid);

		if (data.pid != pid)
		{
			return true;
		}

		data.hWnd = hWnd;
		return false;
	}

	inline HWND find_window(unsigned long pid)
	{
		handle_data data{};

		data.pid = pid;
		data.hWnd = nullptr;
		EnumWindows(enum_windows_callback, reinterpret_cast<LPARAM>(&data));

		return data.hWnd;
	}

	inline void get_all_windows_from_pid(unsigned long dst_pid, std::vector <HWND>& hWnds)
	{
		HWND cur_hWnd = nullptr;
		do
		{
			cur_hWnd = FindWindowEx(nullptr, cur_hWnd, nullptr, nullptr);
			unsigned long check_pid = 0;
			GetWindowThreadProcessId(cur_hWnd, &check_pid);
			if (check_pid == dst_pid)
			{
				hWnds.push_back(cur_hWnd);
			}

		} while (cur_hWnd != nullptr);
	}
}