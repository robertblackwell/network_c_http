#include <c_http/buffer/buffer_chain.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <c_http/alloc.h>
#include <c_http/list.h>
#include <c_http/buffer/cbuffer.h>

typedef struct BufferChain_s {
        ListRef   m_chain;
        int       m_size;
} BufferChain;

static void dealloc(void** p)
{
    Cbuffer_free((CbufferRef*)p);
}
BufferChainRef BufferChain_new()
{
    BufferChainRef tmp = eg_alloc(sizeof(BufferChain));
    tmp->m_chain = List_new(dealloc);
    tmp->m_size = 0;
}
void BufferChain_free(BufferChainRef* thisptr)
{
    BufferChainRef this = *thisptr;
    ListIterator iter = List_iterator(this->m_chain);
    for(;;) {
        if(iter == NULL) {
            break;
        }
        ListIterator next = List_itr_next(this->m_chain, iter);
        List_itr_remove(this->m_chain, &iter);
        iter = next;
    }
    *thisptr = NULL;
    free((void*)this);
}
void BufferChain_append(BufferChainRef this, void* buf, int len)
{
    if (this->m_size > 0) {
        CbufferRef last_cb = List_last(this->m_chain);
        if ((Cbuffer_capacity(last_cb) - Cbuffer_size(last_cb)) >= len) {
            Cbuffer_append(last_cb, buf, len);
            this->m_size += len;
            return;
        }
    }
    int required_len = (len > 256*4*8) ? len+100 : 256*4*8;
    CbufferRef new_cb = Cbuffer_new();
    Cbuffer_append(new_cb, buf, len);
    List_add_back(this->m_chain, (void*)new_cb);
    this->m_size += len;
}
void BufferChain_append_cbuffer(BufferChainRef this, CbufferRef cbuf)
{
    BufferChain_append(this, Cbuffer_data(cbuf), Cbuffer_size(cbuf));
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
int BufferChain_size(const BufferChainRef this)
{
    return this->m_size;
}

CbufferRef BufferChain_compact(const BufferChainRef this)
{
    CbufferRef cb_final = Cbuffer_new();
    if(cb_final == NULL)
        goto memerror_01;
    ListIterator iter = List_iterator(this->m_chain);
    while(iter != NULL) {
        CbufferRef tmp = (CbufferRef)List_itr_unpack(this->m_chain, iter);
        void* data = Cbuffer_data(tmp);
        int sz = Cbuffer_size(tmp);
        Cbuffer_append(cb_final, data, sz); /* MEM CHECK REQUIRED*/
        ListIterator next = List_itr_next(this->m_chain, iter);
        iter = next;
    }
    return cb_final;
    memerror_01:
        return NULL;
}
bool BufferChain_eq_cstr(const BufferChainRef this, char* cstr)
{
    ListIterator iter = List_iterator(this->m_chain);
    int cstr_index = 0;
    while(iter != NULL) {
        CbufferRef tmp = (CbufferRef)List_itr_unpack(this->m_chain, iter);
        char* data = Cbuffer_data(tmp);
        int sz = Cbuffer_size(tmp);
        int l = strlen(cstr);
        for(int cb_index = 0; cb_index < sz; cb_index++) {
            if(cstr[cstr_index] != data[cb_index]) {
                return false;
            } else {
                cstr_index++;
            }
        }
        ListIterator next = List_itr_next(this->m_chain, iter);
        iter = next;
    }
    return true;
}

BufferChainIter BufferChain_iterator(BufferChainRef this)
{
    return List_iterator(this->m_chain);
}
BufferChainIter BufferChain_iter_next(BufferChainRef this, BufferChainIter iter)
{
    return List_itr_next(this->m_chain, iter);
}
CbufferRef BufferChain_unpack_iter(BufferChainRef this, BufferChainIter iter)
{
    return (CbufferRef)List_itr_unpack(this->m_chain, iter);
}
void BufferChain_remove_iter(BufferChainRef this, BufferChainIter iter)
{
    BufferChainIter tmp = iter;
    List_itr_remove(this->m_chain, &tmp);
}

void BufferChain_add_front(BufferChainRef this, CbufferRef cbuf)
{
    List_add_front(this->m_chain, (void*) cbuf);
    this->m_size = Cbuffer_size(cbuf);
}
void BufferChain_add_back(BufferChainRef this, CbufferRef cbuf)
{
    List_add_back(this->m_chain, (void*) cbuf);
    this->m_size = Cbuffer_size(cbuf);
}

CbufferRef BufferChain_pop_front(BufferChainRef this)
{
    CbufferRef front = (CbufferRef) List_first(this->m_chain);
    List_remove_first(this->m_chain);
    return front;
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

