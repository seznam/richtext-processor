/*
 * Vector implementation, compatible with ANSI C (C89) and traditional
 * compilers.
 *
 * There are many other existing implementation that were rejected because they
 * target newer C standard or compilers (there is no point having a project
 * targeting older C versions if its dependencies cannot be built on the target
 * compilers):
 *  - vc_vector - https://github.com/skogorev/vc_vector - targets C99
 *  - GArray from GTK glib -
 *    https://gitlab.gnome.org/GNOME/glib/-/tree/main/glib - does not target
 *    traditional compilers.
 *  - dynarray -
 *    https://stackoverflow.com/questions/3456446/a-good-c-equivalent-of-stl-vector#answer-3463498 -
 *    does not target traditional compilers.
 *  - SGLIB - https://sourceforge.net/projects/sglib/ - targets newer C
 *    versions.
 *  - qLibc - https://github.com/wolkykim/qlibc - targets newer C versions.
 *  - Gena - https://github.com/cher-nov/Gena - does not target traditional
 *    compilers.
 */

#ifndef VECTOR_HEADER_FILE
#define VECTOR_HEADER_FILE 1

#include <stddef.h>

typedef struct VectorSize {
	size_t itemSize;
	unsigned long length;
	unsigned long capacity;
} VectorSize;

typedef struct Vector {
	VectorSize size;
	void *items;
} Vector;

static const unsigned long VECTOR_AUTO_GROW_FACTOR = 2;
static const unsigned long VECTOR_AUTO_GROW_LIMIT = 64;

Vector *Vector_new(size_t itemSize,
		   unsigned long length, unsigned long capacity);

Vector *Vector_from(size_t itemSize, unsigned long length, void *values[]);

Vector *Vector_of1(size_t itemSize, void *value1);

Vector *Vector_of2(size_t itemSize, void *value1, void *value2);

Vector *Vector_of3(size_t itemSize, void *value1, void *value2, void *value3);

Vector *Vector_of4(size_t itemSize, void *value1, void *value2, void *value3,
		   void *value4);

Vector *Vector_of5(size_t itemSize, void *value1, void *value2, void *value3,
		   void *value4, void *value5);

Vector *Vector_of6(size_t itemSize, void *value1, void *value2, void *value3,
		   void *value4, void *value5, void *value6);

Vector *Vector_of7(size_t itemSize, void *value1, void *value2, void *value3,
		   void *value4, void *value5, void *value6, void *value7);

Vector *Vector_of8(size_t itemSize, void *value1, void *value2, void *value3,
		   void *value4, void *value5, void *value6, void *value7,
		   void *value8);

Vector *Vector_grow(Vector * vector, unsigned long capacity);

Vector *Vector_append(Vector * vector, void *value);

Vector *Vector_pop(Vector * vector, void *removedValue);

Vector *Vector_concat(Vector * vector1, Vector * vector2);

Vector *Vector_bigSlice(Vector * vector, unsigned long from, unsigned long to);

Vector *Vector_slice(Vector * vector, long from, long to);

Vector *Vector_get(Vector * vector, unsigned long index, void *value);

Vector *Vector_set(Vector * vector, unsigned long index, void *value);

Vector *Vector_clear(Vector * vector);

void Vector_free(Vector * vector);

#endif
