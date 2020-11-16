#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <c_http/operation.h>


Operation* Opr_new(OpFunc f, void* ctx)
{
    Operation* tmp = malloc(sizeof(Operation));
    Opr_init(tmp, f, ctx);
    return tmp;
}
void Opr_init(Operation* this, OpFunc f, void* ctx)
{
    this->op = f; this->ctx = ctx;
}

//Operation* Opr_new(void* f, void* ctx)
//{
//    Operation* tmp = malloc(sizeof(Operation));
//    Opr_init(tmp, f, ctx);
//    return tmp;
//}
//void Opr_init(Operation* this, void* f, void* ctx)
//{
//    this->f = f; this->ctx = ctx;
//}

void Opr_free(Operation** this_ptr)
{
    free((void*) *this_ptr);
}

