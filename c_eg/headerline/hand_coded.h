#ifndef c_ceg_headerline_list_h
#define c_ceg_headerline_list_h
#include <c_eg/header_line.h>
#include <c_eg/buffer/contig_buffer.h>


__LIST_INCLUDE_H__

void HDRList_add(HDRListRef this, CBufferRef key, CBufferRef value);
HeaderLineRef HDRList_find(HDRListRef hlref, char* key);
void HDRList_remove(HDRListRef hlref, char* key);

#endif