#pragma once
#include <map>
#include <iostream>

#define ERROR_SUCCESS           0
#define ERROR_PARAMETER_WRONG   1001
#define ERROR_INTERNAL_WRONG    1002
#define ERROR_DATABASE_WRONG    1003
#define ERROR_TSHARK_WRONG      1004
#define ERROR_STATUS_WRONG      1005
#define ERROR_FILE_TOOLARGE     1006
#define ERROR_FILE_NOTFOUND     1007
#define ERROR_FILE_SAVE_FAILED  1008

class TsharkError
{
public:
    static std::string getErrorMsg(int errorCode) {
        if (ERROR_MSG_MAP.find(errorCode) != ERROR_MSG_MAP.end()) {
            return ERROR_MSG_MAP[errorCode];
        }
        else {
            return "未知錯誤";
        }
    }

private:
	static std::map<int, std::string> ERROR_MSG_MAP;
};

std::map<int, std::string> TsharkError::ERROR_MSG_MAP = {
    {ERROR_SUCCESS, "操作成功"},
    {ERROR_PARAMETER_WRONG, "參數錯誤"},
    {ERROR_INTERNAL_WRONG, "內部錯誤"},
    {ERROR_DATABASE_WRONG, "數據庫錯誤"},
    {ERROR_TSHARK_WRONG, "tshark執行錯誤"},
    {ERROR_STATUS_WRONG, "狀態錯誤"},
    {ERROR_FILE_TOOLARGE, "文件太大了"},
    {ERROR_FILE_NOTFOUND, "文件不存在"}
};
