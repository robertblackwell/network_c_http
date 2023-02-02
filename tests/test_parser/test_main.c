#define _GNU_SOURCE
#include "helper.h"

int main ()
{
    UT_ADD(test_requests);
    UT_ADD(test_responses);
    int rc = UT_RUN();
    return rc;
}