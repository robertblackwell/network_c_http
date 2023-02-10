
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
* Simple demo of why Rust memory lifetime stuff might be valuable.
*
* This program runs without apparent problems but run under valgrind displays a evident memory corruption
* which can also be easily seen from the code.
*/

typedef struct Example_s {
    char    key[10];
    char    value[100];  
} Example;


char* a_bug_here(Example* ex_p)
{
    return &(ex_p->value[0]);
}

int main()
{
    Example* ex = malloc(sizeof(Example));
    memset(ex, 0, sizeof(Example));

    strncpy(ex->key, "1234567890", 10);
    strncpy(ex->value, "abcdefghijklmnopqrstuvwxyz", 26);
    char* cptr = a_bug_here(ex);
    free(ex);
    int len = strlen(cptr);
    printf("cptr len is : %d  %s\n", len, cptr);
    strcpy(cptr, "012345678901234567890123456");
    printf("cptr len is : %d %s\n", len, cptr);
    free(cptr);

}