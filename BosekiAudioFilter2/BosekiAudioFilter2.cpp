#include "pch.h"

#include "AviUtl2.h"

#include "Logger.h"

#include "AudioWave.h"

extern "C" {
	Logger g_logger;

	FILTER_PLUGIN_TABLE* GetFilterPluginTable(void) {
		g_logger.PluginLog(L"BosekiAudioFilter2: Registered with host application.");
		return &AudioWave::FilterDeclaration;
	}

	bool InitializePlugin(DWORD version) {
		g_logger.PluginLog(L"BosekiAudioFilter2: Plugin initialized. Host version: {:#010x}", version);
		return true;
	}

	void UninitializePlugin() {
		g_logger.PluginLog(L"BosekiAudioFilter2: Plugin uninitialized.");
	}


	void InitializeLogger(LOG_HANDLE* logger) {
		g_logger.SetLogger(logger);
		g_logger.PluginLog(L"BosekiAudioFilter2: Logger initialized.");
			
	}
} // extern "C"
