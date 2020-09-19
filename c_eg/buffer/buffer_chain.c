#ifndef buffer_chain_template_hpp
#define buffer_chain_template_hpp
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <c_eg/list.h>
#include <c_eg/buffer/contig_buffer.h>

typedef struct BufferChain_s {

        ListRef   m_chain;
        int       m_size;

} BufferChain, *BufferChainRef;
static void dealloc(void* p)
{
    CBuffer_free((CBufferRef)p);
}
BufferChainRef BufferChain_new()
{
    BufferChainRef tmp = malloc(sizeof(BufferChain));
    tmp->m_chain = List_new(dealloc);
    tmp->m_size = 0;
}
void BufferChain_free(BufferChainRef this)
{
    ListNodeRef iter = List_iterator(this->m_chain);
    for(;;) {
        if(iter == NULL) {
            break;
        }
        ListNodeRef next = List_itr_next(this->m_chain, iter);
        List_itr_remove(this->m_chain, &iter);
        iter = next;
    }
    free((void*)this);
}
void BufferChain_append(BufferChainRef this, void* buf, int len)
{
    if (this->m_size > 0) {
        CBufferRef last_cb = List_last(this->m_chain);
        if ((CBuffer_capacity(last_cb) - CBuffer_size(last_cb)) >= len) {
            CBuffer_append(last_cb, buf, len);
            this->m_size += len;
            return;
        }
    }
    int required_len = (len > 256*4*8) ? len+100 : 256*4*8;
    CBufferRef new_cb = CBuffer_new();
    CBuffer_append(new_cb, buf, len);
    List_add_back(this->m_chain, (void*)new_cb);
    this->m_size += len;
}
void BufferChain_append_cstr(BufferChainRef this, char* cstr)
{
    BufferChain_append(this, (void*)cstr, strlen(cstr));
}

void BufferChain_clear(BufferChainRef bchain)
{
    assert(false);
//    for(int i = 0; i < bchain->m_chain.size(); i++) {
//        ContifBuffer_free(bchain->mchain.at(i));
//    }
//    bchain->m_chain.clear();
//    bchain->m_size = 0;
}
int BufferChain_size(BufferChainRef this)
{
    return this->m_size;
}
#ifdef BVBVB
int BufferChain_blocks(BufferChainRef this)
{
    return m_chain.size();
}
ContigBuffer& BufferChain_block_at(BufferChainRef bchain, std::size_t index)
{
    if (index >= m_chain.size()) {
            MARVIN_THROW("index out of range");
    }
    return *(m_chain.at(index));
}
std::string BufferChain_to_string(BufferChainRef bchain)
{
    std::string s = "";
    for(CBuf::SPtr& mb : m_chain) {
        s += mb->toString();
    }
    return s;
}
CBuf::SPtr BufferChain_amalgamate(BufferChainRef bchain)
{
    CBuf::SPtr mb_final = m_buf_factory.makeSPtr(this->size());
    for(CBuf::SPtr& mb : m_chain) {
        mb_final->append(mb->data(), mb->size());
    }
    return mb_final;
}
#endif

#endif
