
#include "check_tag.h"
#include <stdint.h>
#if !defined(RBL_CHECK_TAG_INLINE)
size_t rbl_tag_size()
{
    typedef struct DummyTagStruct_s
    {
        RBL_DECLARE_TAG;
        uint64_t after_tag;
    } DummyTagStruct;
    size_t n = offsetof(DummyTagStruct, after_tag);
    return n;
}
#endif
