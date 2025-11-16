#pragma once

#include "AviUtl2.h"

#include <format>

class Logger {

	typedef void (*LOG_CALLBACK)(LOG_HANDLE* handle, const wchar_t* message);

public:

	template<typename... Args>
	void Info(const wchar_t* format, Args... args) {
		if (logger_) {
			if (auto out_fn = logger_->info) {
				SetLog(out_fn, format, args...);
			}
		}
	}

	template<typename... Args>
	void Warn(const wchar_t* format, Args... args) {
		if (logger_) {
			if (auto out_fn = logger_->warn) {
				SetLog(out_fn, format, args...);
			}
		}
	}

	template<typename... Args>
	void Error(const wchar_t* format, Args... args) {
		if (logger_) {
			if (auto out_fn = logger_->error) {
				SetLog(out_fn, format, args...);
			}
		}
	}

	template<typename... Args>
	void Verbose(const wchar_t* format, Args... args) {
		if (logger_) {
			if (auto out_fn = logger_->verbose) {
				SetLog(out_fn, format, args...);
			}
		}
	}

	template <typename... Args>
	void PluginLog(const wchar_t* format, Args... args) {
		if (logger_) {
			if (auto out_fn = logger_->log) {
				SetLog(out_fn, format, args...);
			}
		}
	}

	template <typename... Args>
	void SetLog (LOG_CALLBACK log, const wchar_t* format, Args... args) {
		if constexpr (sizeof...(args) == 0) {
			log(logger_, format);
			return;
		}
		else {
			auto message = std::vformat(format, std::make_wformat_args(args...));
			log(logger_, message.c_str());
		}
	}

	void SetLogger(LOG_HANDLE* logger) {
		logger_ = logger;
	}

private:
	LOG_HANDLE* logger_ = nullptr;
};
