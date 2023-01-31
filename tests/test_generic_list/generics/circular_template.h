
typedef struct PREFIX(_s) PREFIX(), *PREFIX(Ref);

/**
 * NOTE: FunctionList acceptt and return values of a Functor NOT a pointer
 */
PREFIX(Ref) PREFIX(_new)(int capacity);
void PREFIX(_free)(PREFIX(Ref) this);
void PREFIX(_add)(PREFIX(Ref) this, TYPE item);
TYPE PREFIX(_remove)(PREFIX(Ref) this);
int  PREFIX(_size)(PREFIX(Ref) this);
void PREFIX(_display)(PREFIX(Ref) lstref);

