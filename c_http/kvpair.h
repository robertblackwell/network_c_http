#ifndef c_http_kvpair_h_guard
#define c_http_kvpair_h_guard

/**
 * KVPair implements a key-value pair where both key and value area char* strings
 */
struct KVPair_s;
typedef struct KVPair_s KVPair, *KVPairRef;

typedef struct CStrPair {
    char* key;
    char* value;
} CStrPair;
/**
 * Create a new KV pair from two ptr+len
 * \param labptr char*  pointer to start of string that is to be label or key.
 *                      does not assume null terminated. This string will be copied.
 *                      So caller retains responsibility for memory
 * \param lablen int    len of the string to be label/key
 * \param valptr        pointer to start of memory holding string value.
 *                      does not assume null terminated. This string is copied
 *                      So caller retains responsibility for memory.
 * \param valsize       Length of value str
 * \return KVPairRef or NULL on failure
 */
KVPairRef KVPair_new(const char* labptr, int lablen, const char* valptr, int valsize);
/**
 *
 * \param key char*         c string null terminated. Function copies the string so
 *                          caller retains responsibility for memory
 * \param value char*       c string null terminated. Function copies the string so
 *                          caller retains responsibility for memory
 * \return KVPairRef or NULL on failure
 */
KVPairRef KVPair_from_cstrs(const char* key, const char* value);
KVPairRef KVPair_from_cstrpair(CStrPair cstrp);
void KVPair_free(KVPairRef* hlref_ptr);
void KVPair_dealloc(void* ptr);

/**
 * Set the value string in an existing KVPair.
 * \param hlref     KVPairRef
 * \param valptr    pointer to string holding new value. Not a c string
 *                  not null terminated. The value is copied.
 * \param vallen    length of value string
 */
void KVPair_set_value(KVPairRef hlref, const char* valptr, int vallen);
/**
 * Returns the key or label value for an existing KVPair.
 * \param hlref KVPairRef
 * \return char* Null terminated c string. this is a reference to the value
 *                      held inside the KVPairRef so ownership is retained
 *                      in the KVPairRef hlref
 */
char* KVPair_label(const KVPairRef hlref);
/**
 * Returns the value string for an existing KVPair.
 * \param hlref KVPairRef
 * \return char* Null terminated c string. this is a reference to the value
 *                      held inside the KVPairRef so ownership is retained
 *                      in the KVPairRef hlref
 */
char* KVPair_value(const KVPairRef hlref);

#endif