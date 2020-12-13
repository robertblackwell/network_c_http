#ifndef buffer_chain_template_hpp
#define buffer_chain_template_hpp
#include <stddef.h>
#include <c_http/api/iobuffer.h>
#include <c_http/dsl/list.h>

struct BufferChain_s;
typedef struct BufferChain_s* BufferChainRef;
typedef ListIter BufferChainIter;


BufferChainRef BufferChain_new();
void BufferChain_free(BufferChainRef* this);
void BufferChain_append(BufferChainRef this, void* buf, int len);
void BufferChain_append_IOBuffer(BufferChainRef this, IOBufferRef cbuf);
void BufferChain_append_cstr(BufferChainRef this, char* cstr);
void BufferChain_clear(BufferChainRef this);
int BufferChain_size(const BufferChainRef this);
IOBufferRef BufferChain_compact(const BufferChainRef this);
bool BufferChain_eq_cstr(const BufferChainRef this, char* cstr);

BufferChainIter BufferChain_iterator(BufferChainRef this);
BufferChainIter BufferChain_iter_next(BufferChainRef this, BufferChainIter iter);
IOBufferRef BufferChain_unpack_iter(BufferChainRef this, BufferChainIter iter);
void BufferChain_remove_iter(BufferChainRef this, BufferChainIter iter);

/*
 * too dangerous
 */
void BufferChain_append_bufferchain(BufferChainRef this, BufferChainRef other);
void BufferChain_steal_bufferchain(BufferChainRef this, BufferChainRef other);
void BufferChain_add_front(BufferChainRef this, IOBufferRef cbuf);
void BufferChain_add_back(BufferChainRef this, IOBufferRef cbuf);
/**/
IOBufferRef BufferChain_pop_front(BufferChainRef this);

#endif
