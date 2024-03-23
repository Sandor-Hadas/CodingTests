#include "DoublyLinkedList.h"

#include <string>
#include <vector>
#include <iostream>

bool testCompare(const std::string &received, const std::string &expected,
				 const size_t &numReceived, const size_t &numExpected,
				 const std::string &label) {
    if (received.compare(expected) == 0 && numReceived == numExpected) {
    	std::cout << "Test \"" << label.c_str() << "\" OK" << std::endl;
    	return true;
    }
    
    std::cerr << "Test \"" << label.c_str() << "\" FAILED" << std::endl;
    std::cerr << "Expected: " << expected.c_str()
              << " Received: " << received.c_str()
              << "  Expected num: " << numExpected
              << " Received num: " << numReceived << std:: endl;

    return false;
}

int main() {
	std::vector<std::string> testVec;
	testVec.push_back("Alpha");
	testVec.push_back("Beta");
	testVec.push_back("Gamma");
	testVec.push_back("Delta");
	testVec.push_back("Epsilon");
	testVec.push_back("Zeta");
	testVec.push_back("Eta");
	testVec.push_back("Theta");
	testVec.push_back("Iota");
	
	DoublyLinkedList llist;
	std::string stringTest;
	size_t countTest;
	bool anyError = false;

	llist.push_front((void*)testVec.at(0).c_str());
	stringTest = llist.createTestString();
	countTest = llist.getDataCount();
	if (!testCompare(stringTest, "Alpha", countTest, 1, "push_front one")) {
		anyError = true;
	}

	llist.clear();
	stringTest = llist.createTestString();
	countTest = llist.getDataCount();
	if (!testCompare(stringTest, "", countTest, 0, "clear")) {
		anyError = true;
	}
	
	llist.push_back((void*)testVec.at(0).c_str());
	stringTest = llist.createTestString();
	countTest = llist.getDataCount();
	if (!testCompare(stringTest, "Alpha", countTest, 1, "push_back one")) {
		anyError = true;
	}

	llist.clear();
	
	llist.push_front((void*)testVec.at(0).c_str());
	llist.push_front((void*)testVec.at(1).c_str());
	llist.push_front((void*)testVec.at(2).c_str());
	llist.push_front((void*)testVec.at(3).c_str());
	stringTest = llist.createTestString();
	countTest = llist.getDataCount();
	if (!testCompare(stringTest, "Delta,Gamma,Beta,Alpha", countTest, 4, "push_front four")) {
		anyError = true;
	}

	llist.clear();

	llist.push_back((void*)testVec.at(0).c_str());
	llist.push_back((void*)testVec.at(1).c_str());
	llist.push_back((void*)testVec.at(2).c_str());
	llist.push_back((void*)testVec.at(3).c_str());
	llist.push_back((void*)testVec.at(4).c_str());
	llist.push_back((void*)testVec.at(5).c_str());
	stringTest = llist.createTestString();
	countTest = llist.getDataCount();
	if (!testCompare(stringTest, "Alpha,Beta,Gamma,Delta,Epsilon,Zeta", countTest, 6, "push_back six")) {
		anyError = true;
	}

	stringTest = llist.createTestStringReverse();
	if (!testCompare(stringTest, "Zeta,Epsilon,Delta,Gamma,Beta,Alpha", countTest, 6, "reverse six")) {
		anyError = true;
	}

	llist.clear();
	llist.push_back((void*)testVec.at(0).c_str());
	llist.push_back((void*)testVec.at(0).c_str());
	llist.push_back((void*)testVec.at(0).c_str());
	countTest = llist.push_after((void*)testVec.at(1).c_str(), (void*)testVec.at(0).c_str(), 2);
	stringTest = llist.createTestString();
	if (!testCompare(stringTest, "Alpha,Beta,Alpha,Beta,Alpha", countTest, 2, "push_after")) {
		anyError = true;
	}

	countTest = llist.push_after((void*)testVec.at(2).c_str(), (void*)testVec.at(1).c_str(), 2);
	stringTest = llist.createTestString();
	if (!testCompare(stringTest, "Alpha,Beta,Gamma,Alpha,Beta,Gamma,Alpha", countTest, 2, "push_after 2")) {
		anyError = true;
	}

	countTest = llist.push_before((void*)testVec.at(3).c_str(), (void*)testVec.at(2).c_str(), 2);
	stringTest = llist.createTestString();
	if (!testCompare(stringTest, "Alpha,Beta,Delta,Gamma,Alpha,Beta,Delta,Gamma,Alpha", countTest, 2, "push_before")) {
		anyError = true;
	}

	if (anyError) {
		std::cerr << "Something needs a fix!" << std::endl;
		return -1;
	}
	
	return 0;
}
