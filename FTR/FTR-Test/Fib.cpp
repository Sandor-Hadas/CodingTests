#include "Fib.h"

#include <iostream>

void CFib::Init() {
    // Actually the first 1000 Fibonacci numbers are much bigger than any system int value
    // Therefore even the Test says that the input is an integer, it will have to be
    // treated as a floating number
    FibNumbers.clear();
    FibNumbers.emplace(0);
    FibNumbers.emplace(1); // Tricky because appears twice but we store just one
    FibNumbers.emplace(2); // The 4th number on 3rd position
    double last1 = 1;
    double last2 = 2;
    for (size_t i = 4; i < 1000; i++) {
        if (last1 < last2) {
            last1 += last2;
            FibNumbers.emplace(last1);
        }
        else {
            last2 += last1;
            FibNumbers.emplace(last2);
        }
    }
}

// This should print just 999 elements as 1 is not stored duplicated
void CFib::PrintFibNumbers() {
    std::cout << "--- Printing stored fibonacci numbers (1 only once): ---" << std::endl;
    size_t counter = 1;
    for (std::set<double>::iterator it = FibNumbers.begin(); it != FibNumbers.end(); it++) {
        std::cout << counter++ << ": " << *it << std::endl;
    }
    std::cout << "--- Printing done ---" << std::endl;
}

bool CFib::IsFibNumber(const double& number) {
    if (FibNumbers.count(number) > 0) {
        return true;
    }

    return false;
}

std::set<double> CFib::FibNumbers;
