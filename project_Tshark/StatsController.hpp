#pragma once
#include "BaseController.hpp"
class StatsController : public BaseController
{
public:
    StatsController(httplib::Server& server, std::shared_ptr<TsharkManager> tsharkManager)
        :BaseController(server, tsharkManager)
    {
    }

    virtual void registerRoute() {
        __server.Post("/api/getIPStatsList", [this](const httplib::Request& req, httplib::Response& res) {
            getIPStatsList(req, res);
            });

        __server.Post("/api/getProtoStatsList", [this](const httplib::Request& req, httplib::Response& res) {
            getProtoStatsList(req, res);
            });
    }

    void getIPStatsList(const httplib::Request& req, httplib::Response& res) {
        try {
            auto queryParams = req.params;
            int pageNum = getIntParam(req, "pageNum", 1);
            int pageSize = getIntParam(req, "pageSize", 100);

            QueryCondition queryCondition;
            if (!parseQueryCondition(req, queryCondition)) {
                sendErrorResponse(res, ERROR_PARAMETER_WRONG);
                return;
            }

            std::vector<std::shared_ptr<IPStatsInfo>> ipStatsList;
            int total = 0;
            __tsharkManager->getIPStatsList(queryCondition, ipStatsList, total);
            sendDataList(res, ipStatsList, total);
        }
        catch (const std::exception&) {
            sendErrorResponse(res, ERROR_INTERNAL_WRONG);
        }
    }

    void getProtoStatsList(const httplib::Request& req, httplib::Response& res) {
        try {
            int pageNum = getIntParam(req, "pageNum", 1);
            int pageSize = getIntParam(req, "pageSize", 100);

            QueryCondition queryCondition;
            if (!parseQueryCondition(req, queryCondition)) {
                sendErrorResponse(res, ERROR_PARAMETER_WRONG);
                return;
            }

            std::vector<std::shared_ptr<ProtoStatsInfo>> protoStatsList;
            int total = 0;
            __tsharkManager->getProtoStatsList(queryCondition, protoStatsList, total);

            sendDataList(res, protoStatsList, total);
        }
        catch (const std::exception&) {
            sendErrorResponse(res, ERROR_INTERNAL_WRONG);
        }
    }
};

