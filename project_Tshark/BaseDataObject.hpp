#pragma once
#include "rapidjson/document.h"

class BaseDataObject
{
public:
	virtual void toJsonObj(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const = 0;

};

