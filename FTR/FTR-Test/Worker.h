#pragma once

#include <condition_variable>
#include <mutex>

#include "DataTable.h"

class CWorker {
public:
    enum States {
        INPUT_STATE_RUNNING,
        INPUT_STATE_PAUSED,
        INPUT_STATE_ENTRY,
        INPUT_STATE_QUIT
    };

    CWorker();
    void ReadDelayTime();
    void ReadFirstEntry();

    void ThreadRunner();

    void PrintDataTable();

    std::mutex& GetMutex();
    std::condition_variable& GetCondVar();
    States GetCurrentState() const;
    long long GetOutputDelay() const;

private:
    std::string ReadInputLine(const std::string& prompt, const std::string& allowedChars);
    bool ReadDoubleFromLine(const std::string& prompt, double& d);
    void ReadNewEntry();

    States currentState;

    std::mutex workerMutex;
    std::condition_variable workerCondVar;

    long long outputDelay;

    CDataTable dataTable;
};
