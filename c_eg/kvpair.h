#ifndef c_eg_kvpair_h_guard
#define c_eg_kvpair_h_guard

/**
 * KVPair implements a key-value pair where both key and value area char* strings
 */
struct KVPair_s;

typedef struct KVPair_s KVPair;

KVPair* KVPair_new(char* labptr, int lablen, char* valptr, int valsize);
void KVPair_free(KVPair** hlref_ptr);
void KVPair_dealloc(void* ptr);

void KVPair_set_value(KVPair* hlref, char* valptr, int vallen);


char* KVPair_label(KVPair* hlref);
char* KVPair_value(KVPair* hlref);

#endif