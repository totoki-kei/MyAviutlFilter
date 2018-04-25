#include "stdafx.h"
#include "AudioPosition.h"

namespace AudioPosition {

	int     track_count = 2;
	LPCTSTR track_names[] = { _T("レベル"), _T("左右") };
	int     track_default_values[] = { 256, 0 };
	int     track_min_values[] = { 0, -100 };
	int     track_max_values[] = { 256, 100 };


	BOOL FilterProc(FILTER *fp, FILTER_PROC_INFO *fpip) {

		// 音量を変更する
		for (int i = 0; i < fpip->audio_n * fpip->audio_ch; i++) {
			fpip->audiop[i] = (short)(fpip->audiop[i] * fp->track[0] / 256);
		}

		// 左右割合を変更する
		for (int i = 0; i < fpip->audio_n; i++) {
			// 仮に、チャンネル0を左、チャンネル1を右として処理を記述する
			auto left = fpip->audiop[2 * i];
			auto right = fpip->audiop[2 * i + 1];

			auto value = fp->track[1];

			double ll = value < 0 ? 1.0 : (100 - value) / 100.0;
			double lr = value < 0 ? (-value) / 100.0 : 0.0;
			double rl = value < 0 ? 0.0 : (value) / 100.0;
			double rr = value < 0 ? (100 + value) / 100.0 : 1.0;

			fpip->audiop[2 * i] = static_cast<short>(left * ll + right * lr);
			fpip->audiop[2 * i + 1] = static_cast<short>(left * rl + right * rr);
		}

		return TRUE;
	}



	FILTER_DLL FilterDeclaration = {
		// flag
		FILTER_FLAG_PRIORITY_HIGHEST | FILTER_FLAG_AUDIO_FILTER | FILTER_FLAG_EX_INFORMATION,
		// x, y
		0,0,
		// name
		_T("音声フィルタA"),
		// track_n, track_name[], track_default[], track_start[], track_end[]
		track_count,track_names,track_default_values,track_min_values,track_max_values,
		// check_n, check_name[], check_default[]
		NULL,NULL,NULL,
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
		_T("音声フィルタ version 0.01 by Totoki Kei"),
		// func_save_start
		NULL,
		// func_save_end
		NULL,

		// ここから先のメンバは0初期値
	};

}