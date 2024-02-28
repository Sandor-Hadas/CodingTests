#include "pch.h"
#include "CppUnitTest.h"

#include "../FTR-Test/DataTable.h"
#include "../FTR-Test/Fib.h"
#include "../FTR-Test/Worker.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SimpleTest
{
	TEST_CLASS(FibonacciTest)
	{
	public:
		TEST_METHOD(FibonacciNumbers_ValidData)
		{
			CFib::Init();

			Assert::IsTrue(CFib::IsFibNumber(0));
			Assert::IsTrue(CFib::IsFibNumber(1));
			Assert::IsTrue(CFib::IsFibNumber(2));
			Assert::IsTrue(CFib::IsFibNumber(3));
			Assert::IsTrue(CFib::IsFibNumber(5));
			Assert::IsTrue(CFib::IsFibNumber(8));
			Assert::IsTrue(CFib::IsFibNumber(13));
			Assert::IsTrue(CFib::IsFibNumber(21));
			Assert::IsTrue(CFib::IsFibNumber(832040));

			// A failing test:
			// Assert::IsFalse(CFib::IsFibNumber(0));
		}

		TEST_METHOD(FibonacciNumbers_InvalidData)
		{
			CFib::Init();

			Assert::IsFalse(CFib::IsFibNumber(4));
			Assert::IsFalse(CFib::IsFibNumber(832041));
			Assert::IsFalse(CFib::IsFibNumber(-1));
		}
	};

	TEST_CLASS(DataTableTest)
	{

		TEST_METHOD(Empty)
		{
			CDataTable t;
			std::set<CDataTableElement> data = t.GetTestableDataTable();

			Assert::IsTrue(data.empty());
		}

		TEST_METHOD(JustInitialized)
		{
			CDataTable t;
			t.Init(5);
			std::set<CDataTableElement> data = t.GetTestableDataTable();

			Assert::AreEqual((double)data.count(CDataTableElement(1, 5)), 1, 0.1);

		}

		TEST_METHOD(LittleLoad)
		{
			CDataTable t;
			t.Init(10293848576);
			for (size_t i = 0; i < 999; i++) {
				t.AddToDataTable(10293848576);
			}
			std::set<CDataTableElement> data = t.GetTestableDataTable();

			Assert::AreEqual((double)data.count(CDataTableElement(1000, 10293848576)), 1, 0.1);
		}

		TEST_METHOD(DataSorting)
		{
			CDataTable t;
			t.Init(5);
			t.AddToDataTable(5);
			t.AddToDataTable(7);
			t.AddToDataTable(3);
			t.AddToDataTable(3);
			t.AddToDataTable(7);
			t.AddToDataTable(7);
			t.AddToDataTable(3);
			t.AddToDataTable(3);
			t.AddToDataTable(7);
			t.AddToDataTable(9);
			t.AddToDataTable(1);
			t.AddToDataTable(3);
			t.AddToDataTable(3);

			std::set<CDataTableElement> data = t.GetTestableDataTable();

			// Should be: 6:3, 4:7, 2:5, 1:1, 1:9
			double expected[5][2] = { {6,3}, {4,7}, {2, 5}, {1,1}, {1,9} };
			size_t step = 0;
			for (std::set<CDataTableElement>::iterator it = data.begin(); it != data.end(); it++, step++) {
				Assert::AreEqual((double)data.count(CDataTableElement(expected[step][0], expected[step][1])), 1, 0.1);
			}
		}
	};
}
