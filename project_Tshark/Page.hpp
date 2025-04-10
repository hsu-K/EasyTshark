#pragma once
#include <iostream>
#include <sstream>
class PageAndOrder {
public:
    void reset();
    int pageNum = 0;
    int pageSize = 0;
    std::string orderBy = "";
    std::string descOrAsc = "";
};

class PageHelper {
public:

    static PageAndOrder* getPageAndOrder();

    static std::string getPageSql();

private:
    // 使用TLS來實現線程獨立
    static thread_local PageAndOrder pageAndOrder;
};
