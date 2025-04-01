#pragma once
#include "tshark_datatype.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "ip2region_util.h"
#include "AdapterMonitorInfo.h"
#include "MiscUtil.hpp"
#include "Translator.hpp"
#include "TsharkDatabase.hpp"
#include "QueryCondition.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>
#include <unordered_map>
#include <mutex>


using std::string;



class TsharkManager
{
public:
	TsharkManager(string workDir);
	~TsharkManager();

	bool analysisFile(string filePath, size_t& packetNum);

	// 重載analysisFile
	bool analysisFile(string filePath);

	void printAllPackets();

	bool getPacketHexData(uint32_t frameNumber, std::vector<unsigned char>& data);

	vector<AdapterInfo> getNetworkAdapter();

	bool startCapture(string adapterName);

	bool stopCapture();

	// 開始監控所有網路卡流量統計數據
	void startMonitorAdaptersFlowTrend();

	void stopMonitorAdaptersFlowTrend();

	// 獲取所有網卡流量統計數據
	void getAdapterFlowTrendData(std::map < std::string, std::map<long, long>>& flowTrendData);

	// 取得單個封包的詳細資訊
	bool getPacketDetailInfo(uint32_t frameNumber, std::string& result);

	void queryPackets(QueryCondition& queryCondition, std::vector<std::shared_ptr<Packet>>& packets);

private:
	bool parseline(string line, shared_ptr<Packet> packet);

private:
	string tsharkPath;

	string editcapPath;

	string currentFilePath;

	std::unordered_map<uint32_t, shared_ptr<Packet>> allPackets;

	// 在線採集數據包的工作線程
	void captureWorkThreadEntry(string adapterName);

	// 在線分析線程
	std::shared_ptr<std::thread> captureWorkThread;

	bool stopFlag = false;

	// 後台流量趨勢監控
	std::map<string, AdapterMonitorInfo> adapterFlowTrendMonitorMap;

	// 因為map並非線程安全，所以需要加鎖來同步，使用遞迴鎖
	std::recursive_mutex adapterFlowTrendMapLock;
	
	long adapterFlowTrendMonitorStartTime = 0;

	void adapterFlowTrendMonitorThreadEntry(string adapterName);

	// 等待存入資料庫的數據
	std::vector<std::shared_ptr<Packet>> packetsTobeStore;

	// 訪問待存數據的鎖
	std::mutex storeLock;

	// 負責將獲取到的數據包存入資料庫的線程
	std::shared_ptr<std::thread> storageThread;

	std::shared_ptr<TsharkDatabase> storage;

	void storageThreadEntry();

	void processPacket(std::shared_ptr<Packet> packet);

};

