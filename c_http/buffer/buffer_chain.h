#ifndef buffer_chain_template_hpp
#define buffer_chain_template_hpp
#include <stddef.h>
#include <c_http/buffer/cbuffer.h>
#include <c_http/list.h>

struct BufferChain_s;
typedef struct BufferChain_s* BufferChainRef;
typedef ListIter BufferChainIter;


BufferChainRef BufferChain_new();
void BufferChain_free(BufferChainRef* this);
void BufferChain_append(BufferChainRef this, void* buf, int len);
void BufferChain_append_cbuffer(BufferChainRef this, CbufferRef cbuf);
void BufferChain_append_cstr(BufferChainRef this, char* cstr);
void BufferChain_clear(BufferChainRef this);
int BufferChain_size(const BufferChainRef this);
CbufferRef BufferChain_compact(const BufferChainRef this);
bool BufferChain_eq_cstr(const BufferChainRef this, char* cstr);

BufferChainIter BufferChain_iterator(BufferChainRef this);
BufferChainIter BufferChain_iter_next(BufferChainRef this, BufferChainIter iter);
CbufferRef BufferChain_unpack_iter(BufferChainRef this, BufferChainIter iter);
void BufferChain_remove_iter(BufferChainRef this, BufferChainIter iter);

/*
 * too dangerous
 *
void BufferChain_add_front(BufferChainRef this, CbufferRef cbuf);
void BufferChain_add_back(BufferChainRef this, CbufferRef cbuf);
*/
CbufferRef BufferChain_pop_front(BufferChainRef this);

#endif
