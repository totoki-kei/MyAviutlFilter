#pragma once

#include <aulslib\exedit.h>
//#include <aulslib\memref.h>

template <typename T, typename ValueT>
T clamp(ValueT val) {
	if (val < std::numeric_limits<T>::min()) return std::numeric_limits<T>::min();
	if (std::numeric_limits<T>::max() < val) return std::numeric_limits<T>::max();
	return static_cast<T>(val);
}


template <size_t TrackCount>
constexpr int GetTrackCount(LPCTSTR(&names)[TrackCount], int(&default_values)[TrackCount], int(&min_values)[TrackCount], int(&max_values)[TrackCount]) {
	return TrackCount;
}


template <size_t CheckCount>
constexpr int GetCheckCount(LPCTSTR(&names)[CheckCount], int(&default_values)[CheckCount]) {
	return CheckCount;
}



inline DWORD GetEffectIndexInExEditObject(FILTER_PROC_INFO* fpip) {
#ifdef FOR_EXEDIT_VER_0_93
	return *((DWORD*)fpip - 8);
#else
	return *((DWORD*)fpip - 12);
#endif
}

inline auls::EXEDIT_OBJECT* GetExEditObject(FILTER_PROC_INFO* fpip) {
#ifdef FOR_EXEDIT_VER_0_93
	return *((auls::EXEDIT_OBJECT**)fpip - 5);
#else
	return *((auls::EXEDIT_OBJECT**)fpip - 13);
#endif
}
