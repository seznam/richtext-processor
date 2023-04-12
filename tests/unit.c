#include <stdio.h>
#include <stdlib.h>
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
		if (fileName == NULL) {
			return errorMessage;
		}

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

char *unit_assertUnsignedLongEquals(fileName, lineOfCode, valueDescription,
				    matchedValue, expectedValue)
char *fileName;
unsigned int lineOfCode;
char *valueDescription;
unsigned long matchedValue;
unsigned long expectedValue;
{
	char *messageFormat, *formattedMessage;
	if (matchedValue == expectedValue) {
		return NULL;
	}

	if (valueDescription == NULL) {
		valueDescription = "provided value";
	}
	messageFormat = "Expected the %s to be %lu, but was %lu";
	formattedMessage =
	    malloc(sizeof(char) *
		   (strlen(messageFormat) + strlen(valueDescription) +
		    20 + 20 + 1));
	sprintf(formattedMessage, messageFormat, valueDescription,
		expectedValue, matchedValue);
	return unit_assert(fileName, lineOfCode,
			   matchedValue == expectedValue, formattedMessage);
}

char *unit_assertCStringEquals(fileName, lineOfCode, valueDescription,
			       matchedValue, expectedValue)
char *fileName;
unsigned int lineOfCode;
char *valueDescription;
char *matchedValue;
char *expectedValue;
{
	char *messageFormat, *formattedMessage;
	if (strcmp(matchedValue, expectedValue) == 0) {
		return NULL;
	}

	if (valueDescription == NULL) {
		valueDescription = "provided value";
	}
	messageFormat = "Expected the %s to be %s, but was %s";
	formattedMessage =
	    malloc(sizeof(char) *
		   (strlen(messageFormat) + strlen(valueDescription) +
		    strlen(expectedValue) + strlen(matchedValue) + 1));
	sprintf(formattedMessage, messageFormat, valueDescription,
		expectedValue, matchedValue);
	return unit_assert(fileName, lineOfCode, 0, formattedMessage);
}
