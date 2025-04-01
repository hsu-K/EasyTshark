#pragma once
#include <iostream>
#include <map>
#include <thread>

#ifdef _WIN32
#include <windows.h>
typedef DWORD PID_T;
#else
typedef pid_t PID_T;
#endif

class AdapterMonitorInfo
{
public:
	AdapterMonitorInfo();
	std::string adapterName;
	std::map<long, long> flowTrendData;		// key 是時間戳，value 是字節數
	std::shared_ptr<std::thread> monitorThread;		// 負責監控網路卡輸出的線程
	FILE* monitorTsharkPipe;		// 線程與tshark通訊的管道
	PID_T tsharkPid;				// 負責捕獲該網路卡數據的tshark進程PID

};

