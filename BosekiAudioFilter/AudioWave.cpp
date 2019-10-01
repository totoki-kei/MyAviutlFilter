#include "stdafx.h"
#include "AudioWave.h"

#include "Helper.h"

namespace AudioWave {

	LPCTSTR track_names[] = { _T("���g��"), _T("����"), _T("�g�`���[�h") };
	int     track_default_values[] = { 440, 10000, 0 };
	int     track_min_values[] = { 1, -10000, 0 };
	int     track_max_values[] = { 44000, 10000, 3 };
	int     track_count = GetTrackCount(track_names, track_default_values, track_min_values, track_max_values);

	LPCTSTR check_names[] = { _T("�I�[�␳") };
	int check_default_values[] = { 1 };
	int check_count = GetCheckCount(check_names, check_default_values);

	BOOL FilterProc(FILTER *fp, FILTER_PROC_INFO *fpip) {

		// �T���v�����O���[�g���o
		FILE_INFO file_info;
		if (!fp->exfunc->get_file_info(fpip->editp, &file_info)) return FALSE;
		int sample_rate = file_info.audio_rate;

		// 1�T���v���ӂ�̎���
		double time_per_sample = 1.0 / sample_rate;

		// �t���[���̐擪�����b�ڂȂ̂�
		double frame_time = fpip->frame / (double)file_info.video_rate;

		// ���̐؂�ڂ����R�ɂ��邽�߂̑Ή�
		// ���݂̃t���[�����ŏI�t���[���ŁA���݂�1���������������邾���̎��ԓI�]�T�������ꍇ��
		// �[�����ɍ����ւ���
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
			case 0: // �����g
				val = sin(t * 3.141592653589793 * 2) * volume;
				break;
			case 1: // ��`�g
				val = (t < 0.5 ? 1.0 : -1.0) * volume;
				break;
			case 2: // �O�p�g
				val = (t < 0.25 ? t * 4 : t < 0.75 ? -4 * t + 2 : 4 * t - 4) * volume;
				break;
			case 3: // �̂�����g
				val = (2.0 * t - 1) * volume;
				break;
			}

			// �S�`���l���ɉ��Z
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
		_T("Boseki�����t�B���^(�g�`)"),
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
		_T("Boseki�����t�B���^(�g�`) version 1.0 by Totoki Kei"),
		// func_save_start
		NULL,
		// func_save_end
		NULL,

		// ���������̃����o��0�����l
	};

}