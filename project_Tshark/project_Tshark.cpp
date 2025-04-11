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
#include "SessionController.hpp"
#include "StatsController.hpp"
#include "querySQL.hpp"
#include "Page.hpp"


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

httplib::Server::HandlerResponse before_request(const httplib::Request& req, httplib::Response& res) {
    LOG_F(INFO, "Request received for %s", req.path.c_str());

    // 提取分頁參數
    PageAndOrder* pageAndOrder = PageHelper::getPageAndOrder();
    pageAndOrder->pageNum = BaseController::getIntParam(req, "pageNum", 1);
    pageAndOrder->pageSize = BaseController::getIntParam(req, "pageSize", 100);
    pageAndOrder->orderBy = BaseController::getStringParam(req, "orderBy", "");
    pageAndOrder->descOrAsc = BaseController::getStringParam(req, "descOrAsc", "asc");

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

    std::vector<std::shared_ptr<BaseController>> controllerList;
    controllerList.push_back(std::make_shared<PacketController>(svr, g_ptrTsharkManager));
    controllerList.push_back(std::make_shared<AdaptorController>(svr, g_ptrTsharkManager));
    controllerList.push_back(std::make_shared<SessionController>(svr, g_ptrTsharkManager));
    controllerList.push_back(std::make_shared<StatsController>(svr, g_ptrTsharkManager));

    for (auto controller : controllerList) {
        controller->registerRoute();
    }

    svr.listen("127.0.0.1", 8080);
    
 //   g_ptrTsharkManager->analysisFile(packet_file);
 //   g_ptrTsharkManager->printAllSessions();
	//system("pause");

    return 0;
}
