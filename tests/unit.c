#include <stdio.h>
#include "unit.h"

void unit_runTestSuite(testSuiteFunction)
void testSuiteFunction(void);
{
	testSuiteFunction();
	if (tests_failed > 0) {
		printf("TEST SUITE FAILED\n");
	} else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d \tSuccess: %d \tFailures: %d\n", tests_run,
	       tests_run - tests_failed, tests_failed);
}

void unit_runTest(testName, unitTest)
char *testName;
char *unitTest(void);
{
	char *message;
	printf("%s: ", testName);
	message = unitTest();
	tests_run++;
	if (message != NULL) {
		tests_failed++;
		printf("%s\n", message);
	} else {
		printf("OK\n");
	}
}

char *unit_assert(fileName, lineOfCode, test, errorMessage)
char *fileName;
unsigned int lineOfCode;
int test;
char *errorMessage;
{
	char *formattedMessage;
	if (!test) {
		formattedMessage =
		    malloc(sizeof(char) *
			   (strlen(errorMessage) + strlen(fileName) + 10 + 4 +
			    1));
		sprintf(formattedMessage, "%s:%u: %s", fileName, lineOfCode,
			errorMessage);
		return formattedMessage;
	}
	return NULL;
}
