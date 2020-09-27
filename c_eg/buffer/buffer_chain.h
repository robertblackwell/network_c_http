#ifndef buffer_chain_template_hpp
#define buffer_chain_template_hpp
#include <stddef.h>
#include <c_eg/buffer/cbuffer.h>

struct BufferChain_s;

typedef struct BufferChain_s BufferChain;

BufferChain* BufferChain_new();
void BufferChain_free(BufferChain** this);
void BufferChain_append(BufferChain* this, void* buf, int len);
void BufferChain_clear(BufferChain* this);
int BufferChain_size(BufferChain* this);
Cbuffer* BufferChain_compact(BufferChain* this);
bool BufferChain_eq_cstr(BufferChain* this, char* cstr);

#endif
