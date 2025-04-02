#pragma once
#include <random>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <shlobj.h>
#include <rapidxml/rapidxml.hpp>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

using namespace rapidxml;
using namespace rapidjson;


class MiscUtil {
public:

#ifdef _WIN32
#include <windows.h>
	static string UTF8ToANSIString(const string& utf8Str) {
		int utf8Length = static_cast<int>(utf8Str.length());

		// 將UTF-8轉換為寬字節(UTF-16)
		int wideLength = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), utf8Length, nullptr, 0);

		wstring wideStr(wideLength, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), utf8Length, &wideStr[0], wideLength);

		// 將UTF-16轉換為ANSI
		int ansiLength = WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), wideLength, nullptr, 0, nullptr, nullptr);

		string ansiStr(ansiLength, '\0');
		WideCharToMultiByte(CP_ACP, 0, wideStr.c_str(), wideLength, &ansiStr[0], ansiLength, nullptr, nullptr);

		return ansiStr;
	}
#endif

	// 將string轉為Wstring
	static std::wstring stringToWString(const std::string& str) {
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstrTo[0], size_needed);
		return wstrTo;
	}

	static std::string getRandomString(size_t length) {
		const std::string chars = "abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"0123456789";
		std::random_device rd;
		std::mt19937 generator(rd());
		std::uniform_int_distribution<> distribution(0, static_cast<int>(chars.size() - 1));

		std::string randomString;
		for (size_t i = 0; i < length; ++i) {
			randomString += chars[distribution(generator)];
		}
		return randomString;
	}

	// 將XML轉為JSON格式
	static bool xml2JSON(std::string xmlContent, Document& outJsonDoc) {

		// 解析XML
		xml_document<> doc;
		try {
			doc.parse<0>(&xmlContent[0]);
		}
		catch (const rapidxml::parse_error& e) {
			std::cout << "XML Parsing error: " << e.what() << std::endl;
			return false;
		}

		// 創建JSON文檔
		outJsonDoc.SetObject();
		Document::AllocatorType& allocator = outJsonDoc.GetAllocator();

		// 獲取XML根節點
		xml_node<>* root = doc.first_node();
		if (root) {
			// 將根節點轉換為JSON
			Value root_json(kObjectType);
			xml_to_json_recursive(root_json, root, allocator);

			// 將根節點添加到JSON文檔
			outJsonDoc.AddMember(Value(MiscUtil::UTF8ToANSIString(root->name()).c_str(), allocator).Move(), root_json, allocator);
		}

		return true;
	}

	// 可以輸出較長的字串
	static void printLongString(const std::string& str, size_t chunkSize = 1024) {
		for (size_t i = 0; i < str.length(); i += chunkSize) {
			std::cout << str.substr(i, chunkSize);
		}
		std::cout << std::endl;
	}

    static bool fileExists(const std::string& filePath) {
		struct stat buffer;
		// stat 成功返回 0，表示檔案存在
		return (stat(filePath.c_str(), &buffer) == 0);
    }

	// 獲得數據儲存的資料夾
	static std::string getDefaultDataDir() {
		static std::string dir = "";
		if (!dir.empty()) {
			return dir;
		}
#ifdef _WIN32
		char* buffer = nullptr;
		size_t size = 0;
		if (_dupenv_s(&buffer, &size, "APPDATA") == 0 && buffer != nullptr){
			dir = std::string(buffer) + "\\easytshark\\";
			free(buffer);
		}
		
#else
		dir = std::string(std::getenv("HOME")) + "/easytshark/";
#endif 

		CreateDirectoryA(dir.c_str(), NULL);
		return dir;
	}
	
	static std::string getPcapNameByCurrentTimestamp(bool isFullPath = true) {
		// 獲取當前時間
		std::time_t nowTime = std::time(nullptr);
		// 轉換成tm格式
		std::tm localTime;
		localtime_s(&localTime , &nowTime);

		// 格式化文件名
		char buffer[64];
		std::strftime(buffer, sizeof(buffer), "easytshark_%Y-%m-%d_%H-%M-%S.pcap", &localTime);

		return isFullPath ? getDefaultDataDir() + std::string(buffer) : std::string(buffer);
	}

private:
	// 轉換成JSON時需要遞迴來處理子節點
	static void xml_to_json_recursive(Value& json, xml_node<>* node, Document::AllocatorType& allocator) {
		for (xml_node<>* cur_node = node->first_node(); cur_node; cur_node = cur_node->next_sibling()) {

			// 檢查是否要跳過節點，如果hide屬性值為true就跳過該節點
			xml_attribute<>* hide_attr = cur_node->first_attribute("hide");
			if (hide_attr && std::string(hide_attr->value()) == "yes") {
				continue;
			}

			// 檢查是否已經有該節點名稱的陣列
			Value* array = nullptr;
			if (json.HasMember(cur_node->name())) {
				array = &json[cur_node->name()];
			}
			else {
				Value node_array(kArrayType); // 創建新的陣列
				json.AddMember(Value(cur_node->name(), allocator).Move(), node_array, allocator);
				array = &json[cur_node->name()];
			}

			// 創建一個JSON對象代表當前節點
			Value child_json(kObjectType);

			// 處理節點的屬性
			for (xml_attribute<>* attr = cur_node->first_attribute(); attr; attr = attr->next_attribute()) {
				Value attr_name(attr->name(), allocator);
				Value attr_value(attr->value(), allocator);
				child_json.AddMember(attr_name, attr_value, allocator);
			}

			// 遞迴處理子節點
			xml_to_json_recursive(child_json, cur_node, allocator);

			// 將當前節點對象添加到對應陣列中
			array->PushBack(child_json, allocator);
		}
	}


};