#pragma once
#include "httplib/httplib.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "tshark_datatype.h"

class http_Backend
{
public:
    static void query_packet(const httplib::Request& req, httplib::Response& res) {

        // �ѪR�ǤJ��JSON�ƾ�
        rapidjson::Document doc;
        if (doc.Parse(req.body.c_str()).HasParseError()) {
            res.status = 400;
            res.set_content("Invalid JSON format", "text/plain");
            return;
        }

        // ���JSON�ƾڤ����r�q
        do {
            std::string ip;
            uint16_t port;

            // Ū��IP�r�q
            if (doc.HasMember("ip") && doc["ip"].IsString()) {
                ip = doc["ip"].GetString();
            }
            else {
                res.status = 400;
                res.set_content("Missing 'ip' field in JSON", "text/plain");
                break;
            }

            // ����port�r�q
            if (doc.HasMember("port") && doc["port"].IsNumber()) {
                port = doc["port"].GetInt();
            }
            else {
                res.status = 400;
                res.set_content("Missing 'port' field in JSON", "text/plain");
                break;
            }

            // �ǳƪ�^�ƾڡA�o��O�@�����ƾ�
            rapidjson::Document response_doc;
            auto allocator = response_doc.GetAllocator();
            response_doc.SetObject();
            response_doc.AddMember("src_ip", rapidjson::Value(ip.c_str(), allocator), allocator);
            response_doc.AddMember("src_port", port, allocator);
            response_doc.AddMember("dst_ip", rapidjson::Value("127.0.0.1", allocator), allocator);
            response_doc.AddMember("dst_port", 5555, allocator);
            response_doc.AddMember("proto", rapidjson::Value("TCP", allocator), allocator);

            // �NJSON�ǦC�Ƭ��r�Ŧ�
            rapidjson::StringBuffer stringBuffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
            response_doc.Accept(writer);

            res.set_content(stringBuffer.GetString(), "application/json");

        } while (false);
    }
};

