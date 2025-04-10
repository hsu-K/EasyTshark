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

};

