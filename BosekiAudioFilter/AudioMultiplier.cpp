#include "stdafx.h"
#include "AudioMultiplier.h"

#include "Helper.h"

namespace AudioMultiplier {
	constexpr double MAX_VALUE = 1000.0;

	int     track_count = 4; // トラックバーの数
	LPCTSTR track_names[] = { _T("左-分子"), _T("左-分母"), _T("右-分子"), _T("右-分母") };
	int     track_default_values[] = { 1, 1, 1, 1 };
	int     track_min_values[] = { (int)-MAX_VALUE, 1             , (int)-MAX_VALUE, 1             , };
	int     track_max_values[] = { (int)MAX_VALUE , (int)MAX_VALUE, (int)MAX_VALUE , (int)MAX_VALUE, };

	static BOOL FilterProc(FILTER *fp, FILTER_PROC_INFO *fpip)
	{
		// 左右割合を変更する
		for (int i = 0; i < fpip->audio_n; i++) {
			// 仮に、チャンネル0を左、チャンネル1を右として処理を記述する

			if (fpip->audio_ch == 2) {
				// left
				auto& left = fpip->audiop[2 * i];
				{
					left = clamp<short>(left * (double)fp->track[0] / (double)(fp->track[1]));
				}
				// right
				auto& right = fpip->audiop[2 * i + 1];
				{
					right = clamp<short>(right * (double)fp->track[2] / (double)(fp->track[3]));
				}
			}
			else {
				// チャンネル数が2でない : あり得るのか分からないが、この場合左用の値で全チャネルを加工
				for (int ch = 0; ch < fpip->audio_ch; ch++) {
					auto& val = fpip->audiop[fpip->audio_ch * i + ch];
					val = (short)(val * (double)fp->track[0] / (double)(fp->track[1]));
				}
			}

		}

		return TRUE;
	}

	FILTER_DLL FilterDeclaration = {
		// flag
		FILTER_FLAG_PRIORITY_HIGHEST | FILTER_FLAG_AUDIO_FILTER | FILTER_FLAG_EX_INFORMATION,
		// x, y
		0,0,
		// name
		_T("Boseki音声フィルタ(乗算)"),
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
		_T("Boseki音声フィルタ(乗算) version 1.0 by Totoki Kei"),
		// func_save_start
		NULL,
		// func_save_end
		NULL,

		// ここから先のメンバは0初期値
	};
}
