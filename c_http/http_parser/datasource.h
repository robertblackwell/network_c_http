#ifndef c_c_http_datasource_h
#define c_c_http_datasource_h
#include <stdbool.h>
/**
 * Structur part of a test harness for unit testing http parsing.
 * A datasource_t is simulated input to the parsing process
 */
typedef struct datasource_s {
    int     m_block_count;
    char**  m_blocks;
    int     m_errno;

} datasource_t;

void datasource_init(datasource_t* this, char* blocks[]);

/**
 * Returns the next block of utf-8 null terminated data, NULL when done
 * @return char*, NULL when done
 */
char* datasource_next(datasource_t* this);

/**
 * @return bool true when no more data
 */
bool DataSource_finished(datasource_t* this);

/*
* 'Reads' up to length data into buffer and returns the actually number 'read'
*/
int datasource_read_some(datasource_t* this, void* buffer, int length);

/** @} */
#endif