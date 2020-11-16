#ifndef c_http_operation_h
#define c_http_operation_h

#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>

typedef void (OpFunc(void*));

typedef struct Operation_s {
    void*   ctx;
    OpFunc* op;
} Operation;

Operation* Opr_new(OpFunc f, void* ctx);
void Opr_init(Operation* this, OpFunc f, void* ctx);
void Opr_free(Operation** this_ptr);

//Operation* Opr_new(void* f, void* ctx);
//void Opr_init(Operation* this, void*f, void* ctx);



#endif