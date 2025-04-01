// sqlite3_test.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//
#include "sqlite3/sqlite3.h"
#include <iostream>
#include <string>

int main(int  argc, char* argv[])
{
    sqlite3* db;
    char* errMessage = 0;

    // 打開數據庫，如果沒有數據庫文件就會創一個
    int rc = sqlite3_open("packet.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return (0);
    }
    else{
        std::cout << "Opened database sccessfully" << std::endl;
    }


    // 創建Table的SQL命令語句
    std::string createTableSQL = R"(
            CREATE TABLE IF NOT EXISTS t_packets (
                frame_number INTEGER PRIMARY KEY,
                src_ip TEXT,
                src_port INTEGER,
                dst_ip TEXT,
                dst_port INTEGER
            );
        )";

    // 執行SQL命令語句來創建Table
    rc = sqlite3_exec(db, createTableSQL.c_str(), nullptr, nullptr, &errMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMessage << std::endl;
        sqlite3_free(errMessage);
    }
    else {
        std::cout << "Table created successfully" << std::endl;
    }

    // 插入一條數據
    const char* insertSQL = "INSERT INTO t_packets (frame_number, src_ip, src_port, dst_ip, dst_port) VALUES (1, '192.168.1.1', 122345, '192.168.1.1', 80);";
    rc = sqlite3_exec(db, insertSQL, nullptr, 0, &errMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMessage << std::endl;
        sqlite3_free(errMessage);
    }
    else {
        std::cout << "Records created successfully" << std::endl;
    }

    sqlite3_close(db);

    return 0;
}
