/**
 * @file KVPair.h
 * @brief KVPair implements a key-value pair where both key and value are char* strings
 */
#ifndef C_HTTP_KVPAIR_H
#define C_HTTP_KVPAIR_H
#include <stddef.h>
/**
 * @addtogroup group_kvp
 * @brief Abstract type for a key/value pair with both elements strings
 * @{
 */
typedef struct KVPair_s KVPair;

typedef KVPair* KVPairRef;

typedef struct CStrPair {
    char* key;
    char* value;
} CStrPair;
/**
 * @brief Create a new KV pair from two ptr+len strings. Returns NULL on failure
 *
 * @param labptr char*  pointer to start of string that is to be label or key.
 *                      does not assume null terminated. This string will be copied.
 *                      So caller retains responsibility for memory
 * @param lablen size_t len of the string to be label/key
 * @param valptr        pointer to start of memory holding string value.
 *                      does not assume null terminated. This string is copied
 *                      So caller retains responsibility for memory.
 * @param valsize       Length of value str
 * @return KVPairRef or NULL on failure
 */
KVPairRef KVPair_new(const char* labptr, int lablen, const char* valptr, size_t valsize);
/**
 * @brief Create a new KVPair from two c-strings
 * \param key char*         c string null terminated. Function copies the string so
 *                          caller retains responsibility for memory.
 *                          Is converted to upper case.
 * \param value char*       c string null terminated. Function copies the string so
 *                          caller retains responsibility for memory
 * \return KVPairRef or NULL on failure
 */
KVPairRef KVPair_from_cstrs(const char* key, const char* value);
KVPairRef KVPair_from_cstrpair(CStrPair cstrp);
/**
 * @brief Release a KVPair freeing all held resources
 * @param hlref_ptr
 */
void KVPair_free(KVPairRef hlref);
void KVPair_dealloc(void* ptr);

/**
 * @brief Set the value string in an existing KVPair.
 * \param hlref     KVPairRef
 * \param valptr    pointer to string holding new value. Not a c string
 *                  not null terminated. The value is copied.
 * \param vallen    length of value string
 */
void KVPair_set_value(KVPairRef hlref, const char* valptr, size_t vallen);
/**
 * @brief Returns the key or label value for an existing KVPair.
 * \param hlref KVPairRef
 * \return char* Null terminated c string. this is a reference to the value
 *                      held inside the KVPairRef so ownership is retained
 *                      in the KVPairRef hlref
 */
char* KVPair_label(KVPair const * hlref);
/**
 * @brief Returns the value string for an existing KVPair.
 * \param hlref KVPairRef
 * \return char* Null terminated c string. this is a reference to the value
 *                      held inside the KVPairRef so ownership is retained
 *                      in the KVPairRef hlref
 */
char* KVPair_value(KVPair const * hlref);
/** @} */
#endif