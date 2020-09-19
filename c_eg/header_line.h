#ifndef c_eg_header_line_h_guard
#define c_eg_header_line_h_guard

struct HeaderLine_s;

typedef struct HeaderLine_s HeaderLine, *HeaderLineRef;

HeaderLineRef HeaderLine_new(char* labptr, int lablen, char* valptr, int valsize);
void HeaderLine_free(HeaderLineRef* hlref_ptr);
void HeaderLine_dealloc(void* ptr);

void HeaderLine_set_value(HeaderLineRef hlref, char* valptr, int vallen);


char* HeaderLine_label(HeaderLineRef hlref);
char* HeaderLine_value(HeaderLineRef hlref);

#endif