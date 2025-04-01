// httplib_test.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//
#include "httplib/httplib.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "loguru/loguru.hpp"
#include <iostream>

void hello(const httplib::Request& req, httplib::Response& res) {
    std::string name = req.get_param_value("name");
    std::string hello = "hello, " + name;
    res.set_content(hello, "text/plain");
}

void query_packet(const httplib::Request& req, httplib::Response& res) {

    // 解析傳入的JSON數據
    rapidjson::Document doc;
    if (doc.Parse(req.body.c_str()).HasParseError()) {
        res.status = 400;
        res.set_content("Invalid JSON format", "text/plain");
        return;
    }

    // 獲取JSON數據中的字段
    do {
        std::string ip;
        uint16_t port;

        // 讀取IP字段
        if (doc.HasMember("ip") && doc["ip"].IsString()) {
            ip = doc["ip"].GetString();
        }
        else {
            res.status = 400;
            res.set_content("Missing 'ip' field in JSON", "text/plain");
            break;
        }

        // 提取port字段
        if (doc.HasMember("port") && doc["port"].IsNumber()) {
            port = doc["port"].GetInt();
        }
        else{
            res.status = 400;
            res.set_content("Missing 'port' field in JSON", "text/plain");
            break;
        }

        // 準備返回數據，這邊是一條假數據
        rapidjson::Document response_doc;
        auto allocator = response_doc.GetAllocator();
        response_doc.SetObject();
        response_doc.AddMember("src_ip", rapidjson::Value(ip.c_str(), allocator), allocator);
        response_doc.AddMember("src_port", port, allocator);
        response_doc.AddMember("dst_ip", rapidjson::Value("127.0.0.1", allocator), allocator);
        response_doc.AddMember("dst_port", 5555, allocator);
        response_doc.AddMember("proto", rapidjson::Value("TCP", allocator), allocator);

        // 將JSON序列化為字符串
        rapidjson::StringBuffer stringBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
        response_doc.Accept(writer);

        res.set_content(stringBuffer.GetString(), "application/json");

    } while (false);
}

void user_info(const httplib::Request& req, httplib::Response& res) {
    std::string username = req.get_param_value("username");
    int age = std::stoi(req.get_param_value("age"));
    std::string secrect_token = "12345";
    std::string token = req.get_header_value("Authorization");
    
    // 確認Token
    if (token != "Bearer " + secrect_token) {
        res.status = 401;
        res.set_content("Unauthorized", "plain/text");
        return;
    }

    rapidjson::Document response_doc;
    auto allocator = response_doc.GetAllocator();
    response_doc.SetObject();
    response_doc.AddMember("status", rapidjson::Value("success", allocator), allocator);
    response_doc.AddMember("username", rapidjson::Value(username.c_str(), allocator), allocator);
    response_doc.AddMember("age", age, allocator);

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Hello, %s! You are %d years old.",username.c_str(), age);
    response_doc.AddMember("message", rapidjson::Value(buffer, allocator), allocator);

    // 將JSON序列化為字符串
    rapidjson::StringBuffer stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
    response_doc.Accept(writer);

    res.set_content(stringBuffer.GetString(), "application/json");
}

httplib::Server::HandlerResponse before_request(const httplib::Request& req, httplib::Response& res) {
    LOG_F(INFO, "Request received for %s", req.path.c_str());
    LOG_F(INFO, "Request send from %s", req.local_addr.c_str());
    return httplib::Server::HandlerResponse::Unhandled;
}

void after_response(const httplib::Request& req, httplib::Response& res) {
    LOG_F(INFO, "Received response with status %d", res.status);
}

int main(int argc, char* argv[])
{
    loguru::init(argc, argv);
    loguru::add_file("logs.txt", loguru::Truncate, loguru::Verbosity_MAX);


    // 創建一個 HTTP 服務器對象
    httplib::Server svr;
    
    // 設置鉤子函數
    svr.set_pre_routing_handler(before_request);
    svr.set_post_routing_handler(after_response);


    // 設置一個路由，當訪問根路徑時返回 "Hello World!"
    //svr.Get("/hello", [](const httplib::Request& req, httplib::Response& res) {
    //    res.set_content("Hello, World!", "text/plain");
    //});

    svr.Get("/hello", hello);

    svr.Post("/packet_query", query_packet);

    svr.Get("/user_info", user_info);

    // 啟動服務器，監聽8080端口
    svr.listen("127.0.0.1", 8080);

    return 0;
}
