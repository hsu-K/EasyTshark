#pragma once
#include <iostream>
#include "rapidjson/document.h"
#include "BaseDataObject.hpp"
using namespace std;


struct PcapHeader {
    uint32_t magic_number;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t network;
};

struct PacketHeader {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t caplen;
    uint32_t len;
};

class Packet : public BaseDataObject{
public:
    int frame_number = 0;
    double time = 0;
    string src_mac;
    string dst_mac;
    uint32_t cap_len = 0;
    uint32_t len = 0;
    string src_ip;
    uint16_t src_port = 0;
    string src_location;
    string dst_ip;
    uint16_t dst_port = 0;
    string dst_location;
	string trans_proto; // 傳輸協定
	string protocol;    // 這只會顯示最上層的協定，不一定會是傳輸層的
    string info;
    uint32_t file_offset = 0;
	uint32_t belong_session_id = 0; // 這個封包所屬的session id

    virtual void toJsonObj(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const {
        //rapidjson::Value pktObj(rapidjson::kObjectType);
        obj.AddMember("frame_number", frame_number, allocator);
        obj.AddMember("timestamp", time, allocator);
        obj.AddMember("src_mac", rapidjson::Value(src_mac.c_str(), allocator), allocator);
        obj.AddMember("dst_mac", rapidjson::Value(dst_mac.c_str(), allocator), allocator);
        obj.AddMember("src_ip", rapidjson::Value(src_ip.c_str(), allocator), allocator);
        obj.AddMember("src_location", rapidjson::Value(src_location.c_str(), allocator), allocator);
        obj.AddMember("src_port", src_port, allocator);
        obj.AddMember("dst_ip", rapidjson::Value(dst_ip.c_str(), allocator), allocator);
        obj.AddMember("dst_location", rapidjson::Value(dst_location.c_str(), allocator), allocator);
        obj.AddMember("dst_port", dst_port, allocator);
        obj.AddMember("trans_proto", rapidjson::Value(trans_proto.c_str(), allocator), allocator);
        obj.AddMember("len", len, allocator);
        obj.AddMember("cap_len", cap_len, allocator);
        obj.AddMember("protocol", rapidjson::Value(protocol.c_str(), allocator), allocator);
        obj.AddMember("info", rapidjson::Value(info.c_str(), allocator), allocator);
        obj.AddMember("file_offset", file_offset, allocator);
    }
};


struct AdapterInfo {
    int id;     // 前面的編號
    string name;// 中間的名稱
    string remark;// 括號裡的名稱
};