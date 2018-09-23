#include "stdafx.h"
#include "SystemInfo.h"


SystemInfo::SystemInfo()
{
	GetSystemInfo(&sysinfo_);
}


SystemInfo::~SystemInfo()
{
}

SystemInfo g_sysinfo;
