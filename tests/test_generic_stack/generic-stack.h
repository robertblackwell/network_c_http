/**
 * generic-stack.h
 *   Declarations for a simple generic implementation of stacks.
 *
 * <insert MIT license>
 */

#ifdef TYPE

// +--------+--------------------------------------------------------
// | Macros |
// +--------+

/**
 * By default, we do not rename procedures and structs.
 */
#ifndef TYPED
#define TYPED(THING) THING
#endif

// +-------+---------------------------------------------------------
// | Types |
// +-------+

typedef struct TYPED(Stack_s) TYPED(Stack);

// +------------+----------------------------------------------------
// | Procedures |
// +------------+

/**
 * Create a new stack.
 */
TYPED(Stack)* TYPED(Stack_new) (void);

/**
 * Free the space allocated to a stack.
 */
void TYPED(Stack_free) (TYPED(Stack) *stack);

/**
 * Determine the number of elements in the stack.
 */
int TYPED(Stack_size) (TYPED(Stack) *stack);

/**
 * Look at the top element of the stack.  Requires that the
 * stack have at least one element.
 */
TYPE TYPED(Stack_top) (TYPED(Stack) *stack);

/**
 * Remove and return the top element of the stack.  Requires that
 * the stack have at least one element.
 */
TYPE TYPED(Stack_pop) (TYPED(Stack) *stack);

/**
 * Add an element to the stack.
 */
void TYPED(Stack_push) (TYPED(Stack) *stack, TYPE val);

#endif // TYPE