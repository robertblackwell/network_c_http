
#include <c_eg/unittest.h>

#include <assert.h>
#include <c_eg/logger.h>

#define UT_MAX_TESTS 100
#define UT_MAX_ASSERTS 2000
static int utest_count = 0;
static UTObjectRef    utest_table[UT_MAX_TESTS];
static UTAssertResultRef ut_assert_table[UT_MAX_ASSERTS];
static ut_assert_count = 0;
static int utest_error_count;

// make a testcase from a test function
void UTRegister(UTObjectRef uto)
{
	if (utest_count == 0) {
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
static int UTRunOne(UTObjectRef utoref)
{
	int result = utoref->fn();
	utoref->passed = (result ==0);
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
            printf(ut_assert_table[i]->msg);
        }
    }
}
void UTRun()
{
    bool all_passed = true;
	for(int i = 0; i < UT_MAX_TESTS; i++) {
		if(utest_table[i] != NULL) {
			all_passed = all_passed && (UTRunOne(utest_table[i]) != 0);
		} 
	}
	UTReport();
}

void UTRecordAssertResult(char* fn, char* file, int line, char* msg)
{
    UTAssertResultRef arref = malloc(sizeof(UTAssertResult));
    arref->msg = msg;
    arref->fn_name = fn;
    arref->file_name = file;
    ut_assert_table[ut_assert_count] = arref;
    ut_assert_count++;
}