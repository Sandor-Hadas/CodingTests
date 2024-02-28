#pragma once

#include <mutex>
#include <set>
#include <string>

class CDataTableElement {
public:
    CDataTableElement(double d1, double d2) : val1(d1), val2(d2) {}; // Freq+ / value

    bool operator<(const CDataTableElement& dataTableElement) const {
        if (val1 == dataTableElement.val1) { return val2 < dataTableElement.val2; }
        return val1 > dataTableElement.val1;
    }

    double val1;
    double val2;
};

class CDataTable {
public:
    void Init(const double& number);

    void AddToDataTable(const double& number);

    void PrintDataTable();

    // Only for tests
    std::set<CDataTableElement> GetTestableDataTable() { return dataTable; }

private:
    std::set<CDataTableElement> dataTable;
    std::string outString;
    std::mutex tableMutex;
};
