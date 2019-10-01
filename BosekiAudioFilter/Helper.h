#pragma once

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


