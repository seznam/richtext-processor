#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"

Vector *Vector_new(itemSize, length, capacity)
size_t itemSize;
unsigned long length;
unsigned long capacity;
{
	Vector *vector;

	if (capacity < length) {
		return NULL;
	}

	vector = malloc(sizeof(Vector));
	if (vector == NULL) {
		return NULL;
	}

	vector->size.itemSize = itemSize;
	vector->size.length = length;
	vector->size.capacity = capacity;
	vector->items = malloc(itemSize * capacity);
	if (vector->items == NULL) {
		free(vector);
		return NULL;
	}

	return vector;
}

Vector *Vector_from(itemSize, length, values)
size_t itemSize;
unsigned long length;
void *values[];
{
	Vector *vector;
	unsigned long i;
	char *bucket;

	if (itemSize == 0 || values == NULL) {
		return NULL;
	}

	vector = Vector_new(itemSize, length, length);
	if (vector == NULL) {
		return NULL;
	}

	bucket = (char *)vector->items;
	for (i = 0; i < length; i++, bucket += itemSize) {
		memcpy(bucket, values[i], itemSize);
	}

	return vector;
}

Vector *Vector_of1(itemSize, value1)
size_t itemSize;
void *value1;
{
	void *values[1];
	values[0] = value1;
	return Vector_from(itemSize, 1, values);
}

Vector *Vector_of2(itemSize, value1, value2)
size_t itemSize;
void *value1;
void *value2;
{
	void *values[2];
	values[0] = value1;
	values[1] = value2;
	return Vector_from(itemSize, 2, values);
}

Vector *Vector_of3(itemSize, value1, value2, value3)
size_t itemSize;
void *value1;
void *value2;
void *value3;
{
	void *values[3];
	values[0] = value1;
	values[1] = value2;
	values[2] = value3;
	return Vector_from(itemSize, 3, values);
}

Vector *Vector_of4(itemSize, value1, value2, value3, value4)
size_t itemSize;
void *value1;
void *value2;
void *value3;
void *value4;
{
	void *values[4];
	values[0] = value1;
	values[1] = value2;
	values[2] = value3;
	values[3] = value4;
	return Vector_from(itemSize, 4, values);
}

Vector *Vector_of5(itemSize, value1, value2, value3, value4, value5)
size_t itemSize;
void *value1;
void *value2;
void *value3;
void *value4;
void *value5;
{
	void *values[5];
	values[0] = value1;
	values[1] = value2;
	values[2] = value3;
	values[3] = value4;
	values[4] = value5;
	return Vector_from(itemSize, 5, values);
}

Vector *Vector_of6(itemSize, value1, value2, value3, value4, value5, value6)
size_t itemSize;
void *value1;
void *value2;
void *value3;
void *value4;
void *value5;
void *value6;
{
	void *values[6];
	values[0] = value1;
	values[1] = value2;
	values[2] = value3;
	values[3] = value4;
	values[4] = value5;
	values[5] = value6;
	return Vector_from(itemSize, 6, values);
}

Vector *Vector_of7(itemSize, value1, value2, value3, value4, value5, value6,
		   value7)
size_t itemSize;
void *value1;
void *value2;
void *value3;
void *value4;
void *value5;
void *value6;
void *value7;
{
	void *values[7];
	values[0] = value1;
	values[1] = value2;
	values[2] = value3;
	values[3] = value4;
	values[4] = value5;
	values[5] = value6;
	values[6] = value7;
	return Vector_from(itemSize, 7, values);
}

Vector *Vector_of8(itemSize, value1, value2, value3, value4, value5, value6,
		   value7, value8)
size_t itemSize;
void *value1;
void *value2;
void *value3;
void *value4;
void *value5;
void *value6;
void *value7;
void *value8;
{
	void *values[8];
	values[0] = value1;
	values[1] = value2;
	values[2] = value3;
	values[3] = value4;
	values[4] = value5;
	values[5] = value6;
	values[6] = value7;
	values[7] = value8;
	return Vector_from(itemSize, 8, values);
}

Vector *Vector_grow(vector, capacity)
Vector *vector;
unsigned long capacity;
{
	void *newItems;

	if (vector == NULL) {
		return NULL;
	}
	if (vector->size.capacity >= capacity) {
		return vector;
	}

	newItems = realloc(vector->items, vector->size.itemSize * capacity);
	if (newItems == NULL) {
		return NULL;
	}

	vector->items = newItems;
	vector->size.capacity = capacity;
	return vector;
}

Vector *Vector_append(vector, value)
Vector *vector;
void *value;
{
	unsigned long currentCapacity;
	unsigned long grownCapacity;
	char *bucket;

	if (vector == NULL || value == NULL) {
		return NULL;
	}

	currentCapacity = vector->size.capacity;
	if (vector->size.length == currentCapacity) {
		grownCapacity = currentCapacity +
		    (currentCapacity >
		     VECTOR_AUTO_GROW_LIMIT /
		     VECTOR_AUTO_GROW_FACTOR ? VECTOR_AUTO_GROW_LIMIT :
		     currentCapacity * VECTOR_AUTO_GROW_FACTOR);
		vector =
		    Vector_grow(vector, grownCapacity > 0 ? grownCapacity : 1);
		if (vector == NULL) {
			return NULL;
		}
	}

	bucket =
	    (char *)vector->items + vector->size.itemSize * vector->size.length;
	memcpy(bucket, value, vector->size.itemSize);
	vector->size.length++;

	return vector;
}

Vector *Vector_pop(vector, removedValue)
Vector *vector;
void *removedValue;
{
	char *bucket;

	if (vector == NULL || !vector->size.length || removedValue == NULL) {
		return NULL;
	}

	vector->size.length--;
	bucket =
	    (char *)vector->items + vector->size.itemSize * vector->size.length;
	memcpy(removedValue, bucket, vector->size.itemSize);

	return vector;
}

Vector *Vector_concat(vector1, vector2)
Vector *vector1;
Vector *vector2;
{
	Vector *result;
	Vector *source;
	unsigned long maxVector2Length;
	char *bucket;
	unsigned long i;
	unsigned int sourceIndex;

	if (vector1 == NULL || vector2 == NULL) {
		return NULL;
	}
	if (vector1->size.itemSize != vector2->size.itemSize) {
		return NULL;
	}
	maxVector2Length = ULONG_MAX - vector1->size.length;
	if (vector2->size.length > maxVector2Length) {
		/* Imagine having so much RAM this would happen */
		return NULL;
	}

	result =
	    Vector_new(vector1->size.itemSize, 0,
		       vector1->size.length + vector2->size.length);
	if (result == NULL) {
		return NULL;
	}

	for (sourceIndex = 0; sourceIndex < 2; sourceIndex++) {
		source = sourceIndex == 0 ? vector1 : vector2;
		for (bucket = source->items, i = 0; i < source->size.length;
		     i++, bucket += source->size.itemSize) {
			if (Vector_append(result, bucket) == NULL) {
				Vector_free(result);
				return NULL;
			}
		}
	}

	return result;
}

Vector *Vector_bigSlice(vector, from, to)
Vector *vector;
unsigned long from;
unsigned long to;
{
	Vector *slice;
	char *sliceDataSrc;
	unsigned long length;

	if (vector == NULL || from > to) {
		return NULL;
	}

	from = from < vector->size.length ? from : vector->size.length;
	to = to < vector->size.length ? to : vector->size.length;
	length = to - from;

	slice = Vector_new(vector->size.itemSize, length, length);
	sliceDataSrc = (char *)vector->items + vector->size.itemSize * from;
	memcpy(slice->items, sliceDataSrc, vector->size.itemSize * length);

	return slice;
}

Vector *Vector_slice(vector, from, to)
Vector *vector;
long from;
long to;
{
	unsigned long vectorLength;
	unsigned long adjustedFrom;
	unsigned long adjustedTo;

	if (vector == NULL) {
		return NULL;
	}

	vectorLength = vector->size.length;
	adjustedFrom =
	    from >=
	    0 ? (unsigned long)from : vectorLength - (-from % vectorLength);
	adjustedTo =
	    to >= 0 ? (unsigned long)to : vectorLength - (-to % vectorLength);

	return Vector_bigSlice(vector, adjustedFrom, adjustedTo);
}

Vector *Vector_get(vector, index, value)
Vector *vector;
unsigned long index;
void *value;
{
	char *bucket;

	if (vector == NULL || index >= vector->size.length || value == NULL) {
		return NULL;
	}

	bucket = (char *)vector->items + vector->size.itemSize * index;
	memcpy(value, bucket, vector->size.itemSize);

	return vector;
}

Vector *Vector_set(vector, index, value)
Vector *vector;
unsigned long index;
void *value;
{
	char *bucket;

	if (vector == NULL || index >= vector->size.length || value == NULL) {
		return NULL;
	}

	bucket = (char *)vector->items + vector->size.itemSize * index;
	memcpy(bucket, value, vector->size.itemSize);

	return vector;
}

Vector *Vector_clear(vector)
Vector *vector;
{
	if (vector == NULL) {
		return NULL;
	}

	if (vector->items != NULL) {
		free(vector->items);
		vector->items = NULL;
	}

	vector->size.length = 0;
	vector->size.capacity = 0;

	return vector;
}

void Vector_free(vector)
Vector *vector;
{
	if (vector == NULL) {
		return;
	}

	if (vector->items != NULL) {
		free(vector->items);
	}
	free(vector);
}
