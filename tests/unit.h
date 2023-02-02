/*
   Inspired by check (https://libcheck.github.io/check/) and MinUnit
   (https://jera.com/techinfo/jtns/jtn002).
 */

#ifndef UNIT_HEADER_FILE
#define UNIT_HEADER_FILE 1

#include <stddef.h>
#include <stdio.h>

#define START_TEST(testName)\
static char * test_##testName(void);\
static char * test_##testName()\
{\

#define END_TEST\
  return NULL;\
}

#define assert(test, message)\
do {\
  if (!(test)) {\
    return message;\
  }\
} while (0)

#define runTestSuite(suiteFunction)\
do {\
  suiteFunction();\
  if (tests_failed > 0) {\
    printf("TEST SUITE FAILED\n");\
  } else {\
    printf("ALL TESTS PASSED\n");\
  } \
  printf("Tests run: %d \tSuccess: %d \tFailures: %d\n", tests_run, tests_run - tests_failed, tests_failed);\
} while(0)

#define run_test(testName)\
do {\
  char *message;\
	printf("%s: ", #testName);\
  message = test_##testName();\
  tests_run++;\
  if (message) {\
    tests_failed++; \
    printf("%s\n", message); \
  } else {\
    printf("OK\n"); \
  }\
} while (0)

extern unsigned int tests_run;
extern unsigned int tests_failed;

#endif
