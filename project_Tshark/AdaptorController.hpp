#pragma once
#include "BaseController.hpp"
#include "TsharkManager.h"
#include "tshark_datatype.h"
#include "TsharkError.hpp"
#include "MiscUtil.hpp"

class AdaptorController : public BaseController
{
public:
	AdaptorController(httplib::Server &server, std::shared_ptr<TsharkManager> tsharkManager)
		: BaseController(server, tsharkManager){ }

	virtual void registerRoute() {
        __server.Get("/api/getWorkStatus", [this](const httplib::Request& req, httplib::Response& res) {
            getWorkStatus(req, res);
            });

        __server.Post("/api/startCapture", [this](const httplib::Request& req, httplib::Response& res) {
            startCapture(req, res);
            });

        __server.Post("/api/stopCapture", [this](const httplib::Request& req, httplib::Response& res) {
            stopCapture(req, res);
            });

        __server.Get("/api/startMonitorAdaptersFlowTrend", [this](const httplib::Request& req, httplib::Response& res) {
            startMonitorAdaptersFlowTrend(req, res);
            });

        __server.Get("/api/stopMonitorAdaptersFlowTrend", [this](const httplib::Request& req, httplib::Response& res) {
            stopMonitorAdaptersFlowTrend(req, res);
            });

        __server.Get("/api/getAdaptersFlowTrendData", [this](const httplib::Request& req, httplib::Response& res) {
            getAdaptersFlowTrendData(req, res);
            });

        __server.Get("/api/getAdaptersList", [this](const httplib::Request& req, httplib::Response& res) {
            getAdaptersList(req, res);
            });
	}

    void getWorkStatus(const httplib::Request& req, httplib::Response& res) {
        try {
            WORK_STATUS workStatus = __tsharkManager->getWorkStatus();
            rapidjson::Document resDoc;
            rapidjson::Document::AllocatorType& allocator = resDoc.GetAllocator();
            resDoc.SetObject();
            resDoc.AddMember("workState", workStatus, allocator);
            sendJsonResponse(res, resDoc);
        }
        catch (const std::exception&) {
            sendErrorResponse(res, ERROR_INTERNAL_WRONG);
        }
    }

    void startCapture(const httplib::Request& req, httplib::Response& res) {
        try {
            // 先確認參數不為空
            if (req.body.empty()) {
                return sendErrorResponse(res, ERROR_PARAMETER_WRONG);
            }

            // 檢查現在的狀態是否可以抓包
            if (__tsharkManager->getWorkStatus() != STATUS_IDLE) {
                return sendErrorResponse(res, ERROR_STATUS_WRONG);
            }

            // 使用RapidJSON解析網路卡的名稱
            rapidjson::Document doc;
            if (doc.Parse(req.body.c_str()).HasParseError()) {
                return sendErrorResponse(res, ERROR_PARAMETER_WRONG);
            }

            if (!doc.HasMember("adapterName")) {
                return sendErrorResponse(res, ERROR_PARAMETER_WRONG);
            }
            std::string adapterName = doc["adapterName"].GetString();
            if (adapterName.empty()) {
                return sendErrorResponse(res, ERROR_PARAMETER_WRONG);
            }

            // 開始抓包
            if (__tsharkManager->startCapture(adapterName)) {
                
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

    void stopCapture(const httplib::Request& req, httplib::Response& res) {
        try {
            // 判斷是否在抓包，是的話就關閉
            if (__tsharkManager->getWorkStatus() == STATUS_CAPTURING) {
                __tsharkManager->stopCapture();
                sendSuccessResponse(res);
            }
            else {
                sendErrorResponse(res, ERROR_STATUS_WRONG);
            }
        }
        catch (const std::exception&) {
            sendErrorResponse(res, ERROR_INTERNAL_WRONG);
        }
    }

    void startMonitorAdaptersFlowTrend(const httplib::Request& req, httplib::Response& res) {
        try {
            // 先確認當前狀態
            if (__tsharkManager->getWorkStatus() == STATUS_IDLE) {
                __tsharkManager->startMonitorAdaptersFlowTrend();
                sendSuccessResponse(res);
            }
            else if (__tsharkManager->getWorkStatus() == STATUS_MONITORING) {
                sendSuccessResponse(res);
            }
            else {
                sendErrorResponse(res, ERROR_STATUS_WRONG);
            }
        }
        catch (const std::exception&) {
            sendErrorResponse(res, ERROR_INTERNAL_WRONG);
        }
    }

    void stopMonitorAdaptersFlowTrend(const httplib::Request& req, httplib::Response& res) {
        try {
            // 先確認當前狀態
            if (__tsharkManager->getWorkStatus() == STATUS_MONITORING) {
                __tsharkManager->stopMonitorAdaptersFlowTrend();
                sendSuccessResponse(res);
            }
            else {
                sendErrorResponse(res, ERROR_STATUS_WRONG);
            }
        }
        catch (const std::exception&) {
            sendErrorResponse(res, ERROR_INTERNAL_WRONG);
        }
    }

    void getAdaptersFlowTrendData(const httplib::Request& req, httplib::Response& res) {
        try {
            std::map<std::string, std::map<long, long>> flowTrendData;
            __tsharkManager->getAdapterFlowTrendData(flowTrendData);

            // 將flowTrendData轉換為json的形式
            rapidjson::Document resDoc;
            rapidjson::Document::AllocatorType& allocator = resDoc.GetAllocator();
            resDoc.SetObject();

            // 添加code 和 msg
            resDoc.AddMember("code", ERROR_SUCCESS, allocator);
            resDoc.AddMember("msg", rapidjson::Value(TsharkError::getErrorMsg(ERROR_SUCCESS).c_str(), allocator), allocator);

            // 構建data
            rapidjson::Value dataObject(rapidjson::kObjectType);
            for (const auto& adapterItem : flowTrendData) {
                rapidjson::Value adapterDataList(rapidjson::kArrayType);
                for (const auto& timeItem : adapterItem.second) {
                    rapidjson::Value timeObj(rapidjson::kObjectType);
                    timeObj.AddMember("time", (unsigned int)timeItem.second, allocator);
                    timeObj.AddMember("bytes", (unsigned int)timeItem.second, allocator);
                    adapterDataList.PushBack(timeObj, allocator);
                }

                dataObject.AddMember(rapidjson::StringRef(adapterItem.first.c_str()), adapterDataList, allocator);
            }

            resDoc.AddMember("data", dataObject, allocator);

            // 序列化為JSON字符串
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            resDoc.Accept(writer);

            res.set_content(buffer.GetString(), "application/json");

        }
        catch (const std::exception&) {
            sendErrorResponse(res, ERROR_INTERNAL_WRONG);
        }
    }

    void getAdaptersList(const httplib::Request& req, httplib::Response& res) {
        try {
            // 先確認當前狀態
            std::vector<AdapterInfo> adapterList = __tsharkManager->getNetworkAdapter();
            rapidjson::Document resDoc;
            rapidjson::Document::AllocatorType& allocator = resDoc.GetAllocator();
            resDoc.SetObject();

            // 添加code 和 msg
            resDoc.AddMember("code", ERROR_SUCCESS, allocator);
            resDoc.AddMember("msg", rapidjson::Value(TsharkError::getErrorMsg(ERROR_SUCCESS).c_str(), allocator), allocator);

            // 構建data
            rapidjson::Value adapterData(rapidjson::kArrayType);
            for (const auto& adapterItem : adapterList) {
                rapidjson::Value adapterObj(rapidjson::kObjectType);
                adapterObj.AddMember("id", adapterItem.id, allocator);
                adapterObj.AddMember("name", rapidjson::Value(adapterItem.name.c_str(), allocator), allocator);
                adapterObj.AddMember("remark", rapidjson::Value(MiscUtil::UTF8ToANSIString(adapterItem.remark).c_str(), allocator), allocator);
                adapterData.PushBack(adapterObj, allocator);
            }
            resDoc.AddMember("data", adapterData, allocator);

            // 序列化為JSON字符串
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            resDoc.Accept(writer);

            res.set_content(buffer.GetString(), "application/json");

        }
        catch (const std::exception&) {
            sendErrorResponse(res, ERROR_INTERNAL_WRONG);
        }
    }
};

