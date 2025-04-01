#pragma once
#include "ip2region/xdb_search.h"

#include <string>
#include <memory>

class IP2RegionUtil {
public:
	static bool init(const std::string& xdbFilePath);
	static std::string getIpLocation(const std::string& ip);

private:
	static std::string parseLocation(const std::string& input);
	static std::shared_ptr<xdb_search_t> xdbPtr;
};