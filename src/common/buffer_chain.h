#ifndef buffer_chain_template_hpp
#define buffer_chain_template_hpp
#include <stddef.h>
#include <common/iobuffer.h>
/**
 * @addtogroup group_bufferchain
 * @{
 */
/**
 * @brief BufferChain as an opaque type
 */
struct BufferChain_s;
typedef struct BufferChain_s* BufferChainRef;
/**
 * @todo - this is a problem. ListRef leaks into the API. Probably make the iterator opaque.
 */
typedef void* BufferChainIter;

/**
 * @brief Create a new empty buffer chain
 * @return BufferChainRef
 */
BufferChainRef BufferChain_new();
/**
 * @brief Free a BufferChain and its associated resources.
 * @note This function does not update the argument. Its up to the called
 * to assign it to a suitable value after it becomes undefined.
 * @param this BufferChainref
 */
void BufferChain_free(BufferChainRef this);
/**
 * @brief Append the given data + len to the buffer chain by copying.
 *
 * Either add the data to the tail buffer on the chain if it is big enough or
 * allocate a new chain element and put the data in that new tail element.
 *
 * @param this BufferChainRef
 * @param buf  void*
 * @param len  size_t
 */
void BufferChain_append(BufferChainRef this, void* buf, int len);
/**
 * @brief Append the given data (cstr) to the buffer chain by copying.
 *
 * Either add the data to the tail buffer on the chain if it is big enough or
 * allocate a new tail end chain element and put the data in that new tail element.
 *
 * @param this BufferChainRef
 * @param buf  char* cstr
 */
void BufferChain_append_cstr(BufferChainRef this, char* cstr);
/**
 * @brief Append the given IOBuffer to the end of the chain.
 *
 * This works because the buffer chain is just a list of IOBufferRef
 *
 * @param this   BufferChainRef
 * @param iobuf  IOBufferRef
 */
void BufferChain_append_IOBuffer(BufferChainRef this, IOBufferRef cbuf);

void BufferChain_clear(BufferChainRef this);
/**
 * @brief Return the total amount of data stored in the buffer chain in number of bytes.
 * @param this BufferChainRef
 * @return int
 */
int BufferChain_size(const BufferChainRef this);
/**
 * @brief Squash the possibly multiple IOBuffers in the chain into a single IOBuffer by copying.
 *
 * The original BufferChain is left valid.
 *
 * @param this BufferChainRef
 * @return IOBufferRef
 */
IOBufferRef BufferChain_compact(const BufferChainRef this);
/**
 * @brief Tests whether the data content of the buffer chain is the same as a given c-string.
 *
 * Will be true even if the BufferChain does NOT have the terminating null.
 *
 * @param this BUfferChainRef
 * @param cstr char*
 * @return
 */
bool BufferChain_eq_cstr(const BufferChainRef this, char* cstr);
/**
 * @brief Get an iterator pointing at the first element in the buffer chain
 * @param this BufferChainref
 * @return  BufferChainIter or NULL if the chain is empty
 */
BufferChainIter BufferChain_iterator(BufferChainRef this);
/**
 * @brief Get the next iterator along the buffer chain from the current_iter
 * @param this         BufferChainRef
 * @param current_iter BufferChainIter
 * @return BufferChainIter or NULL if no more left
 */
BufferChainIter BufferChain_iter_next(BufferChainRef this, BufferChainIter iter);
/**
 * @brief Dereference or unpack the current_iter to get the contained IOBufferRef
 * @param this         BufferChainRef
 * @param current_iter BufferChainIter
 * @return IOBufferRef
 */
IOBufferRef BufferChain_unpack_iter(BufferChainRef this, BufferChainIter iter);
/**
 * @brief Delete the BufferChain element pointed at by the current_iter
 * @param this         BufferChainRef
 * @param current_iter BufferChainIter
 */
void BufferChain_remove_iter(BufferChainRef this, BufferChainIter iter);
/**
 * @brief Add an IOBuffer to the tail of the BufferChain
 * @param this  BufferChainref
 * @param iobuf IOBufferRef
 */
void BufferChain_add_back(BufferChainRef this, IOBufferRef iobuf);
/**
 * @brief Pop the first IOBufferRef from the BufferChain
 * @param this BufferChainRef
 * @return IOBufferRef or NULL if the chain is empty
 */
IOBufferRef BufferChain_pop_front(BufferChainRef this);
/**
 * @brief Gives a non-owning ref to the first IOBufferRef from the BufferChain
 * @param this BufferChainRef
 * @return IOBufferRef or NULL if the chain is empty
 */
IOBufferRef BufferChain_front(BufferChainRef this);

/*
 * too dangerous
 */
void BufferChain_append_bufferchain(BufferChainRef this, BufferChainRef other);
void BufferChain_steal_bufferchain(BufferChainRef this, BufferChainRef other);
void BufferChain_add_front(BufferChainRef this, IOBufferRef cbuf);
/** @} */
#endif
