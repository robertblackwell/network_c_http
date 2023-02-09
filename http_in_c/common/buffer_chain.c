#include <http_in_c/common/buffer_chain.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <http_in_c/common/alloc.h>
#include <http_in_c/common/list.h>
#include <http_in_c/common/iobuffer.h>

typedef struct BufferChain_s {
        ListRef   m_chain;
        int       m_size;
} BufferChain;

static void dealloc(void** p)
{
    IOBuffer_dispose((IOBufferRef*)p);
}
BufferChainRef BufferChain_new()
{
    BufferChainRef tmp = eg_alloc(sizeof(BufferChain));
    tmp->m_chain = List_new(dealloc);
    tmp->m_size = 0;
}
void BufferChain_free(BufferChainRef this)
{
    ListIterator iter = List_iterator(this->m_chain);
    for(;;) {
        if(iter == NULL) {
            break;
        }
        ListIterator next = List_itr_next(this->m_chain, iter);
        List_itr_remove(this->m_chain, &iter);
        iter = next;
    }
    List_dispose(&(this->m_chain));
    free((void*)this);
}

void BufferChain_dispose(BufferChainRef* thisptr)
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
    List_dispose(&(this->m_chain));
    *thisptr = NULL;
    free((void*)this);
}
void BufferChain_append(BufferChainRef this, void* buf, int len)
{
    if (this->m_size > 0) {
        IOBufferRef last_iob = List_last(this->m_chain);

        if (IOBuffer_space_len(last_iob) >= len) {
            IOBuffer_data_add(last_iob, buf, len);
            this->m_size += len;
            return;
        }
    }
    int required_len = (len > 256*4*8) ? len+100 : 256*4*8;
    IOBufferRef new_iob = IOBuffer_new_with_capacity(required_len);
    IOBuffer_data_add(new_iob, buf, len);
    List_add_back(this->m_chain, (void*)new_iob);
    this->m_size += len;
}
void BufferChain_append_IOBuffer(BufferChainRef this, IOBufferRef iobuf)
{
    BufferChain_append(this, IOBuffer_data(iobuf), IOBuffer_data_len(iobuf));
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

IOBufferRef BufferChain_compact(const BufferChainRef this)
{
    int required_size = BufferChain_size(this);
    IOBufferRef iob_final = IOBuffer_new_with_capacity(required_size);
    if(iob_final == NULL)
        goto memerror_01;
    ListIterator iter = List_iterator(this->m_chain);
    while(iter != NULL) {
        IOBufferRef tmp = (IOBufferRef)List_itr_unpack(this->m_chain, iter);
        void* data = IOBuffer_data(tmp);
        int sz = IOBuffer_data_len(tmp);
        IOBuffer_data_add(iob_final, data, sz); /* MEM CHECK REQUIRED*/
        ListIterator next = List_itr_next(this->m_chain, iter);
        iter = next;
    }
    return iob_final;
    memerror_01:
        return NULL;
}
bool BufferChain_eq_cstr(const BufferChainRef this, char* cstr)
{
    ListIterator iter = List_iterator(this->m_chain);
    int cstr_index = 0;
    while(iter != NULL) {
        IOBufferRef tmp = (IOBufferRef)List_itr_unpack(this->m_chain, iter);
        char* data = IOBuffer_data(tmp);
        int sz = IOBuffer_data_len(tmp);
        int l = strlen(cstr);
        for(int iob_index = 0; iob_index < sz; iob_index++) {
            if(cstr[cstr_index] != data[iob_index]) {
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
IOBufferRef BufferChain_unpack_iter(BufferChainRef this, BufferChainIter iter)
{
    return (IOBufferRef)List_itr_unpack(this->m_chain, iter);
}
void BufferChain_remove_iter(BufferChainRef this, BufferChainIter iter)
{
    ListIter tmp = iter;
    List_itr_remove(this->m_chain, &tmp);
}

void BufferChain_add_front(BufferChainRef this, IOBufferRef iobuf)
{
    List_add_front(this->m_chain, (void*) iobuf);
    this->m_size = IOBuffer_data_len(iobuf);
}
void BufferChain_append_bufferchain(BufferChainRef this, BufferChainRef other)
{
    ListIter iter = List_iterator(other->m_chain);
    while(iter != NULL) {
        IOBufferRef iob = List_itr_unpack(this->m_chain, iter);
        IOBufferRef iob_dup = IOBuffer_dup(iob);
        BufferChain_add_back(this, iob_dup);
        iter = List_itr_next(other->m_chain, iter);
    }
}
void BufferChain_steal_bufferchain(BufferChainRef this, BufferChainRef other)
{
    ListIter iter = List_iterator(other->m_chain);
    IOBufferRef iob = BufferChain_pop_front(other);
    while(iob != NULL) {
        BufferChain_add_back(this, iob);
        iob = BufferChain_pop_front(other);
    }
}
void BufferChain_add_back(BufferChainRef this, IOBufferRef iobuf)
{
    List_add_back(this->m_chain, (void*) iobuf);
    this->m_size += IOBuffer_data_len(iobuf);
}

IOBufferRef BufferChain_pop_front(BufferChainRef this)
{
    IOBufferRef front = (IOBufferRef) List_first(this->m_chain);
    if(front == NULL) {
        return NULL;
    }
    List_remove_first(this->m_chain);
    this->m_size -= IOBuffer_data_len(front);
    return front;
}
IOBufferRef BufferChain_front(BufferChainRef this)
{
    IOBufferRef r = List_first(this->m_chain);
    return r;
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

