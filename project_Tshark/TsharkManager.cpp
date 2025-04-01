#include "TsharkManager.h"
#include <ctime>
#include <chrono>
#include <iomanip>
#include <set>
#include "loguru/loguru.hpp"
#include <io.h>
#include <fcntl.h>

#include "ProcessUtil.hpp"
#include "MiscUtil.hpp"
#include "Translator.hpp"

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
    IP2RegionUtil::init("third_library\\ip2region\\ip2region.xdb");
    tsharkPath = "C:/Reverse_tools/Wireshark/tshark";
    editcapPath = "C:/Reverse_tools/Wireshark/editcap";
    storage = std::make_shared<TsharkDatabase>("packet.db");
}

TsharkManager::~TsharkManager()
{
}

bool TsharkManager::analysisFile(string filePath, size_t& packetNum)
{
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
        
#ifdef _WIN32
        //packet->src_location = IP2RegionUtil::getIpLocation(packet->src_ip);
        packet->src_location = MiscUtil::UTF8ToANSIString(IP2RegionUtil::getIpLocation(packet->src_ip));
        packet->dst_location = MiscUtil::UTF8ToANSIString(IP2RegionUtil::getIpLocation(packet->dst_ip));
#else
        packet->src_location = IP2RegionUtil::getIpLocation(packet->src_ip);
        packet->dst_location = IP2RegionUtil::getIpLocation(packet->dst_ip);
#endif
        

        // 將分析的數據包保存起來
        processPacket(packet);
        //allPackets.insert(std::make_pair<>(packet->frame_number, packet));
    
    }

    LOG_F(INFO, MiscUtil::UTF8ToANSIString("分析完成，數據包總數: %d").c_str(), allPackets.size());
    packetNum = allPackets.size();

    stopFlag = true;

    _pclose(pipe);

    // 紀錄當前分析的文件路徑
    currentFilePath = filePath;

    return false;
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
    line = MiscUtil::UTF8ToANSIString(line);
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
    // 10: tcp.srcport
    // 11: udp.srcport
    // 12: tcp.dstport
    // 13: udp.dstport
    // 14: _ws.col.Protocol
    // 15: _ws.col.Info

    if (fields.size() >= 16) {
        packet->frame_number = std::stoi(fields[0]);
        packet->time = std::stod(fields[1]);
        packet->len = std::stoi(fields[2]);
        packet->cap_len = std::stoi(fields[3]);
        packet->src_mac = fields[4];
        packet->dst_mac = fields[5];
        packet->src_ip = fields[6].empty() ? fields[7] : fields[6];
        packet->dst_ip = fields[8].empty() ? fields[9] : fields[8];
        if (!fields[10].empty() || !fields[11].empty()) {
            packet->src_port = std::stoi(fields[10].empty() ? fields[11] : fields[10]);
        }

        if (!fields[12].empty() || !fields[13].empty()) {
            packet->dst_port = std::stoi(fields[12].empty() ? fields[13] : fields[12]);
        }
        packet->protocol = fields[14];
        packet->info = fields[15];
        return true;
    }
    else {
        return false;
    }
}

bool TsharkManager::startCapture(string adapterName)
{
    LOG_F(INFO, MiscUtil::UTF8ToANSIString("即將開始抓包，網路卡：%s").c_str(), adapterName.c_str());

    stopFlag = false;
    
    // 在開始抓包之前，先把儲存線程打開
    storageThread = std::make_shared<std::thread>(&TsharkManager::storageThreadEntry, this);

    // 利用智能指針來管理線程，將captureWorkThreadEntry作為入口函數進行執行
    captureWorkThread = std::make_shared<std::thread>(&TsharkManager::captureWorkThreadEntry, this, "\"" + adapterName + "\"");
    return false;
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


    return true;
}

void TsharkManager::captureWorkThreadEntry(string adapterName)
{
    string captureFile = "capture.pcap";
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
}

// 開始監控網卡流量，為每個網路卡開一個線程監控
void TsharkManager::startMonitorAdaptersFlowTrend()
{
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

    // 將xml內容轉換為JSON
    rapidjson::Document detailJson;
    if (!MiscUtil::xml2JSON(tsharkResult, detailJson)) {
        LOG_F(ERROR, MiscUtil::UTF8ToANSIString("XML轉JSON失敗").c_str());
        return false;
    }

    Translator translator;
    translator.translateShowNameFields(detailJson["pdml"]["packet"][0]["proto"], detailJson.GetAllocator());

        
    // 序列化為JSON字符串
    rapidjson::StringBuffer stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
    detailJson.Accept(writer);

    result = stringBuffer.GetString();


    // remove 移除臨時檔案
    remove(tmpFilePath.c_str());
    return true;
}

void TsharkManager::queryPackets(QueryCondition& queryCondition, std::vector<std::shared_ptr<Packet>>& packets)
{
    // 函數轉發
    storage->queryPackets(queryCondition, packets);
}
