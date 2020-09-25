#ifndef c_eg_kvpair_h_guard
#define c_eg_kvpair_h_guard

/**
 * KVPair implements a key-value pair where both key and value area char* strings
 */
struct KVPair_s;

typedef struct KVPair_s KVPair, *KVPairRef;

KVPairRef KVPair_new(char* labptr, int lablen, char* valptr, int valsize);
void KVPair_free(KVPairRef* hlref_ptr);
void KVPair_dealloc(void* ptr);

void KVPair_set_value(KVPairRef hlref, char* valptr, int vallen);


char* KVPair_label(KVPairRef hlref);
char* KVPair_value(KVPairRef hlref);

#endif