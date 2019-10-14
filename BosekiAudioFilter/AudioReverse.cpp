#include "stdafx.h"
#include "AudioTest.h"

#include "Helper.h"

#include "CacheControl.h"
#include "MemMap.h"

#include <unordered_map>
#include <sstream>


namespace AudioReverse {
	//LPCTSTR track_names[0] = { };
	//int     track_default_values[0] = { };
	//int     track_min_values[0] = { };
	//int     track_max_values[0] = { };
	//int     track_count = 0;

	//LPCTSTR check_names[0] = { };
	//int     check_default_values[0] = { };
	//int     check_count = 0;

	struct CacheKey {
		int scene_id;
		int layer_num;
		uintptr_t obj_addr;
		int effect_index;
		int frame_no;

		friend bool operator ==(const CacheKey& left, const CacheKey& right) {
			return left.scene_id == right.scene_id
				&& left.layer_num == right.layer_num
				&& left.obj_addr == right.obj_addr
				&& left.effect_index == right.effect_index
				&& left.frame_no == right.frame_no;
		}

		struct Hasher {
			template <typename T>
			size_t GetHash(const T& val) const {
				return std::hash<T>()(val);
			}

			size_t operator ()(const CacheKey& val) const {
				return GetHash(val.scene_id) ^ GetHash(val.layer_num) ^ GetHash(val.obj_addr) ^ GetHash(val.effect_index) ^ GetHash(val.frame_no);
			}
		};
	};

	static MemMap s_memmap(16 * 1024 * 1024);
	static CacheControl<CacheKey, MemMap::Handle, CacheKey::Hasher> s_cache;



	BOOL FilterProc(FILTER* fp, FILTER_PROC_INFO* fpip) {
		// TODO: 拡張編集呼び出しかそうで無いかを区別するコードを追加する

		//CMemref memref;
		//memref.Init(fp);

		//auto processing_filter = memref.Exedit_ScriptProcessingFilter();

		//auto filters = memref.Exedit_StaticFilterTable();
		//for (auto filter = filters; (*filter); filter++) {
		//	OutputDebugStringA((*filter)->name);
		//	OutputDebugStringA("\n");
		//}

		static LONG recurse;

		auto effect_index = GetEffectIndexInExEditObject(fpip);
		auto obj = GetExEditObject(fpip);

		CacheKey key = { obj->scene_set, obj->layer_disp, (uintptr_t)obj, (int)effect_index, fpip->frame };

		DWORD audio_bytes = sizeof(short) * fpip->audio_ch * fpip->audio_n;


		MemMap::Handle hnd;
		if (!s_cache.Get(key, hnd) || hnd.GetSize() < audio_bytes) {
			hnd = s_memmap.Allocate(audio_bytes);
			s_cache.Set(key, hnd);
		}

		if (auto p = hnd.Lock<void>()) {
			memcpy(p, fpip->audiop, audio_bytes);
		}

		if (recurse > 4) return TRUE;
		recurse++;

		auto load_frame = fpip->frame_n - fpip->frame - 1;
		if (0 <= load_frame && load_frame < fpip->frame_n) {
			// 読み出したいフレームの音声フィルタを事前に動かす
			fp->exfunc->get_audio_filtering(fp, fpip->editp, load_frame, nullptr);

			CacheKey load_key = { obj->scene_set, obj->layer_disp, (uintptr_t)obj, (int)effect_index, load_frame };
			MemMap::Handle load_hnd;
			if ((s_cache.Get(load_key, load_hnd) && load_hnd.GetSize() >= audio_bytes)) {
				if (auto p = load_hnd.Lock<void>()) {
					//short* src = static_cast<short*>((void*)p);
					//for (int i = fpip->audio_n - 1; i >= 0; i--) { // 逆順
					//	auto dest = &fpip->audiop[i * fpip->audio_ch];
					//	for (int j = 0; j < fpip->audio_ch; j++) {
					//		*dest = *src;
					//	}

					//}

					short* src = static_cast<short*>((void*)p);
					short* dest = fpip->audiop;

					memcpy(fpip->audiop, p, std::min(load_hnd.GetSize(), audio_bytes));
					std::reverse((int*)fpip->audiop, (int*)(fpip->audiop + (fpip->audio_n * fpip->audio_ch)));
				}
			}
			else {
				ZeroMemory(fpip->audiop, audio_bytes);
			}
		}
		else {
			ZeroMemory(fpip->audiop, audio_bytes);
		}


		recurse--;
		return TRUE; // TRUEを返さないと以後のフィルタが処理されない
	}

	BOOL FilterInit(FILTER* fp) {
		return TRUE;
	}

	BOOL FilterUpdate(FILTER* fp, int status) {
		return TRUE;
	}


	FILTER_DLL FilterDeclaration = {
		// flag
		FILTER_FLAG_PRIORITY_HIGHEST | FILTER_FLAG_AUDIO_FILTER | FILTER_FLAG_EX_INFORMATION,
		// x, y
		0,0,
		// name
		_T("Boseki音声フィルタ(反転)"),
		// track_n, track_name[], track_default[], track_start[], track_end[]
		0,NULL,NULL,NULL,NULL,
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
		_T("Boseki音声フィルタ(反転) version 1.0 by Totoki Kei"),
		// func_save_start
		NULL,
		// func_save_end
		NULL,

		// ここから先のメンバは0初期値
	};

}