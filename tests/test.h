#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;
static int test_current_failed = 0;

#define FAIL(msg) \
    do { \
        test_current_failed = 1; \
        printf("\n\x1b[31m[FAIL]\x1b[0m %s:%d: %s\n", __FILE__, __LINE__, (msg)); \
        return; \
    } while(0)

#define ASSERT_EQ(actual, expected) \
    do { \
        long long _act = (long long)(actual); \
        long long _exp = (long long)(expected); \
        if (_act != _exp) { \
            test_current_failed = 1; \
            printf("\n\x1b[31m[FAIL]\x1b[0m %s:%d: Expected %lld, got %lld\n", __FILE__, __LINE__, _exp, _act); \
            return; \
        } \
    } while(0)

#define ASSERT_NEQ(actual, expected) \
    do { \
        long long _act = (long long)(actual); \
        long long _exp = (long long)(expected); \
        if (_act == _exp) { \
            test_current_failed = 1; \
            printf("\n\x1b[31m[FAIL]\x1b[0m %s:%d: Expected values to differ, both are %lld\n", __FILE__, __LINE__, _act); \
            return; \
        } \
    } while(0)

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            test_current_failed = 1; \
            printf("\n\x1b[31m[FAIL]\x1b[0m %s:%d: Expected true: %s\n", __FILE__, __LINE__, #condition); \
            return; \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            test_current_failed = 1; \
            printf("\n\x1b[31m[FAIL]\x1b[0m %s:%d: Expected false: %s\n", __FILE__, __LINE__, #condition); \
            return; \
        } \
    } while(0)

#define ASSERT_NULL(actual) \
    do { \
        const void *_act = (const void *)(actual); \
        if (_act != NULL) { \
            test_current_failed = 1; \
            printf("\n\x1b[31m[FAIL]\x1b[0m %s:%d: Expected NULL, got %p\n", __FILE__, __LINE__, _act); \
            return; \
        } \
    } while(0)

#define ASSERT_NOT_NULL(actual) \
    do { \
        const void *_act = (const void *)(actual); \
        if (_act == NULL) { \
            test_current_failed = 1; \
            printf("\n\x1b[31m[FAIL]\x1b[0m %s:%d: Expected non-NULL pointer, got NULL\n", __FILE__, __LINE__); \
            return; \
        } \
    } while(0)

#define ASSERT_STR_EQ(actual, expected) \
    do { \
        const char *_act = (const char *)(actual); \
        const char *_exp = (const char *)(expected); \
        if (_act == NULL && _exp == NULL) { \
            /* both NULL -> considered equal */ \
        } else if (_act == NULL || _exp == NULL || strcmp(_act, _exp) != 0) { \
            test_current_failed = 1; \
            printf("\n\x1b[31m[FAIL]\x1b[0m %s:%d: Expected \"%s\", got \"%s\"\n", \
                   __FILE__, __LINE__, \
                   _exp ? _exp : "(null)", \
                   _act ? _act : "(null)"); \
            return; \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        tests_run++; \
        test_current_failed = 0; \
        printf("Running %s... ", #test_func); \
        fflush(stdout); \
        test_func(); \
        if (!test_current_failed) { \
            printf("\x1b[32m[OK]\x1b[0m\n"); \
            tests_passed++; \
        } \
    } while(0)

#define TEST_SUMMARY() \
    do { \
        if (tests_passed == tests_run) { \
            printf("\n\x1b[32m=== Test Summary: %d/%d Passed ===\x1b[0m\n\n", tests_passed, tests_run); \
        } else { \
            printf("\n\x1b[31m=== Test Summary: %d/%d Passed (%d Failed) ===\x1b[0m\n\n", \
                   tests_passed, tests_run, tests_run - tests_passed); \
        } \
    } while(0)

#endif
