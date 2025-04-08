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
#include "Session.hpp"

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

enum WORK_STATUS {
	STATUS_IDLE = 0,
	STATUS_ANALYSIS = 1,
	STATUS_CAPTURING = 2,
	STATUS_MONITORING = 3
};

static std::map<uint8_t, std::string> ipProtoMap = {
	{1, "ICMP"},
	{2, "IGMP"},
	{6, "TCP"},
	{17, "UDP"},
	{47, "GRE"},
	{50, "ESP"},
	{51, "AH"},
	{88, "EIGRP"},
	{89, "OSPF"},
	{132, "SCTP"}
};


class FiveTuple {
public:
	std::string src_ip;
	std::string dst_ip;
	uint16_t src_port;
	uint16_t dst_port;
	std::string trans_protol;

	bool operator==(const FiveTuple& other) const {
		return ((src_ip == other.src_ip && dst_ip == other.dst_ip && src_port == other.src_port && dst_port == other.dst_port) 
			|| (src_ip == other.dst_ip && dst_ip == other.src_ip && src_port == other.dst_port && dst_port == other.src_port))
			&& trans_protol == other.trans_protol;
	}
	
};

// 自定義的hash函數
class FiveTupleHash {
public:
	std::size_t operator()(const FiveTuple& tuple) const {
		std::hash<std::string> hashFn;
		std::size_t h1 = hashFn(tuple.src_ip);
		std::size_t h2 = hashFn(tuple.dst_ip);
		std::size_t h3 = std::hash<uint16_t>()(tuple.src_port);
		std::size_t h4 = std::hash<uint16_t>()(tuple.dst_port);

		// 確保不論是正向還是反向都能得到相同的hash值
		std::size_t directHash = h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
		std::size_t reverseHash = h2 ^ (h1 << 1) ^ (h4 << 2) ^ (h3 << 3);

		return directHash ^ reverseHash;
	}

};

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

	// 取得所有網路卡資訊
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

	// 將數據包格式轉為舊的pcap格式
	bool convertToPcap(const std::string inputFile, const  std::string& outputFile);

	// 獲取當前狀態
	WORK_STATUS getWorkStatus();

	// 重製tsharkManager的所有狀態
	void reset();

	void printAllSessions();

private:
	bool parseline(string line, shared_ptr<Packet> packet);

private:
	string workDir;

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

	// 工作狀態
	WORK_STATUS workStatus = STATUS_IDLE;
	std::recursive_mutex workStatusLock;

	std::unordered_map<FiveTuple, std::shared_ptr<Session>, FiveTupleHash> sessionMap;

};

