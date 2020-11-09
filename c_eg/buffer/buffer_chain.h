#ifndef buffer_chain_template_hpp
#define buffer_chain_template_hpp
#include <stddef.h>
#include <c_eg/buffer/cbuffer.h>

struct BufferChain_s;
typedef struct BufferChain_s* BufferChainRef;

BufferChainRef BufferChain_new();
void BufferChain_free(BufferChainRef* this);
void BufferChain_append(BufferChainRef this, void* buf, int len);
void BufferChain_clear(BufferChainRef this);
int BufferChain_size(const BufferChainRef this);
CbufferRef BufferChain_compact(const BufferChainRef this);
bool BufferChain_eq_cstr(const BufferChainRef this, char* cstr);

#endif
