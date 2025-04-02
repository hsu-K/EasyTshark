#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include "tshark_datatype.h"
#include "loguru/loguru.hpp"
#include "QueryCondition.hpp"

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

		sql = ss.str();
		LOG_F(INFO, "[BUILD SQL]: %s", sql.c_str());
		return sql;
	}

};

