#pragma once
#include <iostream>
#include "sqlite3/sqlite3.h"
#include "loguru/loguru.hpp"
#include "MiscUtil.hpp"
#include "tshark_datatype.h"
#include "QueryCondition.hpp"
#include "PacketSQL.hpp"

class TsharkDatabase
{
public:
    TsharkDatabase(const std::string &dbName) {
        if (sqlite3_open(dbName.c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error("Failed to open database");
        }

        createPacketTable();
    }

    ~TsharkDatabase() {
        if (db) {
            sqlite3_close(db);
        }
    }

	bool createPacketTable() {
		std::string createTableSQL = R"(
			CREATE TABLE IF NOT EXISTS t_packets (
				frame_number INTEGER PIRMARY KEY,
				time REAL,
				cap_len INTEGER,
				len INTEGER,
                src_mac TEXT,
                dst_mac TEXT,
                src_ip TEXT,
                src_location TEXT,
                src_port INTEGER,
                dst_ip TEXT,
                dst_location TEXT,
                dst_port INTEGER,
                protocol TEXT,
                info TEXT,
                file_offset INTEGER
			);		
        )";
        if (sqlite3_exec(db, createTableSQL.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
            LOG_F(ERROR, "Fail to create table t_packets");
            return false;
        }
	}

    bool storePackets(std::vector<std::shared_ptr<Packet>>& packets) {
        //  } Ҩư 
        sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

        // SQL   J y y
        std::string insertSQL = R"(
            INSERT INTO t_packets (
                frame_number, time, cap_len, len, src_mac, dst_mac, src_ip, src_location, src_port,
                dst_ip, dst_location, dst_port, protocol, info, file_offset
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
        )";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, insertSQL.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare insert statement");
        }

        //  M   C  ô  J ƾ 
        bool hasError = false;
        for (const auto& packet : packets) {
            sqlite3_bind_int(stmt, 1, packet->frame_number);
            sqlite3_bind_double(stmt, 2, packet->time);
            sqlite3_bind_int(stmt, 3, packet->cap_len);
            sqlite3_bind_int(stmt, 4, packet->len);
            sqlite3_bind_text(stmt, 5, packet->src_mac.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 6, packet->dst_mac.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 7, packet->src_ip.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 8, packet->src_location.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 9, packet->src_port);
            sqlite3_bind_text(stmt, 10, packet->dst_ip.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 11, packet->dst_location.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 12, packet->dst_port);
            sqlite3_bind_text(stmt, 13, packet->protocol.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 14, packet->info.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 15, packet->file_offset);
        
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                LOG_F(ERROR, "Failed to execute insert statement");
                hasError = true;
                break;
            }

            sqlite3_reset(stmt); //    s y y H K  U   j w
        }
        if (!hasError) {
            //     ưȡA ϥ COMMIT ~ @       g J
            if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
                hasError = true;
            }

            //     y y
            sqlite3_finalize(stmt);
        }

        return !hasError;
    }

    // 尋找資料庫的數據
    bool queryPackets(QueryCondition& queryCondition,std::vector<std::shared_ptr<Packet>>& packetList) {
        sqlite3_stmt* stmt = nullptr, * countStmt = nullptr;
        std::string sql = PacketSQL::buildPacketQuerySQL(queryCondition);
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            LOG_F(ERROR, "Failed to prepare statement: %s", sql.c_str());
            return false;
        }

        //sqlite3_bind_text(stmt, 1, queryCondition.ip.c_str(), -1, SQLITE_STATIC);
        //sqlite3_bind_text(stmt, 2, queryCondition.ip.c_str(), -1, SQLITE_STATIC);
        //sqlite3_bind_int(stmt, 3, queryCondition.port);
        //sqlite3_bind_int(stmt, 4, queryCondition.port);


        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::shared_ptr<Packet> packet = std::make_shared<Packet>();
            packet->frame_number = sqlite3_column_int(stmt, 0);
            packet->time = sqlite3_column_double(stmt, 1);
            packet->cap_len = sqlite3_column_int(stmt, 2);
            packet->len = sqlite3_column_int(stmt, 3);
            // 使用reinterpret_cast來將unsigned char* 強制轉換為 const char*
            packet->src_mac = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            packet->dst_mac = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            packet->src_ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            packet->src_location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            packet->src_port = sqlite3_column_int(stmt, 8);
            packet->dst_ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
            packet->dst_location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
            packet->dst_port = sqlite3_column_int(stmt, 11);
            packet->protocol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
            packet->info = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
            packet->file_offset = sqlite3_column_int(stmt, 14);
            packetList.push_back(packet);
        }
        sqlite3_finalize(stmt);
        return true;
    }
private:
    sqlite3* db = nullptr;
};

