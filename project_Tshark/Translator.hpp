#pragma once
#include <map>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "MiscUtil.hpp"
#include "ProcessUtil.hpp"

using namespace rapidjson;

class Translator
{
public:
	void translateShowNameFields(Value& value, Document::AllocatorType& allocator) {
		// 如果是Object就檢查和翻譯shownamey字段
		if (value.IsObject()) {
			if (value.HasMember("showname") && value["showname"].IsString()) {
				std::string showname = value["showname"].GetString();
				value["showname"].SetString(MiscUtil::UTF8ToANSIString(showname).c_str(), allocator);
				// 遍歷translationMap 查找靜態部分替換
				for (const auto& pair : translationMap) {
					const std::string& key = pair.first;
					const std::string& translate = pair.second;

					// 檢查字段A中是否包含translationMap中的Key
					if (showname.find(key) == 0) {
						showname.replace(0, key.length(), translate);
						value["showname"].SetString(MiscUtil::UTF8ToANSIString(showname).c_str(), allocator);
						break;
					}
				}
			}
			if (value.HasMember("show") && value["show"].IsString()) {
				std::string showname = value["show"].GetString();
				value["show"].SetString(MiscUtil::UTF8ToANSIString(showname).c_str(), allocator);
				// 遍歷translationMap 查找靜態部分替換
				for (const auto& pair : translationMap) {
					const std::string& key = pair.first;
					const std::string& translate = pair.second;

					// 檢查字段A中是否包含translationMap中的Key
					if (showname.find(key) == 0) {
						showname.replace(0, key.length(), translate);
						value["show"].SetString(MiscUtil::UTF8ToANSIString(showname).c_str(), allocator);
						break;
					}
				}
			}

			// 如果有field字段，遞迴處理
			if (value.HasMember("field") && value["field"].IsArray()) {
				Value& fieldArray = value["field"];
				for (auto& field : fieldArray.GetArray()) {
					translateShowNameFields(field, allocator);
				}
			}
		}
		// 如果是陣列，遞迴處理每個元素
		else if (value.IsArray()) {
			for (auto& item : value.GetArray()) {
				translateShowNameFields(item, allocator);
			}
		}
	}


private:
	std::map<std::string, std::string> translationMap = {
		{"General information", "一般資訊"},
		{"Frame Number", "影格編號"},
		{"Captured Length", "擷取長度"},
		{"Captured Time", "擷取時間"},
		{"Section number", "區段編號"},
		{"Interface id", "介面 ID"},
		{"Interface name", "介面名稱"},
		{"Encapsulation type", "封裝類型"},
		{"Arrival Time", "到達時間"},
		{"UTC Arrival Time", "UTC 到達時間"},
		{"Epoch Arrival Time", "時間戳記到達時間"},
		{"Time shift for this packet", "此封包的時間偏移"},
		{"Time delta from previous captured frame", "與前一擷取影格的時間差"},
		{"Time delta from previous displayed frame", "與前一顯示影格的時間差"},
		{"Time since reference or first frame", "自參考影格或第一影格起的時間"},
		{"Frame Number", "影格編號"},
		{"Frame Length", "影格Length"},
		{"Capture Length", "擷取Length"},
		{"Frame is marked", "影格已標記"},
		{"Frame is ignored", "影格已忽略"},
		{"Frame", "影格"},
		{"Protocols in frame", "影格內的通訊協定"},
		{"Ethernet II", "乙太網路 II"},
		{"Destination", "目標位址"},
		{"Address Resolution Protocol", "ARP 位址解析協定"},
		{"Address (resolved)", "位址 (解析後)"},
		{"Type", "類型"},
		{"Stream index", "串流索引"},
		{"Internet Protocol Version 4", "IPv4 網際網路通訊協定"},
		{"Internet Protocol Version 6", "IPv6 網際網路通訊協定"},
		{"Internet Control Message Protocol", "ICMP 網際網路控制訊息協定"},
		{"Version", "版本"},
		{"Header Length", "標頭長度"},
		{"Differentiated Services Field", "差異化服務欄位"},
		{"Total Length", "總長度"},
		{"Identification", "識別碼"},
		{"Flags", "旗標"},
		{"Time to Live", "存活時間 (TTL)"},
		{"Transmission Control Protocol", "TCP 傳輸控制協定"},
		{"User Datagram Protocol", "UDP 使用者資料包協定"},
		{"Domain Name System", "DNS 網域名稱系統"},
		{"Header Checksum", "標頭檢查碼"},
		{"Header checksum status", "標頭檢查碼狀態"},
		{"Source Address", "來源位址"},
		{"Destination Address", "目標位址"},
		{"Source Port", "來源連接埠"},
		{"Destination Port", "目標連接埠"},
		{"Next Sequence Number", "下一個序號"},
		{"Sequence Number", "序號"},
		{"Acknowledgment Number", "確認號碼"},
		{"Acknowledgment number", "確認號碼"},
		{"TCP Segment Len", "TCP 區段長度"},
		{"Conversation completeness", "會話完整性"},
		{"Window size scaling factor", "視窗大小縮放因子"},
		{"Calculated window size", "計算後的視窗大小"},
		{"Window", "視窗"},
		{"Urgent Pointer", "緊急指標"},
		{"Checksum:", "檢查碼:"},
		{"TCP Option - Maximum segment size", "TCP 選項 - 最大區段大小"},
		{"Kind", "種類"},
		{"MSS Value", "MSS 值"},
		{"TCP Option - Window scale", "TCP 選項 - 視窗縮放"},
		{"Shift count", "位移計數"},
		{"Multiplier", "倍數"},
		{"TCP Option - Timestamps", "TCP 選項 - 時間戳記"},
		{"TCP Option - SACK permitted", "TCP 選項 - 允許 SACK"},
		{"TCP Option - End of Option List", "TCP 選項 - 選項列表結束"},
		{"Options", "選項"},
		{"TCP Option - No-Operation", "TCP 選項 - 無操作"},
		{"Timestamps", "時間戳記"},
		{"Time since first frame in this TCP stream", "自此 TCP 串流第一影格起的時間"},
		{"Time since previous frame in this TCP stream", "與此 TCP 串流前一影格的時間差"},
		{"Protocol:", "通訊協定:"},
		{"Source:", "來源:"},
		{"Length:", "長度:"},
		{"Checksum status", "檢查碼狀態"},
		{"Checksum Status", "檢查碼狀態"},
		{"TCP payload", "TCP 負載"},
		{"UDP payload", "UDP 負載"},
		{"Hypertext Transfer Protocol", "HTTP 超文字傳輸協定"},
		{"Transport Layer Security", "TLS 傳輸層安全協定"}
	};
};

