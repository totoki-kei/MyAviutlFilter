// FilterDllCheck.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include <Windows.h>
#include <Shlwapi.h>
#include <iostream>
#include <string>

#include <AviUtl/filter.h>
#include "FilterDllCheck.h"


namespace Fn {
	typedef FILTER_DLL *  __stdcall GetFilterTable(void);
	typedef FILTER_DLL ** __stdcall GetFilterTableList(void);
}

#define PRINT_MEMBER(ptr, member, format) _tprintf(_T("%hs: " format "\n"), #member, ptr -> member);

void PrintFilterInfo(FILTER_DLL* filter) {
	_tprintf(_T("*** Filter '%s'\n"), filter->name);
	
	PRINT_MEMBER(filter, flag, "0x%x");
	PRINT_MEMBER(filter, x, "%d");
	PRINT_MEMBER(filter, y, "%d");
	PRINT_MEMBER(filter, track_n, "%d");
	for (int i = 0; i < filter->track_n; i++) {
		_tprintf(_T("track[%d]: %s (%d, %d, %d)\n"), i, filter->track_name[i], filter->track_s[i], filter->track_default[i], filter->track_e[i]);
	}
	PRINT_MEMBER(filter, check_n, "%d");
	for (int i = 0; i < filter->check_n; i++) {
		_tprintf(_T("check[%d]: %s (%d)\n"), i, filter->check_name[i], filter->check_default[i]);
	}
	PRINT_MEMBER(filter, func_proc, "0x%p");
	PRINT_MEMBER(filter, func_init, "0x%p");
	PRINT_MEMBER(filter, func_exit, "0x%p");
	PRINT_MEMBER(filter, func_update, "0x%p");
	PRINT_MEMBER(filter, func_WndProc, "0x%p");

	PRINT_MEMBER(filter, track, "0x%p");
	PRINT_MEMBER(filter, check, "0x%p");
	PRINT_MEMBER(filter, ex_data_ptr, "0x%p");
	PRINT_MEMBER(filter, ex_data_size, "%d");
	PRINT_MEMBER(filter, information, "%s");

	PRINT_MEMBER(filter, func_save_start, "0x%p");
	PRINT_MEMBER(filter, func_save_end, "0x%p");

	PRINT_MEMBER(filter, exfunc, "0x%p");
	PRINT_MEMBER(filter, hwnd, "0x%p");
	PRINT_MEMBER(filter, dll_hinst, "0x%p");
	PRINT_MEMBER(filter, ex_data_def, "0x%p");

	PRINT_MEMBER(filter, func_is_saveframe, "0x%p");
	PRINT_MEMBER(filter, func_project_load, "0x%p");
	PRINT_MEMBER(filter, func_project_save, "0x%p");
	PRINT_MEMBER(filter, func_modify_title, "0x%p");

	PRINT_MEMBER(filter, dll_path, "%s");
	PRINT_MEMBER(filter, reserve[0], "%d");
	PRINT_MEMBER(filter, reserve[1], "%d");

	_tprintf(_T("*** /Filter '%s'\n"), filter->name);

}

void PrintFilterInfo(LPCTSTR file) {
	HMODULE dll = LoadLibrary(file);
	if (!dll) {
		_tprintf(_T("%s('%s') : error 0x%08x\n"), _T("LoadLibrary"), file, ::GetLastError());
		return;
	}

	Fn::GetFilterTable* gft = (Fn::GetFilterTable*)GetProcAddress(dll, "GetFilterTable");
	Fn::GetFilterTableList* gftl = (Fn::GetFilterTableList*)GetProcAddress(dll, "GetFilterTableList");

	if (gft) {
		PrintFilterInfo(gft());
	}

	if (gftl) {
		for (auto filter_list = gftl(); filter_list && *filter_list; filter_list++) {
			PrintFilterInfo(*filter_list);
		}
	}
}

int main(int argc, TCHAR* argv[])
{
	if (argc > 1) {
		for (int arg_index = 1; arg_index < argc; arg_index++) {
			auto arg = argv[arg_index];

			PrintFilterInfo(arg);
		}
	}
	else {
		puts("input dll path: ");
		std::string line;
		std::getline(std::cin, line);
		if (!line.empty()) {
			PrintFilterInfo(line.c_str());
		}
		system("pause");
	}


    return 0;
}

