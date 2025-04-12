#include "TsharkManager.h"



#ifdef _WIN32
#include <windows.h>
typedef DWORD PID_T;
#else
typedef pid_t PID_T;
#endif

PID_T captureTsharkPid;



// 時間格式的轉換
string printHumanReadableTime(double timestamp) {  
   // 將時間戳分為整數部分和小數部分  
   //double timestamp = std::stod(str_timestamp, nullptr);  

   time_t seconds = static_cast<time_t>(timestamp);  
   double fractional_seconds = timestamp - seconds;  

   // 將整數部分轉換為 tm 結構  
   struct tm timeinfo;  
   gmtime_s(&timeinfo, &seconds);  

   // 打印日期和時間  
   // 初始化一個輸出字符串流，用於將數據格式化為字符串
   std::ostringstream oss;  
   oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S") << "." << std::setw(9) << std::setfill('0') << static_cast<int>(fractional_seconds * 1e9);
   string tw_time = oss.str();  
   //cout << tw_time << endl;
   return tw_time;
}

TsharkManager::TsharkManager(string workDir)
{
    this->workDir = workDir;
    IP2RegionUtil::init("third_library\\ip2region\\ip2region.xdb");
    tsharkPath = "C:/Reverse_tools/Wireshark/tshark";
    editcapPath = "C:/Reverse_tools/Wireshark/editcap";
    // 初始化儲存數據庫
    std::string dbFullPath = this->workDir + "\\mytshark.db";
    storage = std::make_shared<TsharkDatabase>(dbFullPath);
}

TsharkManager::~TsharkManager()
{  
}

bool TsharkManager::analysisFile(string filePath, size_t& packetNum)
{
    reset();

    // 統一轉換成標準的pcap格式
    currentFilePath = MiscUtil::getPcapNameByCurrentTimestamp();
    if (!convertToPcap(filePath, currentFilePath)) {
        LOG_F(ERROR, "convert to pcap failed");
        return false;
    }

    std::vector<string> tsharkArgs = {
        tsharkPath,
        "-r", filePath,
        "-T", "fields",
        "-e", "frame.number",
        "-e", "frame.time_epoch",
        "-e", "frame.len",
        "-e", "frame.cap_len",
        "-e", "eth.src",
        "-e", "eth.dst",
        "-e", "ip.src",
        "-e", "ipv6.src",
        "-e", "ip.dst",
        "-e", "ipv6.dst",
        "-e", "ip.proto",
        "-e", "ipv6.nxt",
        "-e", "tcp.srcport",
        "-e", "udp.srcport",
        "-e", "tcp.dstport",
        "-e", "udp.dstport",
        "-e", "_ws.col.Protocol",
        "-e", "_ws.col.Info"
    };

    string command;
    for (auto arg : tsharkArgs) {
        command += arg;
        command += " ";
    }
    //cout << command << endl;

    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to run tshark command!" << std::endl;
        return false;
    }

    // 打開儲存數據包進程
    stopFlag = false;
    storageThread = std::make_shared<std::thread>(&TsharkManager::storageThreadEntry, this);

    char buffer[4096];

    uint32_t file_offset = sizeof(PcapHeader);
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::shared_ptr<Packet> packet = std::make_shared<Packet>();
        // 進行解析，並記錄不同欄位
        if (!parseline(buffer, packet)) {
            LOG_F(ERROR, buffer);
            assert(false);
        }

        // 計算當前的偏移，並記錄在packet中
        packet->file_offset = file_offset + sizeof(PacketHeader);

        file_offset = file_offset + sizeof(PacketHeader) + packet->cap_len;
        
        packet->src_location = IP2RegionUtil::getIpLocation(packet->src_ip);
        packet->dst_location = IP2RegionUtil::getIpLocation(packet->dst_ip);
//#ifdef _WIN32
//        //packet->src_location = IP2RegionUtil::getIpLocation(packet->src_ip);
//        packet->src_location = MiscUtil::UTF8ToANSIString(IP2RegionUtil::getIpLocation(packet->src_ip));
//        packet->dst_location = MiscUtil::UTF8ToANSIString(IP2RegionUtil::getIpLocation(packet->dst_ip));
//#else
//        packet->src_location = IP2RegionUtil::getIpLocation(packet->src_ip);
//        packet->dst_location = IP2RegionUtil::getIpLocation(packet->dst_ip);
//#endif
        

        // 將分析的數據包保存起來
        processPacket(packet);
        //allPackets.insert(std::make_pair<>(packet->frame_number, packet));
    
    }

    LOG_F(INFO, MiscUtil::UTF8ToANSIString("分析完成，數據包總數: %d").c_str(), allPackets.size());
    packetNum = allPackets.size();

    stopFlag = true;

    _pclose(pipe);

    return true;
}

bool TsharkManager::analysisFile(string filePath)
{
    size_t temp;

    return analysisFile(filePath, temp);
}

void TsharkManager::printAllPackets()
{
    for (auto pair : allPackets) {
        std::shared_ptr<Packet> packet = pair.second;
        // 構建JSON對象
        rapidjson::Document pktObj;
        rapidjson::Document::AllocatorType& allocator = pktObj.GetAllocator();

        // 設置JSON為Object對象類型
        pktObj.SetObject();

        // 添加JSON字段
        pktObj.AddMember("frame_number", packet->frame_number, allocator);
        pktObj.AddMember("timestamp", rapidjson::Value(printHumanReadableTime(packet->time).c_str(), allocator), allocator);
        pktObj.AddMember("src_mac", rapidjson::Value(packet->src_mac.c_str(), allocator), allocator);
        pktObj.AddMember("dst_mac", rapidjson::Value(packet->dst_mac.c_str(), allocator), allocator);
        pktObj.AddMember("src_ip", rapidjson::Value(packet->src_ip.c_str(), allocator), allocator);
        pktObj.AddMember("src_location", rapidjson::Value(packet->src_location.c_str(), allocator), allocator);
        pktObj.AddMember("src_port", packet->src_port , allocator);
        pktObj.AddMember("dst_ip", rapidjson::Value(packet->dst_ip.c_str(), allocator), allocator);
        pktObj.AddMember("dst_location", rapidjson::Value(packet->dst_location.c_str(), allocator), allocator);
        pktObj.AddMember("dst_port", packet->dst_port, allocator);
        pktObj.AddMember("protocol", rapidjson::Value(packet->protocol.c_str(), allocator), allocator);
        pktObj.AddMember("info", rapidjson::Value(packet->info.c_str(), allocator), allocator);
        pktObj.AddMember("file_offset", packet->file_offset, allocator);
        pktObj.AddMember("cap_len", packet->cap_len, allocator);
        pktObj.AddMember("len", packet->cap_len, allocator);
        
        // 序列化JSON字符串
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        pktObj.Accept(writer);

        std::cout << buffer.GetString() << endl;

        std::vector<unsigned char> hexData;
        getPacketHexData(packet->frame_number, hexData);
        printf("Packet Hex: ");
        for (unsigned char byte : hexData) {
            printf("%02X ", byte);
        }
        printf("\n\n");

    }


}

bool TsharkManager::getPacketHexData(uint32_t frameNumber, std::vector<unsigned char>& data)
{
    ifstream file(currentFilePath, ios::binary);
    if (!file) {
        cerr << "無法打開檔案" << endl;
        return false;
    }

    std::shared_ptr<Packet> p = allPackets[frameNumber];
    file.seekg(p->file_offset, ios::beg);
    data.resize(p->cap_len);
    file.read(reinterpret_cast<char*>(data.data()), p->cap_len);

    file.close();
    return true;
}

vector<AdapterInfo> TsharkManager::getNetworkAdapter()
{
    // 需要過去的虛擬網卡
    std::set<string> specialInterface = { "sshdump", "ciscodump", "udpdump", "randpkt", "etwdump"};
    std::vector<AdapterInfo> interfaces;

    char buffer[256] = { 0 };
    string result;

    string cmd = tsharkPath + " -D";
    // 定義一個智能指針，類型是FILE，當要釋放時，就使用_pclose來關閉pipe，_pclose是一個函數指針
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
    if (!pipe) {
        throw std::runtime_error("Failed to run tshark command.");
    }

    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
#ifdef _WIN32
    result = MiscUtil::UTF8ToANSIString(result);
#endif
    //cout << result << endl;

    // istringstream是用於輸入的輸入流
    std::istringstream stream(result);
    std::string line;
    int index = 1;
    while (std::getline(stream, line)) {
        //cout << line << endl;
        size_t startPos = line.find(' ');
        if (startPos != string::npos) {
            size_t endPos = line.find(' ', startPos + 1);
            string interfaceName;
            if (endPos != string::npos) {
                interfaceName = line.substr(startPos+1, endPos - (startPos + 1));
            }
            else {
                interfaceName = line.substr(startPos + 1);
            }

            // 過濾掉特殊網卡
            if (specialInterface.find(interfaceName) != specialInterface.end()) {
                continue;
            }

            AdapterInfo adapterInfo;
            adapterInfo.name = interfaceName;
            adapterInfo.id = index++;
            if (line.find("(") != string::npos && line.find(")") != string::npos) {
                adapterInfo.remark = line.substr(line.find("(") + 1, line.find(")") - (line.find("(") + 1));
            }

            interfaces.push_back(adapterInfo);
        }
    }


    return interfaces;
}

bool TsharkManager::parseline(string line, shared_ptr<Packet> packet)
{
#ifdef _WIN32
    //line = MiscUtil::UTF8ToANSIString(line);
#endif

    if (line.back() == '\n') {
        line.pop_back();
    }

    std::stringstream ss(line);
    string field;
    std::vector<string> fields;

    size_t start = 0, end;
    while ((end = line.find('\t', start)) != string::npos) {
        fields.push_back(line.substr(start, end - start));
        start = end + 1;
    }
    fields.push_back(line.substr(start));

    // 字段順序
    // 0: frame.number
    // 1: frame.time_epoch
    // 2: frame.len
    // 3: frame.cap_len
    // 4: eth.src
    // 5: eth.dst
    // 6: ip.src
    // 7: ipv6.src
    // 8: ip.dst
    // 9: ipv6.dst
    // 10: ip.proto
	// 11: ipv6.nxt
    // 12: tcp.srcport
    // 13: udp.srcport
    // 14: tcp.dstport
    // 15: udp.dstport
    // 16: _ws.col.Protocol
    // 17: _ws.col.Info

    if (fields.size() >= 18) {
        packet->frame_number = std::stoi(fields[0]);
        packet->time = std::stod(fields[1]);
        packet->len = std::stoi(fields[2]);
        packet->cap_len = std::stoi(fields[3]);
        packet->src_mac = fields[4];
        packet->dst_mac = fields[5];
        packet->src_ip = fields[6].empty() ? fields[7] : fields[6];
        packet->dst_ip = fields[8].empty() ? fields[9] : fields[8];
        if (!fields[10].empty() || !fields[11].empty()) {
            uint8_t transProtoNumber = std::stoi(fields[10].empty() ? fields[11] : fields[10]);
            if (ipProtoMap.find(transProtoNumber) != ipProtoMap.end()) {
				packet->trans_proto = ipProtoMap[transProtoNumber];
            }
        }

        if (!fields[12].empty() || !fields[13].empty()) {
            packet->src_port = std::stoi(fields[12].empty() ? fields[13] : fields[12]);
        }
        if (!fields[14].empty() || !fields[15].empty()) {
            packet->dst_port = std::stoi(fields[14].empty() ? fields[15] : fields[14]);
        }
        packet->protocol = fields[16];
        packet->info = fields[17];
        return true;
    }
    else {
        return false;
    }
}

bool TsharkManager::startCapture(string adapterName)
{
    reset();

    LOG_F(INFO, MiscUtil::UTF8ToANSIString("即將開始抓包，網路卡：%s").c_str(), adapterName.c_str());

    stopFlag = false;
    
    // 在開始抓包之前，先把儲存線程打開
    storageThread = std::make_shared<std::thread>(&TsharkManager::storageThreadEntry, this);

    // 利用智能指針來管理線程，將captureWorkThreadEntry作為入口函數進行執行
    captureWorkThread = std::make_shared<std::thread>(&TsharkManager::captureWorkThreadEntry, this, "\"" + adapterName + "\"");
    
    // 設置工作狀態
    workStatus = STATUS_CAPTURING;
    return true;
}

bool TsharkManager::stopCapture()
{
    LOG_F(INFO, MiscUtil::UTF8ToANSIString("即將停止抓包").c_str());
    stopFlag = true;

    ProcessUtil::Kill(captureTsharkPid);
    // 使用join方法，等待抓包線程的退出
    captureWorkThread->join();
    captureWorkThread.reset();

    // 等待儲存線程退出
    storageThread->join();
    storageThread.reset();

    workStatus = STATUS_IDLE;
    return true;
}

void TsharkManager::captureWorkThreadEntry(string adapterName)
{
    string captureFile = MiscUtil::getPcapNameByCurrentTimestamp();
    //string captureFile = "capture.pcap";
    std::vector<string> tsharkArgs = {
            tsharkPath,
            "-i", adapterName.c_str(),
            "-w", captureFile,
            "-F", "pcap",
            "-T", "fields",
            "-e", "frame.number",
            "-e", "frame.time_epoch",
            "-e", "frame.len",
            "-e", "frame.cap_len",
            "-e", "eth.src",
            "-e", "eth.dst",
            "-e", "ip.src",
            "-e", "ipv6.src",
            "-e", "ip.dst",
            "-e", "ipv6.dst",
            "-e", "ip.proto",
            "-e", "ipv6.nxt",
            "-e", "tcp.srcport",
            "-e", "udp.srcport",
            "-e", "tcp.dstport",
            "-e", "udp.dstport",
            "-e", "_ws.col.Protocol",
            "-e", "_ws.col.Info",
    };

    string command;
    for (auto arg : tsharkArgs) {
        command += arg;
        command += " ";
    }
    //cout << command << endl;

    FILE* pipe = ProcessUtil::PopenEx(command.c_str(), &captureTsharkPid);
    //FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        LOG_F(ERROR, "Fail to run tshark command!");
        return;
    }

    char buffer[4096];

    uint32_t file_offset = sizeof(PcapHeader);
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr && !stopFlag) {
        std::shared_ptr<Packet> packet = std::make_shared<Packet>();
        // 進行解析，並記錄不同欄位

        // 忽略掉第一行 "Capturing on 網卡"
        string line = buffer;
        if (line.find("Capturing on") != string::npos) {
            continue;
        }

        if (!parseline(buffer, packet)) {
            LOG_F(ERROR, buffer);
            assert(false);
        }

        // 計算當前的偏移，並記錄在packet中
        packet->file_offset = file_offset + sizeof(PacketHeader);

        file_offset = file_offset + sizeof(PacketHeader) + packet->cap_len;

#ifdef _WIN32
        packet->src_location = MiscUtil::UTF8ToANSIString(IP2RegionUtil::getIpLocation(packet->src_ip));
        packet->dst_location = MiscUtil::UTF8ToANSIString(IP2RegionUtil::getIpLocation(packet->dst_ip));
#else
        packet->src_location = IP2RegionUtil::getIpLocation(packet->src_ip);
        packet->dst_location = IP2RegionUtil::getIpLocation(packet->dst_ip);
#endif


        // 將分析的數據包保存起來
        //allPackets.insert(std::make_pair<>(packet->frame_number, packet));
        processPacket(packet);
    
    }
    
    _pclose(pipe);

    currentFilePath = captureFile;
}

// 監控網路卡流量的線程
void TsharkManager::adapterFlowTrendMonitorThreadEntry(string adapterName)
{
    // 確認這個網卡名稱在Map內
    if (adapterFlowTrendMonitorMap.find(adapterName) == adapterFlowTrendMonitorMap.end()) {
        return;
    }

    char buffer[256] = { 0 };
    std::map<long, long>& trafficPerSecond = adapterFlowTrendMonitorMap[adapterName].flowTrendData;

    string tsharkCmd = tsharkPath + " -i \"" +  adapterName + "\" -T fields -e frame.time_epoch -e frame.len";

    LOG_F(INFO, MiscUtil::UTF8ToANSIString("啟動網卡流量監控：%s").c_str(), tsharkCmd.c_str());

    PID_T tsharkPid = 0;
    FILE* pipe = ProcessUtil::PopenEx(tsharkCmd.c_str(), &tsharkPid);
    if (!pipe) {
        throw std::runtime_error("Failed to run tshark command");
    }

    // 將Pipe保存起來
    adapterFlowTrendMapLock.lock();
    adapterFlowTrendMonitorMap[adapterName].monitorTsharkPipe = pipe;
    adapterFlowTrendMonitorMap[adapterName].tsharkPid = tsharkPid;
    adapterFlowTrendMapLock.unlock();

    // 讀取tshark輸出
    while(fgets(buffer, sizeof(buffer), pipe) != nullptr){
        std::string line(buffer);
        std::istringstream iss(line);
        std::string timestampStr, lengthStr;

        if (line.find("Capturing") != std::string::npos || line.find("captured") != std::string::npos) {
            continue;
        }

        // 解析每行的時間戳和數據包長度
        if (!(iss >> timestampStr >> lengthStr)) {
            continue;
        }

        try {
            // 轉換時間戳為long類型，秒數部分，先轉成double再轉成long
            long timestamp = static_cast<long>(std::stod(timestampStr));

            long packetLength = std::stol(lengthStr);

            // 每秒字節數累加
            trafficPerSecond[timestamp] += packetLength;

            // 如果超過300秒，就刪除最早的數據，只保留最近300秒的數據
            while (trafficPerSecond.size() > 300) {
                auto it = trafficPerSecond.begin();
                LOG_F(INFO, "Removing old data for second: %ld, Traffic: %ld bytes", it->first, it->second);
                trafficPerSecond.erase(it);
            }
        }
        catch (const std::exception&){
            LOG_F(ERROR, "Error parsing tshark output: %s", line.c_str());
        }

    }
    LOG_F(INFO, MiscUtil::UTF8ToANSIString("adapterFlowTrendMonitorThreadEntry 已結束").c_str());
}

void TsharkManager::storageThreadEntry()
{
    // lambda表達式，定義一個局部作用的匿名函數
    auto storageWork = [this]() {
        storeLock.lock();

        // 判斷現在是否有新的數據要存入
        if (!packetsTobeStore.empty()) {
            storage->storePackets(packetsTobeStore);
            packetsTobeStore.clear();
        }

        if (!sessionSetTobeStore.empty()) {
            storage->storeAndUpdateSessions(sessionSetTobeStore);
			sessionSetTobeStore.clear();
        }

        storeLock.unlock();
    };


    // 直到讀取線程停止前，每隔一段時間就執行寫入
    while (!stopFlag) {
        storageWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 線程停止前要再做最後一次儲存，可以先等一下，防止有遺漏的數據
    std::this_thread::sleep_for(std::chrono::seconds(1));
    storageWork();
}

void TsharkManager::processPacket(std::shared_ptr<Packet> packet)
{
    // 將分析的數據包保存起來
    allPackets.insert(std::make_pair<>(packet->frame_number, packet));


    // 接著放入packetsTobeStore等待進入數據庫
    storeLock.lock();
    packetsTobeStore.push_back(packet);
    storeLock.unlock();

    // 只處理TCP和UDP協議
    if (packet->trans_proto == "TCP" || packet->trans_proto == "UDP") {
        // 創建五元組
        FiveTuple tuple{ packet->src_ip, packet->dst_ip, packet->src_port, packet->dst_port, packet->trans_proto };

        std::shared_ptr<Session> session;
        if (sessionMap.find(tuple) == sessionMap.end()) {
			// 創建新的session，因為沒有找到對應的五元組
            session = std::make_shared<Session>();
            session->session_id = sessionMap.size() + 1;
			session->ip1 = packet->src_ip;
			session->ip2 = packet->dst_ip;
			session->ip1_location = packet->src_location;
			session->ip2_location = packet->dst_location;
			session->ip1_port = packet->src_port;
			session->ip2_port = packet->dst_port;
            session->start_time = packet->time;
			session->end_time = packet->time;
            session->trans_proto = packet->trans_proto;
            if (packet->protocol != "TCP" && packet->protocol != "UDP") {
				session->app_proto = packet->protocol;
            }

            sessionMap.insert(std::make_pair<>(tuple, session));
            sessionIdMap.insert(std::make_pair<>(session->session_id, session));
        }
        else {
            // 已經有對應的五元組，就更新seesion
            session = sessionMap[tuple];
			session->end_time = packet->time;
            if (packet->protocol != "TCP" && packet->protocol != "UDP") {
                session->app_proto = packet->protocol;
            }
        }

        session->packet_count++;
		session->total_bytes += packet->len;
		packet->belong_session_id = session->session_id;

        // 統計雙方交互的數據
		if (session->ip1 == packet->src_ip) {
			session->ip1_send_packets_count++;
			session->ip1_send_bytes_count += packet->len;
		}
		else if (session->ip2 == packet->src_ip) {
			session->ip2_send_packets_count++;
			session->ip2_send_bytes_count += packet->len;
		}

		storeLock.lock();
		sessionSetTobeStore.insert(session);
        storeLock.unlock();
    
    }
}

// 開始監控網卡流量，為每個網路卡開一個線程監控
void TsharkManager::startMonitorAdaptersFlowTrend()
{
    reset();

    std::unique_lock<std::recursive_mutex> lock(adapterFlowTrendMapLock);

    adapterFlowTrendMonitorStartTime = static_cast<long>(time(nullptr));

    // 獲取網卡列表
    std::vector<AdapterInfo> adapterList = getNetworkAdapter();

    // 每個網卡都啟動一個線程，統計對應網路卡的數據
    for (auto adapter : adapterList) {
        adapterFlowTrendMonitorMap.insert(std::make_pair<>(adapter.name, AdapterMonitorInfo()));
        AdapterMonitorInfo& monitorInfo = adapterFlowTrendMonitorMap.at(adapter.name);

        monitorInfo.adapterName = adapter.name;
        monitorInfo.monitorThread = std::make_shared<std::thread>(&TsharkManager::adapterFlowTrendMonitorThreadEntry, this, adapter.name);
        if (monitorInfo.monitorThread == nullptr) {
            LOG_F(ERROR, MiscUtil::UTF8ToANSIString("監控線程創建失敗，網卡名%s").c_str(), adapter.name.c_str());
        }
        else {
            LOG_F(INFO, MiscUtil::UTF8ToANSIString("監控線程創建成功，網卡名%s，monitorThread: %p").c_str(), adapter.name.c_str(), monitorInfo.monitorThread.get());
        }

    }

    // 設置工作狀態
    workStatus = STATUS_MONITORING;

}

void TsharkManager::stopMonitorAdaptersFlowTrend()
{
    std::unique_lock<std::recursive_mutex> lock(adapterFlowTrendMapLock);

    // 關閉對應的tshark進程
    for (auto adapterPipePair : adapterFlowTrendMonitorMap) {
        ProcessUtil::Kill(adapterPipePair.second.tsharkPid);
    }

    // 然後關閉管道
    for (auto adapterPipePair : adapterFlowTrendMonitorMap) {

        _pclose(adapterPipePair.second.monitorTsharkPipe);

        // 等待線程退出
        adapterPipePair.second.monitorThread->join();

        LOG_F(INFO, MiscUtil::UTF8ToANSIString("網卡：%s 流量監控已停止").c_str(), adapterPipePair.first.c_str());
    }

    adapterFlowTrendMonitorMap.clear();
    workStatus = STATUS_IDLE;
}

// 取得網路卡的流量，並存在flowTrendData回傳
void TsharkManager::getAdapterFlowTrendData(std::map<std::string, std::map<long, long>>& flowTrendData)
{
    long timeNow = static_cast<long>(time(nullptr));

    long startWindow = timeNow - adapterFlowTrendMonitorStartTime > 300 ? timeNow - 300 : adapterFlowTrendMonitorStartTime;
    long endWindow = timeNow - adapterFlowTrendMonitorStartTime > 300 ? timeNow : adapterFlowTrendMonitorStartTime + 300;

    adapterFlowTrendMapLock.lock();
    for (auto adapterPipePair : adapterFlowTrendMonitorMap) {
        flowTrendData.insert(std::make_pair<>(adapterPipePair.first, std::map<long, long>()));
        
        for (long t = startWindow; t <= endWindow; t++) {
            if (adapterPipePair.second.flowTrendData.find(t) != adapterPipePair.second.flowTrendData.end()) {
                flowTrendData[adapterPipePair.first][t] = adapterPipePair.second.flowTrendData.at(t);
            }
            else {
                flowTrendData[adapterPipePair.first][t] = 0;
            }
        }
    }

    adapterFlowTrendMapLock.unlock();
}

bool TsharkManager::getPacketDetailInfo(uint32_t frameNumber, std::string& result)
{
    std::string tmpFilePath = MiscUtil::getRandomString(10) + ".pcap";
    // 使用editccap來將特定的pcap取出
    std::string splitCmd = editcapPath + " -r " + currentFilePath + " " + tmpFilePath + " " + std::to_string(frameNumber) + "-" + std::to_string(frameNumber);
    if (!ProcessUtil::Exec(splitCmd)) {
        LOG_F(ERROR, "Error in executing command: %s", splitCmd.c_str());
        remove(tmpFilePath.c_str());
        return false;
    }
    
    // 透過tshark獲取指定數據包的詳細資料，輸出格式為XML
    std::string cmd = tsharkPath + " -r " + tmpFilePath + " -T pdml";
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(ProcessUtil::PopenEx(cmd), _pclose);
    if (!pipe) {
        std::cout << "Failed to run tshark command." << std::endl;
        remove(tmpFilePath.c_str());
        return false;
    }

    // 讀取tshark輸出
    char buffer[8192] = { 0 };
    std::string tsharkResult;
    // 對於較大的輸出，使用完全緩衝模式能提高效率
    setvbuf(pipe.get(), NULL, _IOFBF, sizeof(buffer));
    int count = 0;
    while (fgets(buffer, sizeof(buffer) - 1, pipe.get()) != nullptr) {
        tsharkResult += buffer;
        memset(buffer, 0, sizeof(buffer));
    }

    // remove 移除臨時檔案
    remove(tmpFilePath.c_str());

    // 將xml內容轉換為JSON
    rapidjson::Document detailJson;
    if (!MiscUtil::xml2JSON(tsharkResult, detailJson)) {
        LOG_F(ERROR, MiscUtil::UTF8ToANSIString("XML轉JSON失敗").c_str());
        return false;
    }

    // 字段翻譯
    Translator translator;
    translator.translateShowNameFields(detailJson["pdml"]["packet"][0]["proto"], detailJson.GetAllocator());

        
    // 序列化為JSON字符串
    rapidjson::StringBuffer stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
    detailJson.Accept(writer);

    result = stringBuffer.GetString();


    return true;
}

bool TsharkManager::getPacketDetailInfo(uint32_t frameNumber, rapidjson::Document& detailJson)
{
    std::string tmpFilePath = MiscUtil::getRandomString(10) + ".pcap";
    // 使用editccap來將特定的pcap取出
    std::string splitCmd = editcapPath + " -r " + currentFilePath + " " + tmpFilePath + " " + std::to_string(frameNumber) + "-" + std::to_string(frameNumber);
    if (!ProcessUtil::Exec(splitCmd)) {
        LOG_F(ERROR, "Error in executing command: %s", splitCmd.c_str());
        remove(tmpFilePath.c_str());
        return false;
    }

    // 透過tshark獲取指定數據包的詳細資料，輸出格式為XML
    std::string cmd = tsharkPath + " -r " + tmpFilePath + " -T pdml";
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(ProcessUtil::PopenEx(cmd), _pclose);
    if (!pipe) {
        std::cout << "Failed to run tshark command." << std::endl;
        remove(tmpFilePath.c_str());
        return false;
    }

    // 讀取tshark輸出
    char buffer[8192] = { 0 };
    std::string tsharkResult;
    // 對於較大的輸出，使用完全緩衝模式能提高效率
    setvbuf(pipe.get(), NULL, _IOFBF, sizeof(buffer));
    int count = 0;
    while (fgets(buffer, sizeof(buffer) - 1, pipe.get()) != nullptr) {
        tsharkResult += buffer;
        memset(buffer, 0, sizeof(buffer));
    }

    // remove 移除臨時檔案
    remove(tmpFilePath.c_str());

    // 將xml內容轉換為JSON
    if (!MiscUtil::xml2JSON(tsharkResult, detailJson)) {
        LOG_F(ERROR, MiscUtil::UTF8ToANSIString("XML轉JSON失敗").c_str());
        return false;
    }

    // 字段翻譯
    Translator translator;
    translator.translateShowNameFields(detailJson["pdml"]["packet"][0]["proto"], detailJson.GetAllocator());

    // 將原始十六進制數據插進去
    if (detailJson.HasMember("pdml") && detailJson["pdml"].HasMember("packet")) {
        std::string packetHex;
        std::vector<unsigned char> packetData;
        if (getPacketHexData(frameNumber, packetData)) {
            // 將原始數據轉換為16進制格式
            std::ostringstream oss;
            oss << std::hex << std::setfill('0');
            for (unsigned char ch : packetData) {
                oss << std::setw(2) << static_cast<int>(ch);
            }
            packetHex = oss.str();
        }

        detailJson["pdml"]["packet"][0].AddMember(
            "hexdata",
            rapidjson::Value().SetString(packetHex.c_str(), detailJson.GetAllocator()),
            detailJson.GetAllocator()
        );

        // 去掉外層的鍵值
        rapidjson::Value temp;
        temp.CopyFrom(detailJson["pdml"]["packet"][0], detailJson.GetAllocator());
        detailJson.SetObject();
        detailJson.CopyFrom(temp, detailJson.GetAllocator());
        return true;
    }
    return false;
}

void TsharkManager::queryPackets(QueryCondition& queryCondition, std::vector<std::shared_ptr<Packet>>& packets, int& total)
{
    // 函數轉發
    storage->queryPackets(queryCondition, packets, total);
}

bool TsharkManager::convertToPcap(const std::string inputFile, const std::string& outputFile)
{
    // 構建editcap命令，將pcapng轉換為pcap格式
    std::string command = editcapPath + " -F pcap " + inputFile + " " + outputFile;
    if (!ProcessUtil::Exec(command)) {
        LOG_F(ERROR, "Failed to convert to pcap format, command: %s", command.c_str());
        return false;
    }

    LOG_F(INFO, "Successfully converted %s to %s in pcap format", inputFile.c_str(), outputFile.c_str());
    return true;
}

WORK_STATUS TsharkManager::getWorkStatus()
{
    // 把workStatusLock這個鎖在lock的生命週期內鎖住，用於保障線程安全
    std::unique_lock<std::recursive_mutex> lock(workStatusLock);
    return workStatus;
}

void TsharkManager::reset()
{
    LOG_F(INFO, "reset called");

    // 如果還在抓包或分析流量，就將其停止
    if (workStatus == STATUS_CAPTURING) {
        stopCapture();
    }
    else if (workStatus == STATUS_MONITORING) {
        stopMonitorAdaptersFlowTrend();
    }

    workStatus = STATUS_IDLE;
    captureTsharkPid = 0;
    stopFlag = true;

    allPackets.clear();
    packetsTobeStore.clear();
    sessionSetTobeStore.clear();

    if (captureWorkThread) {
        captureWorkThread->join();
        captureWorkThread.reset();
    }
    if (storageThread) {
        storageThread->join();
        storageThread.reset();
    }

    // 刪除之前的數據，從新開始
    remove(currentFilePath.c_str());
    currentFilePath = "";

    // 重製數據庫
    storage.reset(); // 稀構舊的對象
    std::string dbFullPath = this->workDir + "\\mytshark.db";
    remove(dbFullPath.c_str());
    storage = std::make_shared<TsharkDatabase>(dbFullPath);

}

void TsharkManager::printAllSessions()
{
    for (auto& item : sessionMap) {
        rapidjson::Document doc(kObjectType);
		item.second->toJsonObj(doc, doc.GetAllocator());

		// 序列化JSON字符串
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
        MiscUtil::printLongString(buffer.GetString());
		//std::cout << buffer.GetString() << std::endl;
    }
}

void TsharkManager::querySessions(QueryCondition& queryCondition, std::vector<std::shared_ptr<Session>>& sessionList, int& total)
{
	storage->querySessions(queryCondition, sessionList, total);
}

bool TsharkManager::getIPStatsList(QueryCondition& queryCondition, std::vector<std::shared_ptr<IPStatsInfo>>& ipStatsList, int& total)
{
    return storage->queryIPStats(queryCondition, ipStatsList, total);
}

bool TsharkManager::getProtoStatsList(QueryCondition& queryCondition, std::vector<std::shared_ptr<ProtoStatsInfo>>& protoStatsList, int& total)
{
    return storage->queryProtoStats(queryCondition, protoStatsList, total);
}

DataStreamCountInfo TsharkManager::getSessionDataStream(uint32_t sessionId, std::vector<DataStreamItem>& dataStreamList)
{

    DataStreamCountInfo countInfo;
    if (sessionIdMap.find(sessionId) == sessionIdMap.end()) {
        LOG_F(ERROR, "session %d not found", sessionId);
        return countInfo;
    }

    std::shared_ptr<Session> session = sessionIdMap[sessionId];
    std::string transProto = session->trans_proto;

    //把協議名稱轉換成小寫
    std::transform(transProto.begin(), transProto.end(), transProto.begin(), ::tolower);

    // 四元組
    std::string fourTuple;
    if (session->ip1.find(":") != std::string::npos) {
        // IPv6的格式需要增加[]包起來
        fourTuple = "[" + session->ip1 + "]:" + std::to_string(session->ip1_port) + ",[" + session->ip2 + "]:" + std::to_string(session->ip2_port);
    }
    else {
        fourTuple = session->ip1 + ":" + std::to_string(session->ip1_port) + "," + session->ip2 + ":" + std::to_string(session->ip2_port);
    }

    // 準備tshark的命令
    std::string tsharkCmd = tsharkPath + " -r" + currentFilePath + " -q -z follow," + transProto + ",raw," + fourTuple;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(ProcessUtil::PopenEx(tsharkCmd.c_str()), _pclose);
    if (!pipe) {
        throw std::runtime_error("Failed to run tshark command.");
    }

    uint32_t maxItems = 500;
    std::vector<char> buffer(65535);
    bool dataStart = false;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string line(buffer.data());
        DataStreamItem item;

        line = MiscUtil::trimEnd(line);
        if (line.find("Node 0: ") == 0) {
            countInfo.node0 = line.substr(strlen("Node 0: "));
            continue;
        }
        if (line.find("Node 1: ") == 0) {
            countInfo.node1 = line.substr(strlen("Node 1: "));
            dataStart = true;
            continue;
        }

        if (!dataStart || line.find("=====") != std::string::npos) {
            continue;
        }

        if (line[0] == '\t') {
            item.hexData = line.substr(1);
            item.srcNode = countInfo.node1;
            item.dstNode = countInfo.node0;
            countInfo.node1PacketCount++;
            countInfo.node1BytesCount += (item.hexData.length() / 2);
        }
        else {
            item.hexData = line;
            item.srcNode = countInfo.node0;
            item.dstNode = countInfo.node1;
            countInfo.node0PacketCount++;
            countInfo.node0BytesCount += (item.hexData.length() / 2);
        }

        countInfo.totalPacketCount++;
        if (dataStreamList.size() < maxItems) {
            dataStreamList.push_back(item);
        }
    }

    return countInfo;
}
