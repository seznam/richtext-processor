/*
   Inspired by check (https://libcheck.github.io/check/) and MinUnit
   (https://jera.com/techinfo/jtns/jtn002).
 */

#ifndef UNIT_HEADER_FILE
#define UNIT_HEADER_FILE 1

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define START_TEST(testName)\
static char * test_##testName(void);\
static char * test_##testName()\
{\

#define END_TEST\
  return NULL;\
}

#define assert(test, message)\
do {\
  char *_formattedMessage = unit_assert(__FILE__, __LINE__, (test), (message));\
  if (_formattedMessage != NULL) {\
    return _formattedMessage;\
  }\
} while (0)

#define runTestSuite(suiteFunction) unit_runTestSuite(suiteFunction)

#define runTest(testName) unit_runTest(#testName, test_##testName)

extern unsigned int tests_run;
extern unsigned int tests_failed;

/* Internal API */

void unit_runTestSuite(void testSuiteFunction(void));

void unit_runTest(char *testName, char *unitTest(void));

char *unit_assert(char *fileName, unsigned int lineOfCode, int test,
		  char *errorMessage);

#endif
