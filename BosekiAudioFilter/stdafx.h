﻿// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define NOMINMAX
//#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
// Windows ヘッダー ファイル:
#include <tchar.h>
#include <windows.h>
#include <limits>


// TODO: プログラムに必要な追加ヘッダーをここで参照してください
#include <AviUtl/filter.h>

template <typename T, typename ValueT>
T clamp(ValueT val) {
	if (val < std::numeric_limits<T>::min()) return std::numeric_limits<T>::min();
	if (std::numeric_limits<T>::max() < val) return std::numeric_limits<T>::max();
	return static_cast<T>(val);
}

