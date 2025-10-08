#include "stream_ctx.h"

void line_parser_init(LineParserRef lp)
{
    lp->line_buffer_length = 0;
    lp->line_buffer_max = 256;
}
char* line_parser_consume(LineParserRef lp, char* buffer, int buffer_length)
{
    for(int i = 0; i < buffer_length; i++){
        char ch = buffer[i];
        if((ch == '\n')||(ch == '\0')) {
            lp->line_buffer[lp->line_buffer_length] = '\0';
            return (lp->line_buffer);
        } else {
            int j = lp->line_buffer_length;
            if(j < lp->line_buffer_max) {
                lp->line_buffer[j] = ch;
                lp->line_buffer_length++;
            } else {
                assert(0);
            }
        }
    }
    return NULL;
}
