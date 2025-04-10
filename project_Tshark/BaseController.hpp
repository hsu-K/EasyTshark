#pragma once
#include "httplib/httplib.h"
#include "TsharkManager.h"
#include "TsharkError.hpp"
#include <iostream>


class BaseController
{
public:
	BaseController(httplib::Server& server, std::shared_ptr<TsharkManager> tsharkManager)
		: __server(server)
		, __tsharkManager(tsharkManager){}
	
	// 構建虛函數，讓子類可以實例化
	virtual void registerRoute() = 0;
	
// 使用protected讓子類可以訪問
protected:
	httplib::Server& __server;
	std::shared_ptr<TsharkManager> __tsharkManager;

public:
	// 從URL中提取整數參數
	static int getIntParam(const httplib::Request& req, std::string paramName, int defaultValue) {
		int value = defaultValue;
		auto it = req.params.find(paramName);
		if (it != req.params.end()) {
			value = std::stoi(it->second);
		}
		return value;
	}

	// 提取字符串參數
	static std::string getStringParam(const httplib::Request& req, std::string paramName, std::string defaultValue = "") {
		std::string value = defaultValue;
		auto it = req.params.find(paramName);
		if (it != req.params.end()) {
			value = it->second;
		}
		return value;
	}

protected:
	// 使用模板的形式返回數據列表
	template<typename Data>
	void sendDataList(httplib::Response& res, std::vector<std::shared_ptr<Data>>& dataList, int total) {
		rapidjson::Document resDoc;
		rapidjson::Document::AllocatorType& allocator = resDoc.GetAllocator();
		resDoc.SetObject();

		resDoc.AddMember("code", ERROR_SUCCESS, allocator);
		resDoc.AddMember("msg", rapidjson::Value(TsharkError::getErrorMsg(ERROR_SUCCESS).c_str(), allocator), allocator);
		resDoc.AddMember("total", total, allocator);

		// 構建data陣列
		rapidjson::Value dataArray(rapidjson::kArrayType);
		for (const auto& data : dataList) {
			rapidjson::Value obj(rapidjson::kObjectType);
			data->toJsonObj(obj, allocator);
			assert(obj.IsObject());
			dataArray.PushBack(obj, allocator);
		}

		resDoc.AddMember("data", dataArray, allocator);

		// 序列化為JSON字符串
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		resDoc.Accept(writer);

		res.set_content(buffer.GetString(), "application/json");
	}

	// 返回JSON內容
	void sendJsonResponse(httplib::Response& res, rapidjson::Document& dataDoc) {
		rapidjson::Document resDoc;
		rapidjson::Document::AllocatorType& allocator = resDoc.GetAllocator();
		resDoc.SetObject();
		resDoc.AddMember("code", ERROR_SUCCESS, allocator);
		resDoc.AddMember("msg", rapidjson::Value(TsharkError::getErrorMsg(ERROR_SUCCESS).c_str(), allocator), allocator);
		resDoc.AddMember("data", dataDoc, allocator);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		resDoc.Accept(writer);

		res.set_content(buffer.GetString(), "application/json");
	
	}

	// 返回成功響應，但沒有數據
	void sendSuccessResponse(httplib::Response& res) {
		rapidjson::Document resDoc;
		rapidjson::Document::AllocatorType& allocator = resDoc.GetAllocator();
		resDoc.SetObject();

		resDoc.AddMember("code", ERROR_SUCCESS, allocator);
		resDoc.AddMember("msg", rapidjson::Value(TsharkError::getErrorMsg(ERROR_SUCCESS).c_str(), allocator), allocator);
	
		// 序列化為JSON字符串
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		resDoc.Accept(writer);

		res.set_content(buffer.GetString(), "application/json");
	}

	// 返回錯誤響應
	void sendErrorResponse(httplib::Response& res, int errorCode) {
		rapidjson::Document resDoc;
		rapidjson::Document::AllocatorType& allocator = resDoc.GetAllocator();
		resDoc.SetObject();

		resDoc.AddMember("code", errorCode, allocator);
		resDoc.AddMember("msg", rapidjson::Value(TsharkError::getErrorMsg(errorCode).c_str(), allocator), allocator);

		// 序列化為JSON字符串
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		resDoc.Accept(writer);

		res.set_content(buffer.GetString(), "application/json");
	}

	// 提取請求中的參數
	bool parseQueryCondition(const httplib::Request& req, QueryCondition& queryCondition) {
		try {

			// 檢查是否有body數據
			if (req.body.empty()) {
				throw std::runtime_error("Request body is empty");
			}

			// 使用RapidJSON解析JSON
			rapidjson::Document doc;
			if (doc.Parse(req.body.c_str()).HasParseError()) {
				throw std::runtime_error("Failed to parse JSON");
			}

			// 驗證是否是JSON Object
			if (!doc.IsObject()) {
				throw std::runtime_error("Invalid JSON format, expected an object");
			}

			// 提取字段並賦值到QueryCondition中
			if (doc.HasMember("ip") && doc["ip"].IsString()) {
				queryCondition.ip = doc["ip"].GetString();
				if (queryCondition.ip.back() == '*') {
					queryCondition.ip.back() = '%';
				}
			}

			if (doc.HasMember("port") && doc["port"].IsUint()) {
				queryCondition.port = static_cast<uint16_t>(doc["port"].GetUint());
			}

			if (doc.HasMember("proto") && doc["proto"].IsString()) {
				queryCondition.proto = doc["proto"].GetString();
			}

			if (doc.HasMember("mac_addr") && doc["mac_addr"].IsString()) {
				queryCondition.mac_addr = doc["mac_addr"].GetString();
			}

		}
		catch (std::exception&) {
			std::cout << "parse parameter error" << std::endl;
			return false;
		}

		return true;
	}

};

