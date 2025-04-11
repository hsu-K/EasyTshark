#pragma once
#include "loguru/loguru.hpp"
#include "MiscUtil.hpp"
#include "querySQL.hpp"
#include "QueryCondition.hpp"
#include "sqlite3/sqlite3.h"
#include "tshark_datatype.h"
#include "Session.hpp"
#include <iostream>
#include <unordered_set>
#include <map>


class TsharkDatabase
{
public:
    TsharkDatabase(const std::string& dbName) {
        remove(dbName.c_str());
        if (sqlite3_open(dbName.c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error("Failed to open database");
        }

        createPacketTable();
        createSessionTable();
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
        sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

        // insert SQL
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

        // sqlite3綁定
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

            sqlite3_reset(stmt);
        }
        if (!hasError) {

            if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
                hasError = true;
            }


            sqlite3_finalize(stmt);
        }

        return !hasError;
    }

    // 尋找資料庫的數據
    bool queryPackets(QueryCondition& queryCondition, std::vector<std::shared_ptr<Packet>>& packetList, int& total) {
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

        // 接著查詢總數
        sql = PacketSQL::buildPacketQuerySQL_Count(queryCondition);
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &countStmt, nullptr) != SQLITE_OK) {
            LOG_F(ERROR, "Failed to prepare statement: %s", sql.c_str());
            return false;
        }
        if (sqlite3_step(countStmt) == SQLITE_ROW) {
            total = sqlite3_column_int(countStmt, 0);
        }
        sqlite3_finalize(countStmt);

        return true;
    }

    // 創建Session的表
    void createSessionTable() {
        std::string createTableSQL = R"(
			CREATE TABLE IF NOT EXISTS t_sessions (
				session_id INTEGER PRIMARY KEY,
                ip1 TEXT,
                ip1_port INTEGER,
                ip1_location TEXT,
                ip2 TEXT,
                ip2_port INTEGER,
                ip2_location TEXT,
                trans_proto TEXT,
                app_proto TEXT,
                start_time REAL,
                end_time REAL,
                ip1_send_packets_count INTEGER,
                ip1_send_bytes_count INTEGER,
                ip2_send_packets_count INTEGER,
                ip2_send_bytes_count INTEGER,
                packet_count INTEGER,
                total_bytes INTEGER
			);
        )";
        if (sqlite3_exec(db, createTableSQL.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Fail to create table t_sessions");
        }

        // 清空表數據
        std::string clearTableSQL = "DELETE FROM t_sessions;";
        if (sqlite3_exec(db, clearTableSQL.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Fail to clear table t_sessions");
        }
    }

    // 儲存Session數據
    void storeAndUpdateSessions(std::unordered_set<std::shared_ptr<Session>>& sessions) {
        // 開啟事務
        sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

        // SQL UPSERT
        std::string upsertSQL = R"(
            INSERT INTO t_sessions (
                session_id, ip1, ip1_location, ip1_port, ip2, ip2_location, ip2_port,
                trans_proto, app_proto, start_time, end_time,
                ip1_send_packets_count, ip1_send_bytes_count, ip2_send_packets_count, ip2_send_bytes_count,
                packet_count, total_bytes
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ON CONFLICT(session_id) DO UPDATE SET
                trans_proto = excluded.trans_proto,
                app_proto = excluded.app_proto,
                start_time = excluded.start_time,
                end_time = excluded.end_time,
                ip1_send_packets_count = excluded.ip1_send_packets_count,
                ip1_send_bytes_count = excluded.ip1_send_bytes_count,
                ip2_send_packets_count = excluded.ip2_send_packets_count,
                ip2_send_bytes_count = excluded.ip2_send_bytes_count,
                packet_count = excluded.packet_count,
                total_bytes = excluded.total_bytes
        )";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, upsertSQL.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare insert statement");
        }

        for (const auto& session : sessions) {
            sqlite3_bind_int(stmt, 1, session->session_id);
            sqlite3_bind_text(stmt, 2, session->ip1.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, session->ip1_location.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 4, session->ip1_port);
            sqlite3_bind_text(stmt, 5, session->ip2.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 6, session->ip2_location.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 7, session->ip2_port);
            sqlite3_bind_text(stmt, 8, session->trans_proto.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 9, session->app_proto.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_double(stmt, 10, session->start_time);
            sqlite3_bind_double(stmt, 11, session->end_time);
            sqlite3_bind_int(stmt, 12, session->ip1_send_packets_count);
            sqlite3_bind_int(stmt, 13, session->ip1_send_bytes_count);
            sqlite3_bind_int(stmt, 14, session->ip2_send_packets_count);
            sqlite3_bind_int(stmt, 15, session->ip2_send_bytes_count);
            sqlite3_bind_int(stmt, 16, session->packet_count);
            sqlite3_bind_int(stmt, 17, session->total_bytes);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                throw std::runtime_error("Failed to execute UPSERT statement");
            }
            sqlite3_reset(stmt);
        }

        // 結束事務
        sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);

        // 釋放語句
		sqlite3_finalize(stmt);
    }

    bool querySessions(QueryCondition& queryCondition, std::vector<std::shared_ptr<Session>>& sessionList, int& total) {
        sqlite3_stmt *stmt = nullptr, *countStmt = nullptr;
		std::string sql = SessionSQL::buildSessionQuerySQL(queryCondition);
        
		if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            std::cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
			return false;
		}

        while (sqlite3_step(stmt) == SQLITE_ROW) {
			std::shared_ptr<Session> session = std::make_shared<Session>();
            session->session_id = sqlite3_column_int(stmt, 0);
            session->ip1 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            session->ip1_port = sqlite3_column_int(stmt, 2);
            session->ip1_location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            session->ip2 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            session->ip2_port = sqlite3_column_int(stmt, 5);
            session->ip2_location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            session->trans_proto = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            session->app_proto = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            session->start_time = sqlite3_column_double(stmt, 9);
            session->end_time = sqlite3_column_double(stmt, 10);
            session->ip1_send_packets_count = sqlite3_column_int(stmt, 11);
            session->ip1_send_bytes_count = sqlite3_column_int(stmt, 12);
            session->ip2_send_packets_count = sqlite3_column_int(stmt, 13);
            session->ip2_send_bytes_count = sqlite3_column_int(stmt, 14);
            session->packet_count = sqlite3_column_int(stmt, 15);
            session->total_bytes = sqlite3_column_int(stmt, 16);
        
			sessionList.push_back(session);
        }

		sqlite3_finalize(stmt);

        // 接著查詢總數
        sql = SessionSQL::buildSessionQuerySQL_Count(queryCondition);
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &countStmt, nullptr) != SQLITE_OK) {
            LOG_F(ERROR, "Failed to prepare statement: %s", sql.c_str());
            return false;
        }
        if (sqlite3_step(countStmt) == SQLITE_ROW) {
            total = sqlite3_column_int(countStmt, 0);
        }
        sqlite3_finalize(countStmt);
        return true;
    }

    bool queryIPStats(QueryCondition& queryCondition, std::vector<std::shared_ptr<IPStatsInfo>>& ipStatsList, int& total) {
        sqlite3_stmt* stmt = nullptr, * countStmt = nullptr;
        std::string sql = StatsSQL::buildIPStatsQuerySQL(queryCondition);

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            std::cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::shared_ptr<IPStatsInfo> ipStatsInfo = std::make_shared<IPStatsInfo>();
            ipStatsInfo->ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            ipStatsInfo->location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            ipStatsInfo->earliest_time = sqlite3_column_double(stmt, 2);
            ipStatsInfo->latest_time = sqlite3_column_double(stmt, 3);

            // 處理port
            std::string portsStr(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
            auto portVecsStr = MiscUtil::splitString(portsStr, ',');
            ipStatsInfo->ports = MiscUtil::toIntSet(portVecsStr);
        
            std::string transProtosStr(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
            std::string appProtosStr(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));
            ipStatsInfo->protocols = MiscUtil::splitString(transProtosStr + "," + appProtosStr, ',');

            ipStatsInfo->total_sent_packets = sqlite3_column_int(stmt, 7);
            ipStatsInfo->total_sent_bytes = sqlite3_column_int(stmt, 8);
            ipStatsInfo->total_recv_packets = sqlite3_column_int(stmt, 9);
            ipStatsInfo->total_recv_bytes = sqlite3_column_int(stmt, 10);
            ipStatsInfo->tcp_session_count = sqlite3_column_int(stmt, 11);
            ipStatsInfo->udp_session_count = sqlite3_column_int(stmt, 12);

            ipStatsList.push_back(ipStatsInfo);
        }

        sqlite3_finalize(stmt);

        // 接著查詢總數
        sql = StatsSQL::buildIPStatsQuerySQL_Count(queryCondition);
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &countStmt, nullptr) != SQLITE_OK) {
            LOG_F(ERROR, "Failed to prepare statement: %s", sql.c_str());
            return false;
        }
        if (sqlite3_step(countStmt) == SQLITE_ROW) {
            total = sqlite3_column_int(countStmt, 0);
        }
        sqlite3_finalize(countStmt);
        return true;
    }

    bool queryProtoStats(QueryCondition& queryCondition, std::vector<std::shared_ptr<ProtoStatsInfo>>& protoStatsList, int& total) {
        sqlite3_stmt* stmt = nullptr, * countStmt = nullptr;
        std::string sql = StatsSQL::buildProtoStatsQuerySQL(queryCondition);

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            std::cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::shared_ptr<ProtoStatsInfo> protoStatsInfo = std::make_shared<ProtoStatsInfo>();
            protoStatsInfo->proto = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            protoStatsInfo->total_packet = sqlite3_column_int(stmt, 1);
            protoStatsInfo->total_bytes = sqlite3_column_int(stmt, 2);
            protoStatsInfo->session_count = sqlite3_column_int(stmt, 3);
            //protoStatsInfo->proto_description = "";
            protoStatsInfo->proto_description = protocolMap[protoStatsInfo->proto];
            
            protoStatsList.push_back(protoStatsInfo);
        }

        sqlite3_finalize(stmt);

        // 接著查詢總數
        sql = StatsSQL::buildProtoStatsQuerySQL_Count(queryCondition);
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &countStmt, nullptr) != SQLITE_OK) {
            LOG_F(ERROR, "Failed to prepare statement: %s", sql.c_str());
            return false;
        }
        if (sqlite3_step(countStmt) == SQLITE_ROW) {
            total = sqlite3_column_int(countStmt, 0);
        }
        sqlite3_finalize(countStmt);
        return true;
    }

private:
    sqlite3* db = nullptr;

    std::map<std::string, std::string> protocolMap = {
        {"TCP", "傳輸控制協定，建構在IP之上，提供可靠、有序、無錯誤的資料傳輸，並具備流量與壅塞控制能力。"},
        {"UDP", "使用者資料包協定，無連線導向，傳輸速度快但不保證可靠性，適用於即時應用與少量資料傳輸。"},
        {"HTTP", "超文本傳輸協定，常用於瀏覽器與伺服器之間的網頁資料傳輸，採請求-回應模式進行通訊。"},
        {"HTTPS", "加密版HTTP，透過TLS或SSL建立安全通道，保護傳輸資料的完整性與機密性，廣泛用於網站登入與支付。"},
        {"DNS", "網域名稱系統協定，用於將網域名稱解析為IP位址，支援正向與反向解析，維護網際網路主機命名體系。"},
        {"TLS", "傳輸層安全協定，為應用層資料提供加密與完整性驗證，通常用於HTTPS等情境以確保通訊安全。"},
        {"SSL", "安全通訊端層協定，TLS的前身，曾廣泛用於加密資料傳輸，現多被TLS取代但在部分場景中仍可見。"},
        {"ARP", "位址解析協定，用於在區域網路中透過IP位址取得對應的MAC位址，在乙太網環境下尤其重要。"},
        {"ICMP", "網際網路控制訊息協定，傳輸錯誤與控制資訊，例如ping與traceroute等診斷工具依賴其回應功能。"},
        {"DHCP", "動態主機設定協定，用於自動分配IP位址、閘道器、DNS等網路設定資訊，大幅簡化網路管理。"},
        {"FTP", "檔案傳輸協定，使用TCP作為底層協定，可於用戶端與伺服器間進行檔案的上傳與下載操作。"},
        {"SSH", "安全殼層協定，為遠端登入與其他網路服務提供安全加密通道，常用於取代Telnet進行安全操作。"},
        {"Telnet", "早期的遠端登入協定，缺乏加密，通訊內容為明文傳輸，已逐漸被SSH等更安全協定取代。"},
        {"SMTP", "簡易郵件傳輸協定，用於在郵件伺服器之間或郵件用戶端與伺服器之間傳送電子郵件。"},
        {"POP", "郵局協定，使用者從郵件伺服器擷取郵件後通常會將其本地化，常見版本為POP3，使用簡單且效率高。"},
        {"IMAP", "網際網路郵件存取協定，支援在伺服器端管理郵件，用戶端可與伺服器保持同步並操作多個資料夾。"},
        {"LDAP", "輕量型目錄存取協定，主要用於查詢與修改目錄服務資訊，常見於企業級用戶/權限管理系統。"},
        {"NTP", "網路時間協定，透過UDP實作的時間同步服務，確保各網路設備間時間一致性。"},
        {"SNMP", "簡易網路管理協定，用於集中監控與管理網路設備，常收集設備CPU、記憶體、介面流量等資訊。"},
        {"RIP", "路由資訊協定，距離向量路由協定之一，使用跳數作為度量值，適用於小規模網路環境。"},
        {"OSPF", "開放最短路徑優先協定，鏈路狀態路由協定之一，支援大型網路與區域劃分，收斂速度快。"},
        {"BGP", "邊界閘道協定，用於跨自治系統（AS）間的路由資訊交換，是網際網路的核心路由協定。"},
        {"PPTP", "點對點隧道協定，基於PPP封裝的資料隧道技術之一，常用於VPN連線但安全性較弱。"},
        {"L2TP", "第二層隧道協定，與IPSec結合使用時可提供更安全的VPN隧道，廣泛應用於遠端存取場景。"},
        {"GRE", "通用路由封裝協定，用於不同網路間封裝各類第三層協定，常見於隧道與VPN場景。"},
        {"IPsec", "IP安全協定套件，透過加密與認證保障IP層資料的機密性與完整性，多用於VPN部署與安全通訊。"},
        {"SCTP", "串流控制傳輸協定，面向訊息傳輸，支援多宿主與多串流，常用於電信及即時訊號傳輸場景。"},
        {"RTSP", "即時串流協定，適合控制多媒體串流傳輸，常與RTP/RTCP搭配使用，實現點播與直播功能。"},
        {"RTP", "即時傳輸協定，用於在網路上傳輸音影音訊流，搭配RTCP進行品質控制，常見於會議系統。"},
        {"RTCP", "即時傳輸控制協定，與RTP協作用於監控傳輸品質、統計QoS等，為串流媒體提供回饋機制。"},
        {"TFTP", "簡易檔案傳輸協定，基於UDP，通常用於網路設備間傳輸設定檔或PXE開機時下載映像檔。"},
        {"Gopher", "一種早期的文件檢索與發布協定，使用分層選單結構存取資訊，在WWW普及後逐漸式微。"},
        {"TLSv1", "TLS協定的早期版本，提供資料加密與完整性驗證，兼顧相容性與安全性。"},
        {"TLSv1.2", "TLS協定的常用版本，提供資料加密與完整性驗證，兼顧相容性與安全性。"},
        {"TLSv1.3", "TLS協定的最新主流版本，引入零RTT握手與更安全的加密套件，提升網路通訊效率與安全性。"},
        {"QUIC", "Google提出的基於UDP的傳輸協定，整合TLS加密，提升HTTP/3等應用在弱網路環境下的效能。"},
        {"RADIUS", "遠端用戶撥號認證服務，採UDP封裝，集中管理網路用戶的認證、授權與計費資訊。"},
        {"Diameter", "RADIUS的升級版，提供更豐富的訊息與擴充特性，常用於電信業者的計費、認證與策略控制。"},
        {"NetBIOS", "網路基本輸出入系統，為區域網路內電腦提供名稱解析與會話服務，常見於Windows網路。"},
        {"SMB", "伺服器訊息區塊協定，用於在Windows網路中共享檔案、印表機與其他資源，也稱CIFS。"},
        {"CIFS", "通用網際網路檔案系統，SMB協定的早期版本，主要用於遠端檔案存取與資源共享。"},
        {"Kerberos", "網路身份驗證協定，使用對稱金鑰與票證機制，為用戶端與伺服器間的通訊提供安全驗證。"},
        {"Syslog", "系統日誌協定，以UDP或TCP傳輸方式，將設備或系統的日誌訊息集中發送至日誌伺服器記錄。"},
        {"MQTT", "訊息佇列遙測傳輸協定，基於發佈/訂閱模型，適用於物聯網等低頻寬、高延遲或不穩定網路環境。"},
        {"CoAP", "受限應用協定，專為資源受限的物聯網設備設計，基於UDP並採用REST風格互動方式。"},
        {"AMQP", "進階訊息佇列協定，提供訊息中介功能，支援可靠訊息傳遞與靈活的路由機制。"},
        {"SOAP", "簡單物件存取協定，以XML格式封裝的遠端呼叫協定，曾被廣泛用於Web服務。"},
        {"WSDL", "Web服務描述語言，用於描述SOAP Web服務的介面、訊息格式與存取位址，基於XML定義。"},
        {"XML-RPC", "基於HTTP與XML的遠端程序呼叫協定，使用簡單的請求-回應模型，為早期Web服務通訊方式之一。"},
        {"JSON-RPC", "輕量級遠端程序呼叫協定，使用JSON格式封裝資料，基於HTTP或其他傳輸層實現跨平台呼叫。"},
        {"WebSocket", "全雙工通訊協定，基於HTTP握手，可於瀏覽器與伺服器間建立持續的雙向訊息通道。"},
        {"SPDY", "由Google提出的實驗性協定，優化HTTP傳輸效率，透過多路複用與壓縮降低網路延遲。"}

    };
};

