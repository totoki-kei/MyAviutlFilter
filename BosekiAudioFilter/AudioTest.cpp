#include "stdafx.h"
#include "AudioPosition.h"

namespace AudioTest {

	int     track_count = 1;
	LPCTSTR track_names[] = { _T("�ʑ�") };
	int     track_default_values[] = { 0 };
	int     track_min_values[] = { -36000 };
	int     track_max_values[] = { 36000 };


	BOOL FilterProc(FILTER *fp, FILTER_PROC_INFO *fpip) {


		return TRUE;
	}



	FILTER_DLL FilterDeclaration = {
		// flag
		FILTER_FLAG_PRIORITY_HIGHEST | FILTER_FLAG_AUDIO_FILTER | FILTER_FLAG_EX_INFORMATION,
		// x, y
		0,0,
		// name
		_T("Boseki�����t�B���^(���E)"),
		// track_n, track_name[], track_default[], track_start[], track_end[]
		track_count,track_names,track_default_values,track_min_values,track_max_values,
		// check_n, check_name[], check_default[]
		0,NULL,NULL,
		// func_proc
		FilterProc,
		// func_init
		NULL,
		// func_exit
		NULL,
		// func_update
		NULL,
		// func_WndProc
		NULL,
		// track, check
		NULL,NULL,
		// ex_data_ptr
		NULL,
		// ex_data_size
		NULL,
		// information
		_T("Boseki�����t�B���^(���E) version 1.0 by Totoki Kei"),
		// func_save_start
		NULL,
		// func_save_end
		NULL,

		// ���������̃����o��0�����l
	};

}