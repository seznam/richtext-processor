#ifndef TYPED_POINTER_VECTOR_HEADER_FILE
#define TYPED_POINTER_VECTOR_HEADER_FILE 1

#include "vector.h"

#define Vector_ofPointer(type)\
typedef struct type##PointerVector {\
	VectorSize size;\
	type **items;\
} type##PointerVector;\
\
type##PointerVector *type##PointerVector_new(unsigned long length,\
					     unsigned long capacity);\
\
type##PointerVector *type##PointerVector_grow(type##PointerVector * vector,\
					      unsigned long capacity);\
\
type##PointerVector *type##PointerVector_append(type##PointerVector * vector,\
						type ** value);\
\
type##PointerVector *type##PointerVector_pop(type##PointerVector * vector,\
					     type ** removedValue);\
\
type##PointerVector *type##PointerVector_bigSlice(type##PointerVector * vector,\
						  unsigned long from,\
						  unsigned long to);\
\
type##PointerVector *type##PointerVector_slice(type##PointerVector * vector,\
					       long from, long to);\
\
type##PointerVector *type##PointerVector_get(type##PointerVector * vector,\
					     unsigned long index,\
					     type ** value);\
\
type##PointerVector *type##PointerVector_set(type##PointerVector * vector,\
					     unsigned long index,\
					     type ** value);\
\
type##PointerVector *type##PointerVector_clear(type##PointerVector * vector);\
\
void type##PointerVector_free(type##PointerVector * vector);

#define Vector_ofPointerImplementation(type)\
type##PointerVector *type##PointerVector_new(length, capacity)\
unsigned long length;\
unsigned long capacity;\
{\
	return (type##PointerVector *) Vector_new(sizeof(type *), length,\
						  capacity);\
}\
\
type##PointerVector *type##PointerVector_grow(vector, capacity)\
type##PointerVector * vector;\
unsigned long capacity;\
{\
	return (type##PointerVector *) Vector_grow((Vector *) vector,\
						   capacity);\
}\
\
type##PointerVector *type##PointerVector_append(vector, value)\
type##PointerVector *vector;\
type **value;\
{\
	return (type##PointerVector *) Vector_append((Vector *) vector, value);\
}\
\
type##PointerVector *type##PointerVector_pop(vector, removedValue)\
type##PointerVector *vector;\
type **removedValue;\
{\
	return (type##PointerVector *) Vector_pop((Vector *) vector,\
						  removedValue);\
}\
\
type##PointerVector *type##PointerVector_bigSlice(vector, from, to)\
type##PointerVector *vector;\
unsigned long from;\
unsigned long to;\
{\
	return (type##PointerVector *) Vector_bigSlice((Vector *) vector, from,\
						       to);\
}\
\
type##PointerVector *type##PointerVector_slice(vector, from, to)\
type##PointerVector *vector;\
long from;\
long to;\
{\
	return (type##PointerVector *) Vector_slice((Vector *) vector, from,\
						    to);\
}\
\
type##PointerVector *type##PointerVector_get(vector, index, value)\
type##PointerVector *vector;\
unsigned long index;\
type **value;\
{\
	return (type##PointerVector *) Vector_get((Vector *) vector, index,\
						  value);\
}\
\
type##PointerVector *type##PointerVector_set(vector, index, value)\
type##PointerVector *vector;\
unsigned long index;\
type **value;\
{\
	return (type##PointerVector *) Vector_set((Vector *) vector, index,\
						  value);\
}\
\
type##PointerVector *type##PointerVector_clear(vector)\
type##PointerVector *vector;\
{\
	return (type##PointerVector *) Vector_clear((Vector *) vector);\
}\
\
void type##PointerVector_free(vector)\
type##PointerVector *vector;\
{\
	Vector_free((Vector *) vector);\
}

#endif
