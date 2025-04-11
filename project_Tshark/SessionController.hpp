#pragma once
#include "BaseController.hpp"
class SessionController : public BaseController
{
public: SessionController(httplib::Server& server, std::shared_ptr<TsharkManager> tsharkManager)
	: BaseController(server, tsharkManager) {}

	virtual void registerRoute() {
		__server.Post("/api/getSessionList", [this](const httplib::Request& req, httplib::Response& res) {
			getSessionList(req, res);
		});

		__server.Post("/api/getSessionDataStream", [this](const httplib::Request& req, httplib::Response& res) {
			getSessionDataStream(req, res);
		});
	}

	void getSessionList(const httplib::Request& req, httplib::Response& res) {
		try {
			// 將req解析成queryCondition
			QueryCondition queryCondition;
			if (!parseQueryCondition(req, queryCondition)) {
				sendErrorResponse(res, ERROR_PARAMETER_WRONG);
				return;
			}

			// 使用tsharkManager的querySessions來從數據庫找session
			std::vector<std::shared_ptr<Session>> sessionList;
			int total = 0;
			__tsharkManager->querySessions(queryCondition, sessionList, total);
			
			// 將找到的session轉換成json的形式返回成response
			sendDataList(res, sessionList, total);
		}
		catch (const std::exception&) {
			sendErrorResponse(res, ERROR_INTERNAL_WRONG);
		}
	}


	void getSessionDataStream(const httplib::Request& req, httplib::Response& res) {
		try {
			uint32_t sessionId = 0;

			if (req.body.empty()) {
				return sendErrorResponse(res, ERROR_PARAMETER_WRONG);
			}

			// 使用RapidJSON解析JSON
			rapidjson::Document doc;
			if (doc.Parse(req.body.c_str()).HasParseError()) {
				return sendErrorResponse(res, ERROR_PARAMETER_WRONG);
			}

			if (!doc.IsObject()) {
				return sendErrorResponse(res, ERROR_PARAMETER_WRONG);
			}

			// 提取參數字串
			if (doc.HasMember("session_id") && doc["session_id"].IsNumber()) {
				sessionId = doc["session_id"].GetInt();
			}

			// 調用tSharkManager的方法獲取數據
			std::vector<DataStreamItem> dataStreamList;
			DataStreamCountInfo countInfo = __tsharkManager->getSessionDataStream(sessionId, dataStreamList);

			// 準備要返回的JSON數據
			rapidjson::Document resDoc;
			rapidjson::Document::AllocatorType& allocator = resDoc.GetAllocator();
			resDoc.SetObject();

			// 添加code和msg
			resDoc.AddMember("code", ERROR_SUCCESS, allocator);
			resDoc.AddMember("msg", rapidjson::Value(TsharkError::getErrorMsg(ERROR_SUCCESS).c_str(), allocator), allocator);

			// 添加"count"
			rapidjson::Value countObj(rapidjson::kObjectType);
			countInfo.toJsonObj(countObj, allocator);
			resDoc.AddMember("count", countObj, allocator);

			// 構建 data 
			rapidjson::Value dataArray(rapidjson::kArrayType);
			for (const auto& data : dataStreamList) {
				rapidjson::Value obj(rapidjson::kObjectType);
				data.toJsonObj(obj, allocator);
				assert(obj.IsObject());
				dataArray.PushBack(obj, allocator);
			}

			resDoc.AddMember("data", dataArray, allocator);

			// 序列化為 JSON 字符串
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			resDoc.Accept(writer);

			// 設置響應內容
			res.set_content(buffer.GetString(), "application/json");

		}
		catch (const std::exception&) {
			sendErrorResponse(res, ERROR_INTERNAL_WRONG);
		}
	}
};

