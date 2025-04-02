#include <iostream>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <vector>

#include "loguru/loguru.hpp"
#include "httplib/httplib.h"
#include "tshark_datatype.h"
#include "ip2region_util.h"
#include "TsharkManager.h"
#include "QueryCondition.hpp"
#include "PacketController.hpp"
#include "AdaptorController.hpp"


#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

using namespace std;


#ifdef _WIN32
#include <windows.h>

#define popen _popen
#define pclose _pclose

#endif

std::shared_ptr<TsharkManager> g_ptrTsharkManager;

void hello(const httplib::Request& req, httplib::Response& res) {
    std::string name = req.get_param_value("name");
    std::string hello = "hello, " + name;
    res.set_content(hello, "text/plain");
}

httplib::Server::HandlerResponse before_request(const httplib::Request& req, httplib::Response& res) {
    LOG_F(INFO, "Request received for %s", req.path.c_str());
    LOG_F(INFO, "Request send from %s", req.local_addr.c_str());
    return httplib::Server::HandlerResponse::Unhandled;
}

void after_response(const httplib::Request& req, httplib::Response& res) {
    LOG_F(INFO, "Received response with status %d", res.status);
}


// 使用tshark -i \Device\NPF_{AFB90727-75CB-4D83-B303-E9734ACB117B} -c 100 -w capture.pcap -F pcap 取得數據包

//string packet_file = "C:\\Program_Code\\C++\\project_Tshark\\try_cat.pcap";
string packet_file = "C:\\Program_Code\\C++\\project_Tshark\\capture.pcap";
//string packet_file = "C:/Reverse_tools/Wireshark/http_traffic.pcap";
string TSHARK = "C:/Reverse_tools/Wireshark/tshark";


int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "zh_TW.UTF-8");
    loguru::init(argc, argv);
    loguru::add_file("logs.txt", loguru::Truncate, loguru::Verbosity_MAX);
    //loguru::g_stderr_verbosity = loguru::Verbosity_WARNING;

    //TsharkManager tsharkManager("");
    //tsharkManager.analysisFile(packet_file);

    //tsharkManager.startCapture("\\Device\\NPF_{AFB90727-75CB-4D83-B303-E9734ACB117B}");
    //string input;
    //while (true) {
    //    cout << MiscUtil::UTF8ToANSIString("請輸入q停止抓包");
    //    cin >> input;
    //    if (input == "q") {
    //        tsharkManager.stopCapture();
    //        break;
    //    }
    //}

    //tsharkManager.printAllPackets();


    /*
    tsharkManager.startMonitorAdaptersFlowTrend();

    // 睡眠十秒鐘，等待網卡數據
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::map<std::string, std::map<long, long>> trendData;
    tsharkManager.getAdapterFlowTrendData(trendData);

    tsharkManager.stopMonitorAdaptersFlowTrend();
    rapidjson::Document resDoc;
    rapidjson::Document::AllocatorType& allocator = resDoc.GetAllocator();
    resDoc.SetObject();
    rapidjson::Value dataObject(rapidjson::kObjectType);
    for (const auto& adapterItem : trendData) {
        rapidjson::Value adapterDataList(rapidjson::kArrayType);
        for (const auto& timeItem : adapterItem.second) {
            rapidjson::Value timeObj(rapidjson::kObjectType);
            timeObj.AddMember("time", (unsigned int)timeItem.first, allocator);
            timeObj.AddMember("bytes", (unsigned int)timeItem.second, allocator);
            adapterDataList.PushBack(timeObj, allocator);
        }

        dataObject.AddMember(rapidjson::StringRef(adapterItem.first.c_str()), adapterDataList, allocator);
    }
    resDoc.AddMember("data", dataObject, allocator);

    // 序列化為JSON字符串
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    resDoc.Accept(writer);

    LOG_F(INFO, UTF8ToANSIString("網卡流量監控數據：%s").c_str(), buffer.GetString());

    */


    // 讀取單個封包的詳細資料
    //string filePath;
    //cout << MiscUtil::UTF8ToANSIString("請輸入要讀取的檔案：");
    //cin >> filePath;
    //size_t Packet_num;
    //tsharkManager.analysisFile(filePath, Packet_num);
    //cout << MiscUtil::UTF8ToANSIString("請輸入獲取詳細數據包的編號(1 - ") << Packet_num << MiscUtil::UTF8ToANSIString(")：");
    //int frame_number;
    //cin >> frame_number;
    //string strrr;
    //tsharkManager.getPacketDetailInfo(frame_number, strrr);
    //MiscUtil::printLongString(strrr);
 
    g_ptrTsharkManager = std::make_shared<TsharkManager>("C:\\Program_Code\\C++\\project_Tshark\\project_Tshark");

    //g_ptrTsharkManager->startCapture("\\Device\\NPF_{AFB90727-75CB-4D83-B303-E9734ACB117B}");
    //string input;
    //while (true) {
    //    cout << MiscUtil::UTF8ToANSIString("請輸入q停止抓包");
    //    cin >> input;
    //    if (input == "q") {
    //        g_ptrTsharkManager->stopCapture();
    //        break;
    //    }
    //}

    //g_ptrTsharkManager->printAllPackets();

    // 離線分析
    //g_ptrTsharkManager->analysisFile(packet_file);
    
    //g_ptrTsharkManager->startCapture("\\Device\\NPF_{AFB90727-75CB-4D83-B303-E9734ACB117B}");
    //string input;
    //while (true) {
    //    cout << MiscUtil::UTF8ToANSIString("請輸入q停止抓包");
    //    cin >> input;
    //    if (input == "q") {
    //        g_ptrTsharkManager->stopCapture();
    //        break;
    //    }
    //}

    httplib::Server svr;
    
    svr.set_pre_routing_handler(before_request);
    svr.set_post_routing_handler(after_response);

    PacketController packetController(svr, g_ptrTsharkManager);
    packetController.registerRoute();

    AdaptorController adaptorController(svr, g_ptrTsharkManager);
    adaptorController.registerRoute();


    svr.listen("127.0.0.1", 8080);

    return 0;
}
