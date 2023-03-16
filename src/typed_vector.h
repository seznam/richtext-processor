#ifndef TYPED_VECTOR_HEADER_FILE
#define TYPED_VECTOR_HEADER_FILE 1

#include "vector.h"

#define Vector_ofType(type)\
typedef struct type##Vector {\
	VectorSize size;\
	type *items;\
} type##Vector;\
\
type##Vector *type##Vector_new(unsigned long length, unsigned long capacity);\
\
type##Vector *type##Vector_grow(type##Vector * vector, unsigned long capacity);\
\
type##Vector *type##Vector_append(type##Vector * vector, type * value);\
\
type##Vector *type##Vector_pop(type##Vector * vector, type * removedValue);\
\
type##Vector *type##Vector_concat(type##Vector * vector1,\
				  type##Vector * vector2);\
\
type##Vector *type##Vector_bigSlice(type##Vector * vector, unsigned long from,\
				    unsigned long to);\
\
type##Vector *type##Vector_slice(type##Vector * vector, long from, long to);\
\
type##Vector *type##Vector_get(type##Vector * vector, unsigned long index,\
			       type * value);\
\
type##Vector *type##Vector_set(type##Vector * vector, unsigned long index,\
			       type * value);\
\
type##Vector *type##Vector_clear(type##Vector * vector);\
\
void type##Vector_free(type##Vector * vector);

#define Vector_ofTypeImplementation(type)\
type##Vector *type##Vector_new(length, capacity)\
unsigned long length;\
unsigned long capacity;\
{\
	return (type##Vector *) Vector_new(sizeof(type), length, capacity);\
}\
\
type##Vector *type##Vector_grow(vector, capacity)\
type##Vector * vector;\
unsigned long capacity;\
{\
	return (type##Vector *) Vector_grow((Vector *) vector, capacity);\
}\
\
type##Vector *type##Vector_append(vector, value)\
type##Vector *vector;\
type *value;\
{\
	return (type##Vector *) Vector_append((Vector *) vector, value);\
}\
\
type##Vector *type##Vector_pop(vector, removedValue)\
type##Vector *vector;\
type *removedValue;\
{\
	return (type##Vector *) Vector_pop((Vector *) vector, removedValue);\
}\
\
type##Vector *type##Vector_concat(vector1, vector2)\
type##Vector *vector1;\
type##Vector *vector2;\
{\
	return (type##Vector *) Vector_concat((Vector *) vector1,\
					      (Vector *) vector2);\
}\
\
type##Vector *type##Vector_bigSlice(vector, from, to)\
type##Vector *vector;\
unsigned long from;\
unsigned long to;\
{\
	return (type##Vector *) Vector_bigSlice((Vector *) vector, from, to);\
}\
\
type##Vector *type##Vector_slice(vector, from, to)\
type##Vector *vector;\
long from;\
long to;\
{\
	return (type##Vector *) Vector_slice((Vector *) vector, from, to);\
}\
\
type##Vector *type##Vector_get(vector, index, value)\
type##Vector *vector;\
unsigned long index;\
type *value;\
{\
	return (type##Vector *) Vector_get((Vector *) vector, index, value);\
}\
\
type##Vector *type##Vector_set(vector, index, value)\
type##Vector *vector;\
unsigned long index;\
type *value;\
{\
	return (type##Vector *) Vector_set((Vector *) vector, index, value);\
}\
\
type##Vector *type##Vector_clear(vector)\
type##Vector *vector;\
{\
	return (type##Vector *) Vector_clear((Vector *) vector);\
}\
\
void type##Vector_free(vector)\
type##Vector *vector;\
{\
	Vector_free((Vector *) vector);\
}

#endif
