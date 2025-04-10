#pragma once
#include "httplib/httplib.h"
#include "TsharkManager.h"
#include "BaseController.hpp"
#include "tshark_datatype.h"
#include "TsharkError.hpp"
#include "MiscUtil.hpp"

class PacketController : public BaseController
{
public:
	PacketController(httplib::Server& server, std::shared_ptr<TsharkManager> tsharkManager) 
		: BaseController(server, tsharkManager)
	{
    }

	virtual void registerRoute() {
		__server.Post("/api/getPakcetList", [this](const httplib::Request& req, httplib::Response& res) {
			getPacketList(req, res);
		});
		__server.Post("/api/analysisFile", [this](const httplib::Request& req, httplib::Response& res) {
			analysisFile(req, res);
		});
	}

	// 獲取數據包列表
	void getPacketList(const httplib::Request& req, httplib::Response& res) {
        
		// 獲取JSON數據中的字段
		try {
			// 先解析有哪些查詢條件，並存進queryCondition
			QueryCondition queryCondition;
			if (!parseQueryCondition(req, queryCondition)) {
				sendErrorResponse(res, ERROR_PARAMETER_WRONG);
			}

			// 調用tsharkManger的方法獲取數據
			std::vector<std::shared_ptr<Packet>> packetList;
			// queryPackets查詢資料庫
			int total;
			__tsharkManager->queryPackets(queryCondition, packetList, total);
			// 以JSON格式回傳查詢到的數據包
			sendDataList(res, packetList, total);
		}
		catch (const std::exception&) {
			sendErrorResponse(res, ERROR_INTERNAL_WRONG);
		}
	}

	void analysisFile(const httplib::Request& req, httplib::Response& res) {
		try {
			if (req.body.empty()) {
				return sendErrorResponse(res, ERROR_PARAMETER_WRONG);
			}

			// 檢查當前狀況是否允許分析文件，避免現在正在做其他事情，造成衝突
			if (__tsharkManager->getWorkStatus() != STATUS_IDLE) {
				return sendErrorResponse(res, ERROR_STATUS_WRONG);
			}

			// 使用RapidJSON解析JSON
			rapidjson::Document doc;
			if (doc.Parse(req.body.c_str()).HasParseError()) {
				return sendErrorResponse(res, ERROR_PARAMETER_WRONG);
			}

			// 取得要分析的數據包的路徑
			std::string filePath = doc["filePath"].GetString();
			if (!MiscUtil::fileExists(filePath.c_str())) {
				return sendErrorResponse(res, ERROR_FILE_NOTFOUND);
			}

			// 開始分析
			if (__tsharkManager->analysisFile(filePath)) {
				sendSuccessResponse(res);
			}
			else {
				sendErrorResponse(res, ERROR_TSHARK_WRONG);
			}

		}
		catch (const std::exception&) {
			sendErrorResponse(res, ERROR_INTERNAL_WRONG);
		}
	}

};

