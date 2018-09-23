#pragma once
class SystemInfo
{
	SYSTEM_INFO sysinfo_;
public:
	SystemInfo();
	~SystemInfo();

	DWORD GetAllocationGranularity() const {
		return sysinfo_.dwAllocationGranularity;
	}
};

extern SystemInfo g_sysinfo;
