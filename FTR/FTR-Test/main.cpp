#include <chrono>
#include <iostream>
#include <thread>

#include "DataTable.h"
#include "Fib.h"
#include "Worker.h"

int main() {
    CFib::Init();

    CWorker worker;

    std::thread thread_obj(&CWorker::ThreadRunner, &worker);//, 0);

    std::unique_lock<std::mutex> lck(worker.GetMutex());
    do {
        while (worker.GetCondVar().wait_for(lck, std::chrono::seconds(worker.GetOutputDelay())) == std::cv_status::timeout) {
            if (worker.GetCurrentState() == CWorker::INPUT_STATE_RUNNING) {
                worker.PrintDataTable();
            }
        }
    } while (worker.GetCurrentState() != CWorker::INPUT_STATE_QUIT);

    thread_obj.join();

    std::cout << "See you next time!" << std::endl;

    return 0;
}
