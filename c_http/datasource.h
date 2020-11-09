#ifndef c_c_http_datasource_h
#define c_c_http_datasource_h
#include <stdbool.h>
/**
 * Purpose of this class is to demo and test  use of the parser in a situation
 * that simulates synchronously reading from some source of bytes other than a real socket
 *
 * Generally a DataSource is created from the lines in a ParserTest
 */
typedef struct DataSource_s {
    /**
     *  points as the current block being provided
     */
    int     m_block_count;

    /**
     *  A array/list of data blocks null terminated
     */
    char**  m_blocks;
    /**
     * errno from most recent simulated io error
     */
    int     m_errno;

} DataSource;

void DataSource_init(DataSource* this, char* blocks[]);

/**
 * Returns the next block of utf-8 null terminated data, NULL when done
 * @return char*, NULL when done
 */
char* DataSource_next(DataSource* this);

/**
 * @return bool true when no more data
 */
bool DataSource_finished(DataSource* this);

/*
* 'Reads' up to length data into buffer and returns the actually number 'read'
*/
int DataSource_read(DataSource* this, void* buffer, int length);


#endif