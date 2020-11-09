#define _GNU_SOURCE

#include <c_http/datasource.h>
#include <string.h>
#include <assert.h>
#include <c_http/alloc.h>

/**
 *
 * @param this DataSource*
 * @param blocks a pointer to an array of char* (an array of const cstring pointers)
 */
void DataSource_init(DataSource* this, char** blocks)
{
    this->m_block_count = 0;
    this->m_blocks = blocks;
}

/**
 * Returns the next block of utf-8 null terminated data, NULL when done
 * @return char*, NULL when done
 */
char* DataSource_next(DataSource* this)
{
    char* block = this->m_blocks[this->m_block_count];
    this->m_block_count++;
    return block;
}
/**
 * @return bool true when no more data
 */
bool DataSource_finished(DataSource* this)
{
    return (this->m_blocks[this->m_block_count] == NULL);
}

int DataSource_read(DataSource* this, void* buffer, int length)
{
    char* block = this->m_blocks[this->m_block_count];
    if (block == NULL) {
        return 0;
    } else if (strcmp(block, "error") == 0) {
        this->m_errno = -99;
        return -1;
    } else {
        this->m_block_count++;
        int block_len = strlen(block);
        assert(block_len < length);
        memcpy((void*)buffer, block, block_len);
        return block_len;
    }
}
