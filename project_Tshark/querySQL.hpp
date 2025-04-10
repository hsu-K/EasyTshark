#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include "tshark_datatype.h"
#include "loguru/loguru.hpp"
#include "QueryCondition.hpp"
#include "Page.hpp"


class PacketSQL
{
public:

	// 生成SQL的函數
	static std::string buildPacketQuerySQL(QueryCondition& condition) {
		std::string sql;
		std::stringstream ss;
		ss << "SELECT * FROM t_packets";

		std::vector<std::string> conditionList;

		if (!condition.ip.empty()) {
			char buf[100] = { 0 };
			snprintf(buf, sizeof(buf), "src_ip LIKE'%s' or dst_ip LIKE '%s'", condition.ip.c_str(), condition.ip.c_str());
			conditionList.push_back(buf);
		}
		if (condition.port != 0) {
			char buf[100] = { 0 };
			snprintf(buf, sizeof(buf), "src_port='%d' or dst_port='%d'", condition.port, condition.port);
			conditionList.push_back(buf);
		}
		if (!condition.proto.empty()) {
			char buf[100] = { 0 };
			snprintf(buf, sizeof(buf), "protocol='%s'", condition.proto.c_str());
			conditionList.push_back(buf);
		}
		if (!condition.mac_addr.empty()) {
			char buf[100] = { 0 };
			snprintf(buf, sizeof(buf), "src_mac='%s' or dst_mac='%s'", condition.mac_addr.c_str(), condition.mac_addr.c_str());
			conditionList.push_back(buf);
		}

		// 拼接WHERE條件
		if (!conditionList.empty()) {
			ss << " WHERE ";
			for (size_t i = 0; i < conditionList.size(); i++) {
				if (i > 0) {
					ss << " AND ";
				}
				ss << conditionList[i];
			}
		}

		ss << PageHelper::getPageSql();

		sql = ss.str();
		LOG_F(INFO, "[BUILD SQL]: %s", sql.c_str());
		return sql;
	}

	static std::string buildPacketQuerySQL_Count(QueryCondition& condition) {
		std::string sql = buildPacketQuerySQL(condition);
		// 用於計算全部的數量
		auto pos = sql.find("LIMIT");
		if (pos != std::string::npos) {
			sql = sql.substr(0, pos);
		}
		std::string countSql = "SELECT COUNT(0) FROM (" + sql + ") t_temp";
		LOG_F(INFO, "[BUILD SQL]: %s", countSql.c_str());
		return countSql;
	}
};

class SessionSQL {
public:
	static std::string buildSessionQuerySQL(QueryCondition& condition) {
		std::string sql;
		std::stringstream ss;
		ss << "SELECT * FROM t_sessions";
		std::vector<std::string> conditionList;
		if (!condition.proto.empty()) {
			char buf[100] = { 0 };
			snprintf(buf, sizeof(buf), "(app_proto like '%%%s%%' or trans_proto like '%%%s%%')", condition.proto.c_str(), condition.proto.c_str());
			conditionList.push_back(buf);
		}
		if (!condition.ip.empty()) {
			char buf[100] = { 0 };
			snprintf(buf, sizeof(buf), "(ip1='%s' or ip2='%s')", condition.ip.c_str(), condition.ip.c_str());
			conditionList.push_back(buf);
		}
		if (condition.port != 0) {
			char buf[100] = { 0 };
			snprintf(buf, sizeof(buf), "(ip1_port=%d or ip2_port=%d)", condition.port, condition.port);
			conditionList.push_back(buf);
		}
		if (condition.session_id != 0) {
			char buf[100] = { 0 };
			snprintf(buf, sizeof(buf), "(session_id=%d)", condition.session_id);
			conditionList.push_back(buf);
		}

		if (!conditionList.empty()) {
			ss << " WHERE ";
			for (size_t i = 0; i < conditionList.size(); i++) {
				if (i > 0) {
					ss << " AND ";
				}
				ss << conditionList[i];
			}
		}

		ss << PageHelper::getPageSql();

		sql = ss.str();
		LOG_F(INFO, "[BUILD SQL]: %s", sql.c_str());
		return sql;
	}

	static std::string buildSessionQuerySQL_Count(QueryCondition& condition) {
		std::string sql = buildSessionQuerySQL(condition);
		// 用於計算全部的數量
		auto pos = sql.find("LIMIT");
		if (pos != std::string::npos) {
			sql = sql.substr(0, pos);
		}
		std::string countSql = "SELECT COUNT(0) FROM (" + sql + ") t_temp";
		LOG_F(INFO, "[BUILD SQL]: %s", countSql.c_str());
		return countSql;
	}

};