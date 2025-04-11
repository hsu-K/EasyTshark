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
			// �Nreq�ѪR��queryCondition
			QueryCondition queryCondition;
			if (!parseQueryCondition(req, queryCondition)) {
				sendErrorResponse(res, ERROR_PARAMETER_WRONG);
				return;
			}

			// �ϥ�tsharkManager��querySessions�ӱq�ƾڮw��session
			std::vector<std::shared_ptr<Session>> sessionList;
			int total = 0;
			__tsharkManager->querySessions(queryCondition, sessionList, total);
			
			// �N��쪺session�ഫ��json���Φ���^��response
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

			// �ϥ�RapidJSON�ѪRJSON
			rapidjson::Document doc;
			if (doc.Parse(req.body.c_str()).HasParseError()) {
				return sendErrorResponse(res, ERROR_PARAMETER_WRONG);
			}

			if (!doc.IsObject()) {
				return sendErrorResponse(res, ERROR_PARAMETER_WRONG);
			}

			// �����ѼƦr��
			if (doc.HasMember("session_id") && doc["session_id"].IsNumber()) {
				sessionId = doc["session_id"].GetInt();
			}

			// �ե�tSharkManager����k����ƾ�
			std::vector<DataStreamItem> dataStreamList;
			DataStreamCountInfo countInfo = __tsharkManager->getSessionDataStream(sessionId, dataStreamList);

			// �ǳƭn��^��JSON�ƾ�
			rapidjson::Document resDoc;
			rapidjson::Document::AllocatorType& allocator = resDoc.GetAllocator();
			resDoc.SetObject();

			// �K�[code�Mmsg
			resDoc.AddMember("code", ERROR_SUCCESS, allocator);
			resDoc.AddMember("msg", rapidjson::Value(TsharkError::getErrorMsg(ERROR_SUCCESS).c_str(), allocator), allocator);

			// �K�["count"
			rapidjson::Value countObj(rapidjson::kObjectType);
			countInfo.toJsonObj(countObj, allocator);
			resDoc.AddMember("count", countObj, allocator);

			// �c�� data 
			rapidjson::Value dataArray(rapidjson::kArrayType);
			for (const auto& data : dataStreamList) {
				rapidjson::Value obj(rapidjson::kObjectType);
				data.toJsonObj(obj, allocator);
				assert(obj.IsObject());
				dataArray.PushBack(obj, allocator);
			}

			resDoc.AddMember("data", dataArray, allocator);

			// �ǦC�Ƭ� JSON �r�Ŧ�
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			resDoc.Accept(writer);

			// �]�m�T�����e
			res.set_content(buffer.GetString(), "application/json");

		}
		catch (const std::exception&) {
			sendErrorResponse(res, ERROR_INTERNAL_WRONG);
		}
	}
};

