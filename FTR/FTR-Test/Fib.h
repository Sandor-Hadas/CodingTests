#pragma once

#include <set>

class CFib {
public:
    static void Init();
    static bool IsFibNumber(const double& number);

    // Test
    static void PrintFibNumbers();

private:
    static std::set<double> FibNumbers; // Generate fibonacci numbers in Init() and store in here
};
