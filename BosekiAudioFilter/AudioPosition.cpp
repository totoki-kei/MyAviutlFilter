#include "stdafx.h"
#include "AudioPosition.h"

#include "Helper.h"

namespace AudioPosition {

	int     track_count = 1;
	LPCTSTR track_names[] = { _T("左右") };
	int     track_default_values[] = { 0 };
	int     track_min_values[] = { -10000 };
	int     track_max_values[] = {  10000 };


	BOOL FilterProc(FILTER *fp, FILTER_PROC_INFO *fpip) {

		// 左右割合を変更する
		for (int i = 0; i < fpip->audio_n; i++) {
			// 仮に、チャンネル0を左、チャンネル1を右として処理を記述する
			auto left = fpip->audiop[2 * i];
			auto right = fpip->audiop[2 * i + 1];

			auto balance = fp->track[0] / 10000.0;

			double ll = balance < 0 ? 1.0           : 1.0 - balance;
			double lr = balance < 0 ? 0.0 - balance : 0.0          ;
			double rl = balance < 0 ? 0.0           : 0.0 + balance;
			double rr = balance < 0 ? 1.0 + balance : 1.0          ;

			fpip->audiop[2 * i] =     clamp<short>(left * ll + right * lr);
			fpip->audiop[2 * i + 1] = clamp<short>(left * rl + right * rr);
		}

		return TRUE;
	}



	FILTER_DLL FilterDeclaration = {
		// flag
		FILTER_FLAG_PRIORITY_HIGHEST | FILTER_FLAG_AUDIO_FILTER | FILTER_FLAG_EX_INFORMATION,
		// x, y
		0,0,
		// name
		_T("Boseki音声フィルタ(左右)"),
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
		_T("Boseki音声フィルタ(左右) version 1.0 by Totoki Kei"),
		// func_save_start
		NULL,
		// func_save_end
		NULL,

		// ここから先のメンバは0初期値
	};

}