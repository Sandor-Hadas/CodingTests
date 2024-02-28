#include "DataTable.h"

#include <iostream>
#include <sstream>

void CDataTable::Init(const double& number) {
    // std::lock_guard<std::mutex> lock(tableMutex);

    dataTable.clear();
    AddToDataTable(number);
}

void CDataTable::AddToDataTable(const double& number) {
    std::lock_guard<std::mutex> lock(tableMutex);

    outString = "";

    std::set<CDataTableElement>::iterator it = dataTable.end();
    for (it = dataTable.begin(); it != dataTable.end(); it++) {
        if (it->val2 == number) {
            double count = it->val1;
            dataTable.erase(it);
            dataTable.insert(CDataTableElement(count + 1, number));
            return;
        }
    }

    dataTable.insert(CDataTableElement(1, number));
}

void CDataTable::PrintDataTable() {
    std::lock_guard<std::mutex> lock(tableMutex);

    if (outString.length() == 0) {
        std::ostringstream out;
        out.precision(0);
        for (std::set<CDataTableElement>::iterator it = dataTable.begin(); it != dataTable.end(); it++) {
            out << std::fixed << it->val2 << ':' << it->val1 << ", ";
        }
        outString = out.str();
        if (outString.length() > 0) {
            outString = outString.substr(0, outString.length() - 2);
        }
    }

    std::cout << outString << std::endl;
}
