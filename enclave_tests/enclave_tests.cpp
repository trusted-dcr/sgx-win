#include <iostream>
#include "stdafx.h"
#include "CppUnitTest.h"
#include "enclave_handle.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std::chrono;

namespace enclave_tests
{		
	TEST_CLASS(enclave_tests)
	{
	public:

    TEST_METHOD_INITIALIZE(setup) {
    
    }

    TEST_METHOD_CLEANUP(teardown) {
    }
		
		TEST_METHOD(TestMethod1)
		{
			// TODO: Your test code here
		}

	};
}