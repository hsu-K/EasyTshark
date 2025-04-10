#pragma once
#include <iostream>

class QueryCondition
{
public:
	std::string ip;
	uint16_t port = 0;
	std::string proto;
	std::string mac_addr;
	uint32_t session_id = 0;
};

