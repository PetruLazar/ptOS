#pragma once
#include <iostream.h>

typedef bool (*testFunc)();

struct TestType
{
	const char* testName;
	testFunc testPtr;
	bool passed;
};

#define LIBC_STUB namespace std { ostream cout; }	\
namespace Memory { Heap *selectedHeap; }

#define ARRAYSIZE(array) (sizeof(array) / sizeof(array[0]))
#define TESTPTR(funname) test_##funname
#define MAKETEST(funname) { #funname, test_##funname, false }
#define TEST(funname) bool test_##funname()
#define TEST_INIT bool test_failed = false
#define TESTMODULE_END 	int failedCount = 0, passedCount = 0;													\
	std::cout << "\nTest results:";																				\
	for (int testIdx = 0; testIdx < ARRAYSIZE(tests); testIdx++) {												\
		std::cout << (tests[testIdx].passed ? "\nPASSED - " : "\nFAILED - ") << tests[testIdx].testName;		\
		if (tests[testIdx].passed) passedCount++;																\
		else failedCount++;																						\
	}																											\
	int passProcentage = passedCount * 10000 / (passedCount + failedCount);										\
	std::cout << "\nOverview: " << passedCount << '/' << (passedCount + failedCount) << " pass rate (" << (passProcentage / 100) << '.' << (passProcentage % 100 / 10) << (passProcentage % 10) << "%).\n\n";	\
	exit(failedCount > 0)
	
#define TEST_END return test_failed == false
#define DEFINE_TESTS TestType tests[] =
#define DEFINE_TESTCASES(inputs, outputs) struct test_case	\
	{														\
		struct inputType									\
			inputs in;										\
		struct outputType									\
			outputs out;									\
	};														\
	test_case test_cases[] = 

#define EXECUTE_ALL_TESTS for (int testIdx = 0; testIdx < ARRAYSIZE(tests); testIdx++) {	\
		std::cout << "Running \"" <<  tests[testIdx].testName << "\":";						\
		bool passed = tests[testIdx].testPtr();												\
																							\
		std::cout << (passed ? " PASSED\n" : "\nFAILED\n");									\
		tests[testIdx].passed = passed;														\
	}
#define FOREACH_TESTCASE for (int test_case_id = 0; test_case_id < ARRAYSIZE(test_cases); test_case_id++)
#define INPUT(inputvar) (test_cases[test_case_id].in.inputvar)
#define OUTPUT(outputvar) (test_cases[test_case_id].out.outputvar)
#define INPUTLIST
#define OUTPUTLIST
#define TESTCASELIST
#define MAKESTR_HELPER(obj) #obj
#define MAKESTR(obj) MAKESTR_HELPER(obj)
#define test_assert_expected(variable, expectedExperession, expectedValue) if (!(variable expectedExperession expectedValue)) { std::cout << "\n" __FILE__ ":" MAKESTR(__LINE__) ": Assertion failed for test case " << test_case_id << ": Observed value " << variable << " for variable " #variable ", but expected " #expectedExperession " " << expectedValue << "!"; test_failed = true;  }
#define test_assert_expected_named(variable, expectedExperession, expectedValue) if (!(variable expectedExperession expectedValue)) { std::cout << "\n" __FILE__ ":" MAKESTR(__LINE__) ": Assertion failed for test case " << test_case_id << ": Observed value " << variable << " for variable " #variable ", but expected " #expectedExperession " " #expectedValue "!"; test_failed = true;  }
#define test_assert(condition) if (!(condition)) { std::cout << "\n" __FILE__ ":" MAKESTR(__LINE__) ": Assertion failed for test case " << test_case_id << ":\"" #condition "\" evaluated to false!"; test_failed = true; }