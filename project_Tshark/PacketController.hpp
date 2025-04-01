#pragma once
#include "httplib/httplib.h"
#include "TsharkManager.h"
#include "BaseController.hpp"
#include "tshark_datatype.h"

class PacketController : public BaseController
{
public:
	PacketController(httplib::Server& server, std::shared_ptr<TsharkManager> tsharkManager) 
		: BaseController(server, tsharkManager)
	{
    }

	void registerRoute() {
		__server.Post("/api/getPakcetList", [this](const httplib::Request& req, httplib::Response& res) {
			getPacketList(req, res);
		});
	}

	// 獲取數據包列表
	void getPacketList(const httplib::Request& req, httplib::Response& res) {
        
		// 獲取JSON數據中的字段
		try {
			QueryCondition queryCondition;
			if (!parseQueryCondition(req, queryCondition)) {
				sendErrorResponse(res, ERROR_PARAMETER_WRONG);
			}

			// 調用tsharkManger的方法獲取數據
			std::vector<std::shared_ptr<Packet>> packetList;
			__tsharkManager->queryPackets(queryCondition, packetList);
			sendDataList(res, packetList);
		}
		catch (const std::exception&) {
			sendErrorResponse(res, ERROR_INTERNAL_WRONG);
		}
	}

};

