#ifndef c_eg_kvpair_h_guard
#define c_eg_kvpair_h_guard

/**
 * KVPair implements a key-value pair where both key and value area char* strings
 */
struct KVPair_s;
typedef struct KVPair_s KVPair, *KVPairRef;

typedef struct CStrPair {
    char* key;
    char* value;
} CStrPair;

KVPairRef KVPair_new(char* labptr, int lablen, char* valptr, int valsize);
KVPairRef KVPair_from_cstrs(char* key, char* value);
KVPairRef KVPair_from_cstrpair(CStrPair cstrp);
void KVPair_free(KVPairRef* hlref_ptr);
void KVPair_dealloc(void* ptr);

void KVPair_set_value(KVPairRef hlref, char* valptr, int vallen);
char* KVPair_label(KVPairRef hlref);
char* KVPair_value(KVPairRef hlref);

#endif