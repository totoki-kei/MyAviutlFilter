// BosekiSoundFilter.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"

#include "AudioMultiplier.h"
#include "AudioPosition.h"
#include "AudioWave.h"

//---------------------------------------------------------------------
// フィルタ構造体のポインタを渡す関数
//---------------------------------------------------------------------
EXTERN_C FILTER_DLL** cdecl GetFilterTableList(void)
{
	//must terminate with nullptr
	//http://qiita.com/yumetodo/items/4d972da03b3be788fcda
	static FILTER_DLL* pluginlist[] = {
		&AudioMultiplier::FilterDeclaration,
		&AudioPosition::FilterDeclaration,
		&AudioWave::FilterDeclaration,
		nullptr,
	};

	return pluginlist;

}