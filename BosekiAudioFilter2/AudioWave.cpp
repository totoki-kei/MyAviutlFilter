#include "pch.h"
#include "AudioWave.h"

//#include "Helper.h"

#include <memory>
#include <vector>
#include <cmath>

namespace AudioWave {


	auto frequency = FILTER_ITEM_TRACK(L"周波数", 440, 1, 44000);

	auto volume = FILTER_ITEM_TRACK(L"音量", 10000, -10000, 10000);

	auto wave_mode_items = new FILTER_ITEM_SELECT::ITEM[]{
		{ L"正弦波", 0 },
		{ L"矩形波", 1 },
		{ L"三角波", 2 },
		{ L"のこぎり波", 3 },
		{ nullptr, 0 }
	};
	auto wave_mode = FILTER_ITEM_SELECT(L"波形モード", 0, wave_mode_items);

	auto operation_mode_items = new FILTER_ITEM_SELECT::ITEM[]{
		{ L"上書き", 0 },
		{ L"加算", 1 },
		{ L"減算", 2 },
		{ L"乗算(%)", 3 },
		{ nullptr, 0 }
	};
	auto operation_mode = FILTER_ITEM_SELECT(L"演算モード", 0, operation_mode_items);

	void* FilterControlItems[] = {
		&frequency,
		&volume,
		&wave_mode,
		&operation_mode,
		nullptr
	};

	bool FilterProcAudio(FILTER_PROC_AUDIO* audio) {
		// サンプリングレート導出
		int sample_rate = audio->scene->sample_rate;
		// サンプル数
		int sample_num = audio->object->sample_num;
		// 1サンプルあたりの時間
		double time_per_sample = 1.0 / sample_rate;

		// 波形データのバッファ(1チャネル分）
		auto wave_buffer = std::make_unique<float[]>(sample_num);

		// 波形の時間(t)
		double time = audio->object->time;

		// バッファを埋める
		for (int i = 0; i < sample_num; i++) {
			auto t = fmod(time * frequency.value, 1);
			auto vol = (volume.value / 10000.0);
			double val = 0;
			switch (wave_mode.value) {
			case 0: // 正弦波
				val = sin(t * 3.141592653589793 * 2) * vol;
				break;
			case 1: // 矩形波
				val = (t < 0.5 ? 1.0 : -1.0) * vol;
				break;
			case 2: // 三角波
				val = (t < 0.25 ? t * 4 : t < 0.75 ? -4 * t + 2 : 4 * t - 4) * vol;
				break;
			case 3: // のこぎり波
				val = (t * 2 - 1) * vol;
				break;
			}
			wave_buffer[i] = static_cast<float>(val);

			time += time_per_sample;
		}

		// 各チャネルへの反映
		for (int ch = 0; ch < audio->object->channel_num; ch++) {
			if (operation_mode.value == 0) {
				// そのまま上書きする
				audio->set_sample_data(wave_buffer.get(), ch);
			}
			else {
				// 現在の波形データを取得する
				auto operation_buffer = std::make_unique<float[]>(sample_num);
				audio->get_sample_data(operation_buffer.get(), ch);

				// 波形データを合成する
				for (int i = 0; i < sample_num; i++) {
					switch (operation_mode.value) {
					case 0: // 上書き
						operation_buffer[i] = wave_buffer[i];
						break;
					case 1: // 加算
						operation_buffer[i] += wave_buffer[i];
						break;
					case 2: // 減算
						operation_buffer[i] -= wave_buffer[i];
						break;
					case 3: // 乗算(%)
						operation_buffer[i] *= (wave_buffer[i] / 100);
						break;
					}
				}

				// 合成した波形データをセットする
				audio->set_sample_data(operation_buffer.get(), ch);
			}
		}


		return 1;
	}

	FILTER_PLUGIN_TABLE FilterDeclaration = {
		/* flag */ FILTER_PLUGIN_TABLE::FLAG_AUDIO | FILTER_PLUGIN_TABLE::FLAG_INPUT,
		/* name */ L"Boseki Wave Generator",
		/* label */ L"",
		/* information */ L"",
		/* items */ reinterpret_cast<void**>(FilterControlItems),
		/* func_proc_video */ nullptr,
		/* func_proc_audio */ FilterProcAudio
	};

}