#ifndef buffer_chain_template_hpp
#define buffer_chain_template_hpp
#include <stddef.h>
#include <memory>
#include <iostream>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <marvin/buffer/contig_buffer.hpp>
#include <marvin/buffer/contig_buffer_factory.hpp>

typedef struct BufferChain_s {

        std::vector<ContigBufferRef>          m_chain;
        std::size_t                           m_size;
        ContigBufferRef (*m_config_factory)();
} BufferChain, *BufferChainRef;

BufferChainRef BufferChain_new(ConfigBufferRef(*factory)())
{
    m_chain = (BufferChainRef)malloc(sizeof(BufferChain_s));
    m_chain = std::vector<ContigBuffer::SPtr>();
    m_size = 0;
    m_contig_factory = factory;
}
void BufferChain_free(BufferChainRef bchain)
{
    for(int i = 0; i < bchain->m_chain.size(); i++) {
        ContigBuffer_free(bchain->m_chain.at(i))
    }
    free((void*)bchain);
}
void BufferChain_append(BufferChainRef bchain, void* buf, std::size_t len)
{
    if (bchain->m_chain.size() > 0) {
        ContigBufferRef last_cb = m_chain.at(m_chain.size()-1);
        if ((ContigBuffer_capacity(last_mb) - ContigBuffer_size(last_mb)) >= len) {
            ContigBuffer_append(last_mb, buf, len);
            bchain->m_size += len;
            return;
        }
    }
    std::size_t required_len = (len > 256*4*8) ? len+100 : 256*4*8;
    ContigBufferRef new_cb = bchain->m_contig_factory();
    ContigBuffer_append(new_cb, buf, len);
    BufferChain_push_back(bchain, new_mb);
}
void BufferChain_append(BufferChainRef bchain, std::string str)
{
    append((void*)str.c_str(), str.size());
}
void BufferChain_append(BufferChainRef bchain, std::string& str)
{
    append((void*)str.c_str(), str.size());
}

void BufferChain_push_back(BufferChainRef bchain, ContigBufferRef cb)
{
    bchain->m_size += ContigBuffer_size(cb);
    bchain->m_chain.push_back(mb);
}
void BufferChain_clear(BufferChainRef bchain)
{
    for(int i = 0; i < bchain->m_chain.size(); i++) {
        ContifBuffer_free(bchain->mchain.at(i));
    }
    bchain->m_chain.clear();
    bchain->m_size = 0;
}
std::size_t BufferChain_size(BufferChainRef bchain)
{
    return m_size;
}
        
std::size_t BufferChain_blocks(BufferChainRef bchain, )
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
