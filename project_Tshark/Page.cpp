#include "Page.hpp"

void PageAndOrder::reset()
{
    pageNum = 0;
    pageSize = 0;
    std::string orderBy = "";
    std::string descOrAsc = "";
}

PageAndOrder* PageHelper::getPageAndOrder()
{
    return &pageAndOrder;
}

std::string PageHelper::getPageSql()
{
    std::stringstream ss;
    if (!pageAndOrder.orderBy.empty()) {
        ss << " ORDER BY " << pageAndOrder.orderBy << " " << pageAndOrder.descOrAsc;
    }
    int offset = (pageAndOrder.pageNum - 1) * pageAndOrder.pageSize;
    ss << " LIMIT " << pageAndOrder.pageSize << " OFFSET " << offset << ";";

    return ss.str();
}

thread_local PageAndOrder PageHelper::pageAndOrder;

