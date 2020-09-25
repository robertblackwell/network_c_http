#ifndef c_c_eg_datasource_h
#define c_c_eg_datasource_h
//#include <c_eg/list.h>
//#include <c_eg/rdsocket.h>
//#include <c_eg/message.h>
//#include <c_eg/parser.h>
#include <stdbool.h>
/**
 * Purpose of this class is to demo use of the parser in a situation
 * that simulates synchronously reading from some source of bytes
 * Generally a DataSource is created from the lines in a ParserTest
 */
typedef struct DataSource_s {
    // points as the current block being provided
    int   m_block_count;
    /** A array/list of data blocks null terminated*/
    char** m_blocks;
} DataSource, *DataSourceRef;

void DataSource_init(DataSourceRef this, char* blocks[]);

/**
 * Returns the next block of utf-8 null terminated data, NULL when done
 * @return char*, NULL when done
 */
char* DataSource_next(DataSourceRef this);

/**
 * @return bool true when no more data
 */
bool DataSource_finished(DataSourceRef this);

/*
* 'Reads' up to length data into buffer and returns the actually number 'read'
*/
int DataSource_read(DataSourceRef this, void* buffer, int length);


#endif