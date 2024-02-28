#pragma once

#include <conio.h>
#include <iostream>
#include <climits>

#include "Worker.h"
#include "Fib.h"

CWorker::CWorker() {
    ReadDelayTime();
    ReadFirstEntry();
    currentState = INPUT_STATE_RUNNING;
}

void CWorker::ReadDelayTime() {
    outputDelay = 0;

    while (outputDelay < 1) {
        std::string tmpString = ReadInputLine("Please enter the output period", "0123456789");
        if (tmpString.length() > 0) {
            try {
                outputDelay = std::stoll(tmpString);
                if (outputDelay < 1) {
                    std::cerr << "Error, output delay must be > 0" << std::endl;
                }
            } catch (std::invalid_argument const& ex) {
                std::cout << "std::invalid_argument::what(): " << ex.what() << std::endl;
            } catch (std::out_of_range const& ex) {
                std::cerr << "Error, output delay must be <= " << LLONG_MAX << " exception: " << ex.what() << std::endl;
            }
        }
    }
    // std::cout << "Output delay: " << outputDelay << "s" << std::endl;
}

void CWorker::ReadFirstEntry() {
    double d;
    while (!ReadDoubleFromLine("Please enter the first number", d));
    dataTable.Init(d);
}

void CWorker::ThreadRunner() {
    States prevState = currentState;

    while (currentState != INPUT_STATE_QUIT) {
        if (currentState == INPUT_STATE_ENTRY) {
            ReadNewEntry();
            currentState = prevState;
        } else if (_kbhit()) {
            int c = _getch();

            bool needsNotification = false;
            if (currentState == INPUT_STATE_RUNNING) {
                if (c == 'p' || c == 'P') {
                    currentState = INPUT_STATE_PAUSED;
                } else if (c == 'n' || c == 'N') {
                    prevState = currentState;
                    currentState = INPUT_STATE_ENTRY;
                } else if (c == 'q' || c == 'Q') {
                    currentState = INPUT_STATE_QUIT;
                    needsNotification = true;
                }
            }
            else if (currentState == INPUT_STATE_PAUSED) {
                if (c == 'p' || c == 'P' || c == 'r' || c == 'R') {
                    currentState = INPUT_STATE_RUNNING;
                } else if (c == 'n' || c == 'N') {
                    prevState = currentState;
                    currentState = INPUT_STATE_ENTRY;
                } else if (c == 'q' || c == 'Q') {
                    currentState = INPUT_STATE_QUIT;
                    needsNotification = true;
                }
            }
            if (needsNotification) {
                workerCondVar.notify_one();
            }
        }
    }
}

void CWorker::PrintDataTable() {
    dataTable.PrintDataTable();
}

std::mutex& CWorker::GetMutex() {
    return workerMutex;
}

std::condition_variable& CWorker::GetCondVar() {
    return workerCondVar;
}

CWorker::States CWorker::GetCurrentState() const {
    return currentState;
}

long long CWorker::GetOutputDelay() const {
    return outputDelay;
}

std::string CWorker::ReadInputLine(const std::string& prompt, const std::string& allowedChars) {
    // Print what the user should enter
    std::string entered;
    std::cout << prompt << ": ";
    fflush(stdout);

    // Read user input and clear the input buffer
    std::cin >> entered;
    std::cin.ignore(std::cin.rdbuf()->in_avail());

    // Check input
    if (entered.find_first_not_of(allowedChars) != std::string::npos) {
        std::cerr << "Please enter only numeric characters (" << allowedChars << ")" << std::endl;
        return "";
    }
    return entered;
}

bool CWorker::ReadDoubleFromLine(const std::string& prompt, double& d) {
    std::string tmpString = ReadInputLine(prompt, "-0123456789");

    d = 0;
    try {
        d = std::stod(tmpString);
        if (CFib::IsFibNumber(d)) {
            std::cout << "FIB!" << std::endl;
        }
    } catch (...) {
        std::cerr << "Invalid input ignored" << std::endl;
        return false;
    }

    return true;
}

void CWorker::ReadNewEntry() {
    double d;
    if (ReadDoubleFromLine("Please enter the new number", d)) {
        dataTable.AddToDataTable(d);
    }
}
