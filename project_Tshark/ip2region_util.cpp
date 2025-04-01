#include "ip2region_util.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

// 定義靜態成員變量
std::shared_ptr<xdb_search_t> IP2RegionUtil::xdbPtr;

bool IP2RegionUtil::init(const std::string& xdbFilePath)
{
	xdbPtr = std::make_shared<xdb_search_t>(xdbFilePath);
	xdbPtr->init_content();
    return false;
}

std::string IP2RegionUtil::getIpLocation(const std::string& ip)
{
	// if is IPv6, return empty string
	if (ip.size() > 15) {
		return "";
	}

	std::string location = xdbPtr->search(ip);
	if (!location.empty() && location.find("invalid") == std::string::npos) {
		return parseLocation(location);
	}
	else {
		return "";
	}
}

std::string IP2RegionUtil::parseLocation(const std::string& input)
{
	std::vector<std::string> tokens;
	std::string token;
	std::stringstream ss(input);
	if (input.find("内网") != std::string::npos) {
		return "內網";
	}

	while (std::getline(ss, token, '|')) {
		if (token == "中国") {
			token = "中國";
		}
		else if (token == "中华电信") {
			token = "中華電信";
		}
		else if (token == "台湾省") {
			//tokens.pop_back();

			token = "台灣";
		}
		else if (token == "桃园") {
			token = "桃園";
		}

		tokens.push_back(token);
	}

	if (tokens.size() >= 4) {
		std::string result;
		if (tokens[0].compare("0") != 0) {
			result.append(tokens[0]);
		}
		if (tokens[2].compare("0") != 0) {
			result.append("-" + tokens[2]);
		}
		if (tokens[3].compare("0") != 0) {
			result.append("-" + tokens[3]);
		}

		return result;
	}
	else {
		return input;
	}

}
