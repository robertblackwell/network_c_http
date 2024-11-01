/**
 * @file KVPair.c
 * @Brief Implementation file for key/value pair
 */

#include <http_in_c/http/kvpair.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <http_in_c/common/alloc.h>
/**
 * @brief key value pair with string key and string value
 */
struct KVPair_s {
    // this string is always null terminated
    char*  label_ptr;
    size_t label_len;
    // this string is always null terminated
    char*  value_ptr;
    size_t value_len;
};

//char* make_upper(char* src)
//{
//    int srclen = strlen(src);
//    char* dest = eg_alloc(srclen+1);
//    char* s = src;
//    char* p = dest;
//    for(int i = 0; i < srclen; i++) {
//        p[i] = toupper((unsigned char) dest[i]);
//    }
//    p[srclen+1] = (unsigned char)'\0';
//    return dest;
//}
// creates and initializes a new KVPair obj. Returns NULL on allocation failure
KVPairRef KVPair_new(const char* labptr, int lablen, const char* valptr, size_t vallen)
{
    // store {label}: {value}\r\n\0
    KVPairRef hlref = eg_alloc(sizeof(KVPair));
    if(hlref  == NULL) goto mem_error_1;
    hlref->label_len = lablen;
    hlref->value_len = vallen;

    hlref->label_ptr = eg_alloc(lablen+1);
    if(hlref->label_ptr == NULL) goto mem_error_2;
    memcpy(hlref->label_ptr, labptr, lablen);
        // Convert to upper case
    char* p = hlref->label_ptr;
    for(int i = 0; i < lablen; i++) {
        p[i] = (char)toupper((unsigned char) labptr[i]);
    }
    /**
     * Note NULL termination
     */
    p[lablen] = '\0';

    hlref->value_ptr = eg_alloc(vallen+1);
    if(hlref->value_ptr == NULL) goto mem_error_2;
    memcpy(hlref->value_ptr, valptr, vallen);
    /**
     * Note NULL termination
     */
    hlref->value_ptr[vallen] = (char)'\0';

    return  hlref;
    mem_error_1:
        // nothing got allocated
        return NULL;
    mem_error_2:
        // hlref ok one of the otehrs failed
        if(hlref->label_ptr != NULL) free((void*)hlref->label_ptr);
        if(hlref->value_ptr != NULL) free((void*)hlref->value_ptr);
        free((void*)hlref);
        return NULL;
}
KVPairRef KVPair_from_cstrs(const char* key, const char* value)
{
    return KVPair_new(key, strlen(key), value, strlen(value));
}
KVPairRef KVPair_from_cstrpair(CStrPair cstrp)
{
    return KVPair_from_cstrs(cstrp.key, cstrp.value);
}

void KVPair_dispose(KVPairRef* hlref_ptr)
{
    KVPairRef hlref = *hlref_ptr;
    eg_free(hlref->label_ptr);
    hlref->label_len = 0;
//    hlref->label_ptr = NULL;
    eg_free(hlref->value_ptr);
    hlref->value_len = 0;
//    hlref->value_ptr = NULL;
    eg_free((void*) hlref);
    *hlref_ptr = NULL;
}
void KVPair_dealloc(void* ptr) { KVPair_dispose((KVPairRef*)(ptr));}
char* KVPair_label(const KVPairRef hlref)
{
    return hlref->label_ptr;
}
char* KVPair_value(const KVPairRef hlref)
{
    return hlref->value_ptr;
}
void KVPair_set_value(KVPairRef hlref, const char* valptr, size_t vallen)
{
    char* oldvalptr = hlref->value_ptr;
    hlref->value_ptr = eg_alloc(vallen+1);
    memcpy(hlref->value_ptr, valptr, vallen);
    hlref->value_ptr[vallen] = '\0';
    free(oldvalptr);
}
