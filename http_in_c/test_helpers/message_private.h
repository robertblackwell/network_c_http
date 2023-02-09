/**
* This is a private interface to the message object only to be used for testing
*/
#include <http_in_c/http/header_list.h>
/**
 * Within a Message instance header lines are stored in a HdrList structure.
 * Methods are provided to:
 * -    get the HdrList struct for manipulation             - Message_get_header_list
 * -    add a single header line from a pair of const char* - add_header_cstring
 *      this function copies the c-string parameters so the caller remains the owner.
 * -    get the value string for a header key - the const char* return value continues
 *      to be owned by the Message instance. The return value is a weak reference.
 *      Not found is signalled by return value NULL;
 */
 /**
  * Get a reference to the internal HdrList
  * @param this MessageRef
  * @return HdrListRef
  */
HdrListRef Message_get_headerlist(MessageRef this);
