
#include <c_http/unittest.h>

#include <assert.h>
#include <pthread.h>

#include <string.h>
#include <c_http/dsl/alloc.h>

#include <c_http/logger.h>

#define UT_MAX_TESTS 100
#define UT_MAX_ASSERTS 2000

pthread_mutex_t ut_lock;
static int utest_count = 0;
static UTObjectRef    utest_table[UT_MAX_TESTS];
static UTAssertResultRef ut_assert_table[UT_MAX_ASSERTS];
static int ut_assert_count = 0;
static int utest_error_count;

// make a testcase from a test function
// should only be run from the main thread
void UTRegister(UTObjectRef uto)
{
	if (utest_count == 0) {
	    pthread_mutex_init(&ut_lock, NULL);
		for(int i = 0; i < UT_MAX_TESTS; i++) {
			utest_table[i] = NULL;
		}
		for(int i = 0; i < UT_MAX_ASSERTS; i++) {
		    ut_assert_table[i] = NULL;
        }
	}

	utest_table[utest_count] = uto;
	utest_count++;

}
int UTRunOne(UTObjectRef utoref)
{
	int result = utoref->fn();
	utoref->passed = (result == 0);
	return result;
}
void UTReport()
{
    for(int i = 0; i < UT_MAX_TESTS; i++) {
        if(utest_table[i] != NULL) {
            printf("Test %s %s \n", utest_table[i]->name, (utest_table[i]->passed ? BRIGHT_GREEN("Passed") : BRIGHT_RED("Failed")));
        }
    }
    for(int i = 0; i < UT_MAX_ASSERTS; i++) {
        if(ut_assert_table[i] != NULL) {
            printf("%s", ut_assert_table[i]->msg);
        }
    }
}
int UTRun()
{
    bool all_passed = true;
	for(int i = 0; i < UT_MAX_TESTS; i++) {
	    UTObjectRef ref = utest_table[i];
		if(ref != NULL) {
		    int rc = UTRunOne(ref);
			all_passed = all_passed && (rc == 0);
		} 
	}
	UTReport();
	return (all_passed)? 0: 1;
}

void UTRecordAssertResult(const char* fn, const char* file, int line, const char* msg)
{
    // only asserts are to be running from multiple threads
    pthread_mutex_lock(&ut_lock);
    UTAssertResultRef arref = eg_alloc(sizeof(UTAssertResult));
    strcpy(arref->msg, msg);
    arref->fn_name = fn;
    arref->file_name = file;
    ut_assert_table[ut_assert_count] = arref;
    ut_assert_count++;
    pthread_mutex_unlock(&ut_lock);
}