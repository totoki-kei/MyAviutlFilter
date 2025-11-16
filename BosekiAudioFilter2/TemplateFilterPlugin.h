#pragma once
#include <AviUtl2/filter2.h>

namespace TemplateFilterPlugin {
	class Plugin {
		public:
		static FILTER_PLUGIN_TABLE* GetFilterPluginTable();
		static bool InitializePlugin(DWORD version);
		static void UninitializePlugin();
		static void InitializeLogger(LOG_HANDLE* logger);
	};
}