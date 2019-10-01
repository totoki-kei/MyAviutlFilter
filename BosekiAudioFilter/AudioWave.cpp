#include "stdafx.h"
#include "AudioWave.h"

#include "Helper.h"

namespace AudioWave {

	LPCTSTR track_names[] = { _T("周波数"), _T("音量"), _T("波形モード") };
	int     track_default_values[] = { 440, 10000, 0 };
	int     track_min_values[] = { 1, -10000, 0 };
	int     track_max_values[] = { 44000, 10000, 3 };
	int     track_count = GetTrackCount(track_names, track_default_values, track_min_values, track_max_values);

	LPCTSTR check_names[] = { _T("終端補正") };
	int check_default_values[] = { 1 };
	int check_count = GetCheckCount(check_names, check_default_values);

	BOOL FilterProc(FILTER *fp, FILTER_PROC_INFO *fpip) {

		// サンプリングレート導出
		FILE_INFO file_info;
		if (!fp->exfunc->get_file_info(fpip->editp, &file_info)) return FALSE;
		int sample_rate = file_info.audio_rate;

		// 1サンプル辺りの時間
		double time_per_sample = 1.0 / sample_rate;

		// フレームの先頭が何秒目なのか
		double frame_time = fpip->frame / (double)file_info.video_rate;

		// 音の切れ目を自然にするための対応
		// 現在のフレームが最終フレームで、現在の1周期を処理しきるだけの時間的余裕が無い場合は
		// ゼロ音に差し替える
		double frame_stop = std::numeric_limits<double>::max();
		if (fp->check[0] && fpip->frame == fpip->frame_n - 1) {
			auto frame_end_time = frame_time + 1.0 / file_info.video_rate;
			frame_stop = frame_end_time - fmod(frame_end_time, (1.0 / fp->track[0]));
		}

		double time = frame_time;
		for (int i = 0; i < fpip->audio_n; i++) {
			if (time >= frame_stop) break;

			auto t = fmod(time * fp->track[0], 1);
			auto volume = (fp->track[1] * 32767.0 / 10000.0);

			double val = 0;
			switch (fp->track[2]) {
			case 0: // 正弦波
				val = sin(t * 3.141592653589793 * 2) * volume;
				break;
			case 1: // 矩形波
				val = (t < 0.5 ? 1.0 : -1.0) * volume;
				break;
			case 2: // 三角波
				val = (t < 0.25 ? t * 4 : t < 0.75 ? -4 * t + 2 : 4 * t - 4) * volume;
				break;
			case 3: // のこぎり波
				val = (2.0 * t - 1) * volume;
				break;
			}

			// 全チャネルに加算
			for (int ch = 0; ch < fpip->audio_ch; ch++) {
				fpip->audiop[fpip->audio_ch * i + ch] = clamp<short>(fpip->audiop[fpip->audio_ch * i + ch] + val);
			}

			time += time_per_sample;
		}


		return TRUE;
	}



	FILTER_DLL FilterDeclaration = {
		// flag
		FILTER_FLAG_PRIORITY_HIGHEST | FILTER_FLAG_AUDIO_FILTER | FILTER_FLAG_EX_INFORMATION,
		// x, y
		0,0,
		// name
		_T("Boseki音声フィルタ(波形)"),
		// track_n, track_name[], track_default[], track_start[], track_end[]
		track_count,track_names,track_default_values,track_min_values,track_max_values,
		// check_n, check_name[], check_default[]
		check_count,check_names,check_default_values,
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
		_T("Boseki音声フィルタ(波形) version 1.0 by Totoki Kei"),
		// func_save_start
		NULL,
		// func_save_end
		NULL,

		// ここから先のメンバは0初期値
	};

}