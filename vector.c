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

	vector = (Vector *) malloc(sizeof(Vector));
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
		grownCapacity =
		    currentCapacity >
		    VECTOR_AUTO_GROW_LIMIT /
		    VECTOR_AUTO_GROW_FACTOR ? VECTOR_AUTO_GROW_LIMIT :
		    currentCapacity * VECTOR_AUTO_GROW_FACTOR;
		vector = Vector_grow(vector, grownCapacity || 1);
		if (vector == NULL) {
			return NULL;
		}
	}

	bucket =
	    (char *)vector->items + vector->size.itemSize * vector->size.length;
	memcpy(bucket, value, vector->size.itemSize);

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
