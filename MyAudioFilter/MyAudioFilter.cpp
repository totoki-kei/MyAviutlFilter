// MyAudioFilter.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"

#include <tchar.h>
#include <windows.h>
#include <limits>
#include <memory>
#include <vector>
#include <thread>

#include <AviUtl/filter.h>

#define FILTER_TEST 2

#if defined(FILTER_TEST)

#if FILTER_TEST == 1

//---------------------------------------------------------------------
// フィルタ構造体定義
//---------------------------------------------------------------------
static int     track_count = 1;
static LPCTSTR track_names[]          = { _T("音量") };
static int     track_default_values[] = {       100 };
static int     track_min_values[]     = {         0 };
static int     track_max_values[]     = {       200 };

FILTER_DLL filter = {
//  flag
    FILTER_FLAG_AUDIO_FILTER,
//  x, y
    0, 0,
//  name
    _T("音声フィルタテスト(1)"),
//  track_n,     track_name[], track_default[],      track_start[],    track_end[]
    track_count, track_names,  track_default_values, track_min_values, track_max_values,
//  check_n, check_name[], check_default[]
    NULL,    NULL,         NULL,
//  func_proc
    func_proc,
//  func_init
    NULL,
//  func_exit
    NULL,
//  func_update
    NULL,
//  func_WndProc
    NULL,
//  track, check
    NULL,  NULL,
//  ex_data_ptr
    NULL,
//  ex_data_size
    NULL,
//  information
    _T("音声フィルタ(1) version 0.01"),
//  func_save_start
    NULL,
//  func_save_end
    NULL,

    // ここから先のメンバは0初期値
};


//---------------------------------------------------------------------
// フィルタ処理関数
//---------------------------------------------------------------------
static BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip)
{
    // 音量を変更する
    for (int i = 0; i < fpip->audio_n * fpip->audio_ch; i++) {
        double vol = (fpip->audiop[i] * fp->track[0] / 100.0);

        // 上限/下限をオーバーしていたら上限/下限に補正する
        const double short_min = std::numeric_limits<short>::min();
        const double short_max = std::numeric_limits<short>::max();
        if (vol < short_min) vol = short_min;
        if (short_max < vol) vol = short_max;

        fpip->audiop[i] = static_cast<short>(vol);
    }

    return TRUE;
}

#elif FILTER_TEST == 2
//---------------------------------------------------------------------
// フィルタ構造体定義
//---------------------------------------------------------------------
static int     track_count            = 1;
static LPCTSTR track_names[]          = { _T("左右") };
static int     track_default_values[] = {         0 };
static int     track_min_values[]     = {      -100 };
static int     track_max_values[]     = {       100 };

static int     check_count = 1;
static LPCTSTR check_names[] = { _T("ボタン") };
static int     check_default[] = { -1 };

FILTER_DLL filter = {
	//  flag
	FILTER_FLAG_AUDIO_FILTER
	| FILTER_FLAG_ALWAYS_ACTIVE/*[フィルタ]メニューに表示されなくなる*/
	| FILTER_FLAG_NO_CONFIG    /*[編集]メニューに表示されなくなる*/
	| FILTER_FLAG_EX_INFORMATION
	,
	//  x, y
	0, 0,
	//  name
	_T("音声フィルタテスト(2)"),
	//  track_n,     track_name[], track_default[],      track_start[],    track_end[]
	track_count, track_names,  track_default_values, track_min_values, track_max_values,
	//  check_n, check_name[], check_default[]
	check_count, check_names,  check_default,
	//  func_proc
	func_proc,
	//  func_init
	func_init,
	//  func_exit
	func_exit,
	//  func_update
	func_update,
	//  func_WndProc
	func_WndProc,
	//  track, check
	NULL,  NULL,
	//  ex_data_ptr
	NULL,
	//  ex_data_size
	NULL,
	//  information
	_T("音声フィルタ(2) version 0.01"),
	//  func_save_start
	NULL,
	//  func_save_end
	NULL,

	// ここから先のメンバは0初期値
};

template <typename T, typename ValueT>
T clamp(ValueT val) {
	if (val < std::numeric_limits<T>::min()) return std::numeric_limits<T>::min();
	if (std::numeric_limits<T>::max() < val) return std::numeric_limits<T>::max();
	return static_cast<T>(val);
}

//---------------------------------------------------------------------
// フィルタ処理関数
//---------------------------------------------------------------------
static BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip)
{
    // 予防線: チャンネル数が2でない場合は何もしない
    if (fpip->audio_ch != 2) return TRUE;

    // 左右割合を変更する
    for (int i = 0; i < fpip->audio_n; i++) {
        // チャンネル0を左、チャンネル1を右として処理を記述する
        short left  = fpip->audiop[2 * i];
        short right = fpip->audiop[2 * i + 1];

        double balance = fp->track[0] / 100.0;

        double left_left, left_right, right_left, right_right;

        if (balance < 0) {
            // 左に寄せる
            left_left   = 1.0;
            left_right  = 0.0 - balance;
            right_left  = 0.0;
            right_right = 1.0 + balance;
        }
        else {
            // 右に寄せる
            left_left   = 1.0 - balance;
            left_right  = 0.0;
            right_left  = 0.0 + balance;
            right_right = 1.0;
        }

        fpip->audiop[2 * i]     = clamp<short>(left * left_left  + right * left_right );
        fpip->audiop[2 * i + 1] = clamp<short>(left * right_left + right * right_right);
    }

    return TRUE;
}


#elif FILTER_TEST == 3

//---------------------------------------------------------------------
// フィルタ構造体定義
//---------------------------------------------------------------------
static int     track_count            = 2;
static LPCTSTR track_names[]          = { _T("遅延(ms)"), _T("強さ(%)") };
static int     track_default_values[] = {             1,            50 };
static int     track_min_values[]     = {             1,             0 };
static int     track_max_values[]     = {          1000,           100 };

static int     check_count     = 1;
static LPCTSTR check_names[]   = { _T("再帰的に処理") };
static int     check_default[] = {                1 };

FILTER_DLL filter = {
	//  flag
	FILTER_FLAG_AUDIO_FILTER,
	//  x, y
	0, 0,
	//  name
	_T("音声フィルタテスト(3)"),
	//  track_n,     track_name[], track_default[],      track_start[],    track_end[]
	track_count, track_names,  track_default_values, track_min_values, track_max_values,
	//  check_n, check_name[], check_default[]
	check_count, check_names,  check_default,
	//  func_proc
	func_proc,
	//  func_init
	NULL,
	//  func_exit
	NULL,
	//  func_update
	NULL,
	//  func_WndProc
	NULL,
	//  track, check
	NULL,  NULL,
	//  ex_data_ptr
	NULL,
	//  ex_data_size
	NULL,
	//  information
	_T("音声フィルタ(3) version 0.01"),
	//  func_save_start
	NULL,
	//  func_save_end
	NULL,

	// ここから先のメンバは0初期値
};

template <typename T, typename ValueT>
T clamp(ValueT val) {
	if (val < std::numeric_limits<T>::min()) return std::numeric_limits<T>::min();
	if (std::numeric_limits<T>::max() < val) return std::numeric_limits<T>::max();
	return static_cast<T>(val);
}

std::vector<short> GetAudioData(FILTER *fp, FILTER_PROC_INFO* fpip, int frame) {
	if (frame == fpip->frame) {
		// 現在のフレームだった場合はfpipのデータをコピー
		std::vector<short> buffer(fpip->audio_n * fpip->audio_ch);
		memcpy(buffer.data(), fpip->audiop, buffer.size());
		return buffer;
	}

	int length = fp->exfunc->get_audio_filtering(fp, fpip->editp, frame, nullptr);
	std::vector<short> buffer(length * fpip->audio_ch);
	fp->exfunc->get_audio_filtering(fp, fpip->editp, frame, buffer.data());

	return buffer;
}

//---------------------------------------------------------------------
// フィルタ処理関数
//---------------------------------------------------------------------
static BOOL func_proc(FILTER *fp, FILTER_PROC_INFO *fpip)
{
	static thread_local DWORD recursive = 0;
	if (fp->check[0] == 0) {
		// 再帰呼び出しチェック
		if (recursive) return TRUE;
	}

	// サンプリングレート導出
	FILE_INFO file_info;
	if (!fp->exfunc->get_file_info(fpip->editp, &file_info)) return FALSE;
	int sample_rate = file_info.audio_rate;

	// ディレイ時間をミリ秒からサンプル数に変換
	int delay_samples = sample_rate * fp->track[0] / 1000;

	// 該当するフレーム番号を導出
	int samples = 0;
	int frame = fpip->frame - 1;
	while (samples < delay_samples && frame >= 0) {
		samples += fp->exfunc->get_audio_filtering(fp, fpip->editp, frame, nullptr);
		frame--;
	}

	if (samples < delay_samples) {
		// 過去データが無い
		return TRUE;
	}

	recursive++;

	// ディレイ用音声データ
	auto delay_buffer = GetAudioData(fp, fpip, frame);
	// データ参照時のオフセット
	int delay_offset = (samples - delay_samples) * fpip->audio_ch;

	for (int i = 0; i < fpip->audio_n * fpip->audio_ch; i++) {
		// データが足りなくなった時は次のフレームに移動
		if (delay_offset + i >= (int)delay_buffer.size()) {
			frame++;
			delay_buffer = GetAudioData(fp, fpip, frame);
			delay_offset = -i;
		}

		fpip->audiop[i] = clamp<short>(fpip->audiop[i] + delay_buffer[delay_offset + i] * fp->track[1] / 100.0);
	}

	recursive--;

	return TRUE;
}


#endif

#endif
//---------------------------------------------------------------------
// フィルタ構造体のポインタを渡す関数
//---------------------------------------------------------------------
extern "C" FILTER_DLL * __stdcall GetFilterTable(void)
{
	return &filter;
}


BOOL func_init(FILTER *fp) {
	
	OutputDebugStringA(__FUNCTION__ _T("\n"));
	return TRUE;
}
BOOL func_exit(FILTER *fp) {
	OutputDebugStringA(__FUNCTION__ _T("\n"));
	return TRUE;
}
BOOL func_update(FILTER *fp, int status) {
	OutputDebugStringA(__FUNCTION__ _T("\n"));
	return TRUE;
}
BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void *editp, FILTER *fp) {
	OutputDebugStringA(__FUNCTION__ _T("\n"));
	if (message == WM_COMMAND && wparam == MID_FILTER_BUTTON + 0) {
		MessageBox(hwnd, "ボタンが押されました。", NULL, MB_OK);
	}
	return TRUE;
}
