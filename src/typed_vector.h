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
type##Vector *type##Vector_from(unsigned long length, type *values[]);\
\
type##Vector *type##Vector_of1(type value1);\
\
type##Vector *type##Vector_of2(type value1, type value2);\
\
type##Vector *type##Vector_of3(type value1, type value2, type value3);\
\
type##Vector *type##Vector_of4(type value1, type value2, type value3,\
			       type value4);\
\
type##Vector *type##Vector_of5(type value1, type value2, type value3,\
			       type value4, type value5);\
\
type##Vector *type##Vector_of6(type value1, type value2, type value3,\
			       type value4, type value5, type value6);\
\
type##Vector *type##Vector_of7(type value1, type value2, type value3,\
			       type value4, type value5, type value6,\
			       type value7);\
\
type##Vector *type##Vector_of8(type value1, type value2, type value3,\
			       type value4, type value5, type value6,\
			       type value7, type value8);\
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
type##Vector *type##Vector_from(length, values)\
unsigned long length;\
type *values[];\
{\
	return (type##Vector *) Vector_from(sizeof(type), length,\
					    (void**) values);\
}\
\
type##Vector *type##Vector_of1(value1)\
type value1;\
{\
	return (type##Vector *) Vector_of1(sizeof(type), &value1);\
}\
\
type##Vector *type##Vector_of2(value1, value2)\
type value1;\
type value2;\
{\
	return (type##Vector *) Vector_of2(sizeof(type), &value1, &value2);\
}\
\
type##Vector *type##Vector_of3(value1, value2, value3)\
type value1;\
type value2;\
type value3;\
{\
	return (type##Vector *) Vector_of3(sizeof(type), &value1, &value2,\
					   &value3);\
}\
\
type##Vector *type##Vector_of4(value1, value2, value3, value4)\
type value1;\
type value2;\
type value3;\
type value4;\
{\
	return (type##Vector *) Vector_of4(sizeof(type), &value1, &value2,\
					   &value3, &value4);\
}\
\
type##Vector *type##Vector_of5(value1, value2, value3, value4, value5)\
type value1;\
type value2;\
type value3;\
type value4;\
type value5;\
{\
	return (type##Vector *) Vector_of5(sizeof(type), &value1, &value2,\
					   &value3, &value4, &value5);\
}\
\
type##Vector *type##Vector_of6(value1, value2, value3, value4, value5, value6)\
type value1;\
type value2;\
type value3;\
type value4;\
type value5;\
type value6;\
{\
	return (type##Vector *) Vector_of6(sizeof(type), &value1, &value2,\
					   &value3, &value4, &value5, &value6);\
}\
\
type##Vector *type##Vector_of7(value1, value2, value3, value4, value5, value6,\
			       value7)\
type value1;\
type value2;\
type value3;\
type value4;\
type value5;\
type value6;\
type value7;\
{\
	return (type##Vector *) Vector_of7(sizeof(type), &value1, &value2,\
					   &value3, &value4, &value5, &value6,\
					   &value7);\
}\
\
type##Vector *type##Vector_of8(value1, value2, value3, value4, value5, value6,\
			       value7, value8)\
type value1;\
type value2;\
type value3;\
type value4;\
type value5;\
type value6;\
type value7;\
type value8;\
{\
	return (type##Vector *) Vector_of8(sizeof(type), &value1, &value2,\
					   &value3, &value4, &value5, &value6,\
					   &value7, &value8);\
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
