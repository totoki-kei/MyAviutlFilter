#include "stdafx.h"
#include "AudioTest.h"

#include "Helper.h"

#include <sstream>


namespace AudioTest {

	int     track_count = 1;
	LPCTSTR track_names[] = { _T("トラック1") };
	int     track_default_values[] = { 0 };
	int     track_min_values[] = { -36000 };
	int     track_max_values[] = { 36000 };



	BOOL FilterProc(FILTER *fp, FILTER_PROC_INFO *fpip) {
		// TODO: 拡張編集呼び出しかそうで無いかを区別するコードを追加する

		//CMemref memref;
		//memref.Init(fp);

		//auto processing_filter = memref.Exedit_ScriptProcessingFilter();

		//auto filters = memref.Exedit_StaticFilterTable();
		//for (auto filter = filters; (*filter); filter++) {
		//	OutputDebugStringA((*filter)->name);
		//	OutputDebugStringA("\n");
		//}


		auto effect_index = GetEffectIndexInExEditObject(fpip);
		auto obj = GetExEditObject(fpip);

		std::stringstream ss;

		ss << __FUNCTION__;
		ss << "  " << "scene: " << obj->scene_set; // シーン番号
		ss << ", " << "layer: " << obj->layer_disp; // レイヤー番号
		ss << ", " << "addr(obj): " << obj; // 拡張編集オブジェクトのアドレス
		ss << ", " << "filter_index: " << effect_index << "/" << obj->GetFilterNum(); // オブジェクト中のフィルタ番号(0の場合本体)と合計フィルタ数
		ss << std::endl;

		OutputDebugStringA(ss.str().c_str());

		if (effect_index == 0) {
			sprintf_s(obj->dispname, "%p(%d, %d)", obj, obj->scene_set, obj->layer_disp);
		}

		//char str[256] = {};
		//sprintf_s(str, "layer=%d,obj=%p,effect=%d frame=(%d/%d) filter_num=%d\n", obj->layer_disp, obj, effect_index, fpip->frame, fpip->frame_n, obj->GetFilterNum());
		//OutputDebugStringA(str);

		

		return FALSE;
	}

	BOOL FilterInit(FILTER *fp) {
		return TRUE;
	}

	BOOL FilterUpdate(FILTER* fp, int status) {
		OutputDebugString((__FUNCTION__ " status: " + std::to_string(status) + "\n").c_str());
		return TRUE;
	}


	FILTER_DLL FilterDeclaration = {
		// flag
		FILTER_FLAG_PRIORITY_HIGHEST | FILTER_FLAG_AUDIO_FILTER | FILTER_FLAG_EX_INFORMATION,
		// x, y
		0,0,
		// name
		_T("Boseki音声フィルタ(テスト)"),
		// track_n, track_name[], track_default[], track_start[], track_end[]
		track_count,track_names,track_default_values,track_min_values,track_max_values,
		// check_n, check_name[], check_default[]
		0,NULL,NULL,
		// func_proc
		FilterProc,
		// func_init
		FilterInit,
		// func_exit
		NULL,
		// func_update
		FilterUpdate,
		// func_WndProc
		NULL,
		// track, check
		NULL,NULL,
		// ex_data_ptr
		NULL,
		// ex_data_size
		0,
		// information
		_T("Boseki音声フィルタ(テスト) version 1.0 by Totoki Kei"),
		// func_save_start
		NULL,
		// func_save_end
		NULL,

		// ここから先のメンバは0初期値
	};

}