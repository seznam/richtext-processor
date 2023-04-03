#include <limits.h>
#include "../src/vector.h"
#include "unit.h"

/*
   This file does not bother to free heap-allocated memory because it's a suite
	 of unit tests, ergo it's not a big deal.
 */

unsigned int tests_run = 0;
unsigned int tests_failed = 0;

static void all_tests(void);

int main(void);

START_TEST(Vector_new_returnsNULLIfCapacityIsLessThanLength)
{
	Vector *vector = Vector_new(1, 2, 1);
	assert(vector == NULL, "Expected null vector");
END_TEST}

START_TEST(Vector_new_createsAVectorIfLengthMatchesCapacity)
{
	Vector *vector = Vector_new(1, 2, 2);
	assert(vector != NULL
	       && vector->size.length == 2, "Expected a vector of length 2");
END_TEST}

START_TEST(Vector_new_createsAVectorOfRequestedProperties)
{
	Vector *vector = Vector_new(4, 3, 5);
	assert(vector != NULL, "Expected a non-null vector");
	assert(vector->items != NULL, "Expected items not to be NULL");
	assert(vector->size.itemSize == 4,
	       "Expected the size.itemSize to be 4");
	assert(vector->size.length == 3, "Expected size.length to be 3");
	assert(vector->size.capacity == 5, "Expected size.capacity to be 5");
END_TEST}

START_TEST(Vector_from_rejectsZeroItemSize)
{
	void *values[1];
	int value = 11;
	Vector *vector;
	values[0] = &value;
	vector = Vector_from(0, 1, values);
	assert(vector == NULL, "Expected the returned vector to be NULL");
END_TEST}

START_TEST(Vector_from_rejectsNullValues)
{
	Vector *vector = Vector_from(sizeof(int), 0, NULL);
	assert(vector == NULL, "Expected the returned vector to be NULL");
END_TEST}

START_TEST(Vector_from_canCreateEmptyVector)
{
	void *values[1];
	Vector *vector = Vector_from(sizeof(int), 0, values);
	assert(vector != NULL, "Expected a non-null vector");
	assert(vector->size.itemSize == sizeof(int),
	       "Expected size.itemSize to be size of int");
	assert(vector->size.length == 0, "Expected size.length to be 0");
	assert(vector->size.capacity == 0, "Expected size.capacity to be 0");
END_TEST}

START_TEST(Vector_from_createsVectorOfProvidedValues)
{
	void *values[3];
	int value1 = 11;
	int value2 = 17;
	int value3 = 21;
	Vector *vector;
	int *vectorValue;
	values[0] = &value1;
	values[1] = &value2;
	values[2] = &value3;
	vector = Vector_from(sizeof(int), 3, values);
	assert(vector != NULL, "Expected a non-null vector");
	assert(vector->size.itemSize == sizeof(int),
	       "Expected size.itemSize to be size of int");
	assert(vector->size.length == 3, "Expected size.length to be 3");
	assert(vector->size.capacity == 3, "Expected size.capacity to be 3");
	vectorValue = vector->items;
	assert(*vectorValue == 11, "Expected the 1st item to be 11");
	assert(*(vectorValue + 1) == 17, "Expected the 1st item to be 17");
	assert(*(vectorValue + 2) == 21, "Expected the 1st item to be 21");
END_TEST}

START_TEST(Vector_of1_createsVectorOf1Value)
{
	int value1 = 15;
	int *vectorValue;
	Vector *vector = Vector_of1(sizeof(value1), &value1);
	assert(vector != NULL, "Expected a non-null vector");
	assert(vector->size.itemSize == sizeof(int),
	       "Expected size.itemSize to be size of int");
	assert(vector->size.length == 1, "Expected size.length to be 1");
	assert(vector->size.capacity == 1, "Expected size.capacity to be 1");
	vectorValue = vector->items;
	assert(*vectorValue == value1, "Expected the 1st item to be 15");
	value1 = 0;
	assert(*vectorValue == 15,
	       "Expected the 1st item to retain its value after later modification of input");
END_TEST}

START_TEST(Vector_of2_createsVectorOf2Values)
{
	int value1 = 15;
	int value2 = 22;
	int *vectorValue;
	Vector *vector = Vector_of2(sizeof(value1), &value1, &value2);
	assert(vector != NULL, "Expected a non-null vector");
	assert(vector->size.itemSize == sizeof(int),
	       "Expected size.itemSize to be size of int");
	assert(vector->size.length == 2, "Expected size.length to be 2");
	assert(vector->size.capacity == 2, "Expected size.capacity to be 2");
	vectorValue = vector->items;
	assert(*vectorValue == value1, "Expected the 1st item to be 15");
	value1 = 0;
	assert(*vectorValue == 15,
	       "Expected the 1st item to retain its value after later modification of input");
	assert(*(vectorValue + 1) == value2, "Expected the 2nd item to be 22");
END_TEST}

START_TEST(Vector_of3_createsVectorOf3Values)
{
	int value1 = 15;
	int value2 = 22;
	int value3 = 7;
	int *vectorValue;
	Vector *vector = Vector_of3(sizeof(value1), &value1, &value2, &value3);
	assert(vector != NULL, "Expected a non-null vector");
	assert(vector->size.itemSize == sizeof(int),
	       "Expected size.itemSize to be size of int");
	assert(vector->size.length == 3, "Expected size.length to be 3");
	assert(vector->size.capacity == 3, "Expected size.capacity to be 3");
	vectorValue = vector->items;
	assert(*vectorValue == value1, "Expected the 1st item to be 15");
	value1 = 0;
	assert(*vectorValue == 15,
	       "Expected the 1st item to retain its value after later modification of input");
	assert(*(vectorValue + 1) == value2, "Expected the 2nd item to be 22");
	assert(*(vectorValue + 2) == value3, "Expected the 3rd item to be 7");
END_TEST}

START_TEST(Vector_of4_createsVectorOf4Values)
{
	int value1 = 15;
	int value2 = 22;
	int value3 = 7;
	int value4 = -11;
	int *vectorValue;
	Vector *vector =
	    Vector_of4(sizeof(value1), &value1, &value2, &value3, &value4);
	assert(vector != NULL, "Expected a non-null vector");
	assert(vector->size.itemSize == sizeof(int),
	       "Expected size.itemSize to be size of int");
	assert(vector->size.length == 4, "Expected size.length to be 4");
	assert(vector->size.capacity == 4, "Expected size.capacity to be 4");
	vectorValue = vector->items;
	assert(*vectorValue == value1, "Expected the 1st item to be 15");
	value1 = 0;
	assert(*vectorValue == 15,
	       "Expected the 1st item to retain its value after later modification of input");
	assert(*(vectorValue + 1) == value2, "Expected the 2nd item to be 22");
	assert(*(vectorValue + 2) == value3, "Expected the 3rd item to be 7");
	assert(*(vectorValue + 3) == value4, "Expected the 4th item to be -11");
END_TEST}

START_TEST(Vector_of5_createsVectorOf5Values)
{
	int value1 = 15;
	int value2 = 22;
	int value3 = 7;
	int value4 = -11;
	int value5 = 33;
	int *vectorValue;
	Vector *vector =
	    Vector_of5(sizeof(value1), &value1, &value2, &value3, &value4,
		       &value5);
	assert(vector != NULL, "Expected a non-null vector");
	assert(vector->size.itemSize == sizeof(int),
	       "Expected size.itemSize to be size of int");
	assert(vector->size.length == 5, "Expected size.length to be 5");
	assert(vector->size.capacity == 5, "Expected size.capacity to be 5");
	vectorValue = vector->items;
	assert(*vectorValue == value1, "Expected the 1st item to be 15");
	value1 = 0;
	assert(*vectorValue == 15,
	       "Expected the 1st item to retain its value after later modification of input");
	assert(*(vectorValue + 1) == value2, "Expected the 2nd item to be 22");
	assert(*(vectorValue + 2) == value3, "Expected the 3rd item to be 7");
	assert(*(vectorValue + 3) == value4, "Expected the 4th item to be -11");
	assert(*(vectorValue + 4) == value5, "Expected the 5th item to be 33");
END_TEST}

START_TEST(Vector_of6_createsVectorOf6Values)
{
	int value1 = 15;
	int value2 = 22;
	int value3 = 7;
	int value4 = -11;
	int value5 = 33;
	int value6 = 5;
	int *vectorValue;
	Vector *vector =
	    Vector_of6(sizeof(value1), &value1, &value2, &value3, &value4,
		       &value5, &value6);
	assert(vector != NULL, "Expected a non-null vector");
	assert(vector->size.itemSize == sizeof(int),
	       "Expected size.itemSize to be size of int");
	assert(vector->size.length == 6, "Expected size.length to be 6");
	assert(vector->size.capacity == 6, "Expected size.capacity to be 6");
	vectorValue = vector->items;
	assert(*vectorValue == value1, "Expected the 1st item to be 15");
	value1 = 0;
	assert(*vectorValue == 15,
	       "Expected the 1st item to retain its value after later modification of input");
	assert(*(vectorValue + 1) == value2, "Expected the 2nd item to be 22");
	assert(*(vectorValue + 2) == value3, "Expected the 3rd item to be 7");
	assert(*(vectorValue + 3) == value4, "Expected the 4th item to be -11");
	assert(*(vectorValue + 4) == value5, "Expected the 5th item to be 33");
	assert(*(vectorValue + 5) == value6, "Expected the 6th item to be 5");
END_TEST}

START_TEST(Vector_of7_createsVectorOf7Values)
{
	int value1 = 15;
	int value2 = 22;
	int value3 = 7;
	int value4 = -11;
	int value5 = 33;
	int value6 = 5;
	int value7 = 41;
	int *vectorValue;
	Vector *vector =
	    Vector_of7(sizeof(value1), &value1, &value2, &value3, &value4,
		       &value5, &value6, &value7);
	assert(vector != NULL, "Expected a non-null vector");
	assert(vector->size.itemSize == sizeof(int),
	       "Expected size.itemSize to be size of int");
	assert(vector->size.length == 7, "Expected size.length to be 7");
	assert(vector->size.capacity == 7, "Expected size.capacity to be 7");
	vectorValue = vector->items;
	assert(*vectorValue == value1, "Expected the 1st item to be 15");
	value1 = 0;
	assert(*vectorValue == 15,
	       "Expected the 1st item to retain its value after later modification of input");
	assert(*(vectorValue + 1) == value2, "Expected the 2nd item to be 22");
	assert(*(vectorValue + 2) == value3, "Expected the 3rd item to be 7");
	assert(*(vectorValue + 3) == value4, "Expected the 4th item to be -11");
	assert(*(vectorValue + 4) == value5, "Expected the 5th item to be 33");
	assert(*(vectorValue + 5) == value6, "Expected the 6th item to be 5");
	assert(*(vectorValue + 6) == value7, "Expected the 7th item to be 41");
END_TEST}

START_TEST(Vector_of8_createsVectorOf8Values)
{
	int value1 = 15;
	int value2 = 22;
	int value3 = 7;
	int value4 = -11;
	int value5 = 33;
	int value6 = 5;
	int value7 = 41;
	int value8 = 29;
	int *vectorValue;
	Vector *vector =
	    Vector_of8(sizeof(value1), &value1, &value2, &value3, &value4,
		       &value5, &value6, &value7, &value8);
	assert(vector != NULL, "Expected a non-null vector");
	assert(vector->size.itemSize == sizeof(int),
	       "Expected size.itemSize to be size of int");
	assert(vector->size.length == 8, "Expected size.length to be 8");
	assert(vector->size.capacity == 8, "Expected size.capacity to be 8");
	vectorValue = vector->items;
	assert(*vectorValue == value1, "Expected the 1st item to be 15");
	value1 = 0;
	assert(*vectorValue == 15,
	       "Expected the 1st item to retain its value after later modification of input");
	assert(*(vectorValue + 1) == value2, "Expected the 2nd item to be 22");
	assert(*(vectorValue + 2) == value3, "Expected the 3rd item to be 7");
	assert(*(vectorValue + 3) == value4, "Expected the 4th item to be -11");
	assert(*(vectorValue + 4) == value5, "Expected the 5th item to be 33");
	assert(*(vectorValue + 5) == value6, "Expected the 6th item to be 5");
	assert(*(vectorValue + 6) == value7, "Expected the 7th item to be 41");
	assert(*(vectorValue + 7) == value8, "Expected the 8th item to be 29");
END_TEST}

START_TEST(Vector_grow_returnsNullForNullVector)
{
	assert(Vector_grow(NULL, 8) == NULL,
	       "Expected NULL result when passing-in NULL Vector");
END_TEST}

START_TEST(Vector_grow_doesNotModifyVectorIfNotNeeded)
{
	Vector *vector = Vector_new(sizeof(int), 0, 4);
	void *items = vector->items;
	assert(Vector_grow(vector, 3) == vector,
	       "The vector was needlessly modified");
	assert(Vector_grow(vector, 3)->size.capacity == 4,
	       "The vector was needlessly modified");
	assert(Vector_grow(vector, 3)->items == items,
	       "The vector was needlessly modified");
	assert(Vector_grow(vector, 4) == vector,
	       "The vector was needlessly modified");
	assert(Vector_grow(vector, 4)->size.capacity == 4,
	       "The vector was needlessly modified");
	assert(Vector_grow(vector, 4)->items == items,
	       "The vector was needlessly modified");
END_TEST}

START_TEST(Vector_grow_increasesCapacityWhilePreservingData)
{
	Vector *vector = Vector_new(sizeof(int), 0, 2);
	Vector *vector2;
	int *items = (int *)vector->items;
	*(items) = 42;
	*(items + 1) = 2001;
	vector->size.length = 2;

	/* 1 MiB should be large enough to span multiple blocks and force reallocation */
	vector2 = Vector_grow(vector, 1024 * 1024);
	assert(vector2 == vector, "Expected the same vector pointer as result");
	assert(items != vector->items, "The items pointer was not modified");
	assert(vector->size.itemSize == sizeof(int),
	       "The size.itemSize field was modified");
	assert(vector->size.length == 2, "The size.length field was modified");
	assert(vector->size.capacity == 1024 * 1024,
	       "The size.capacity is incorrect");
	items = (int *)vector->items;
	assert(*items == 42, "The data has not been preserved");
	assert(*(items + 1) == 2001, "The data has not been preserved");
END_TEST}

START_TEST(Vector_append_returnNullForNullVector)
{
	int tmp = 1;
	assert(Vector_append(NULL, &tmp) == NULL,
	       "Expected NULL returned for NULL vector");
END_TEST}

START_TEST(Vector_append_returnNullForNullValuePointer)
{
	Vector *vector = Vector_new(sizeof(int), 0, 1);
	assert(Vector_append(vector, NULL) == NULL,
	       "Expected NULL returned for NULL value pointer");
END_TEST}

START_TEST(Vector_append_appendsWithoutGrowingCapacityIfPossible)
{
	Vector *vector = Vector_new(sizeof(int), 0, 1);
	int value = 42;
	assert(Vector_append(vector, &value) == vector,
	       "Expected the vector itself as returned value");
	assert(vector->size.length == 1,
	       "Expected the length to be grown to 1");
	assert(vector->size.capacity == 1,
	       "Expected capacity to remain unchanged");
	assert(*((int *)vector->items) == value,
	       "Expected the first item to be ");
END_TEST}

START_TEST(Vector_append_copiesTheProvidedValue)
{
	Vector *vector = Vector_new(sizeof(int), 0, 1);
	int value = 11;
	Vector_append(vector, &value);
	value = 10;
	assert(*((int *)vector->items) == 11,
	       "Expected the provided value to be copied instead of referenced");
END_TEST}

START_TEST(Vector_append_appendsMultipleValues)
{
	Vector *vector = Vector_new(sizeof(int), 0, 4);
	int value = 42;
	Vector_append(vector, &value);
	value = 50;
	Vector_append(vector, &value);
	value = 56;
	Vector_append(vector, &value);
	value = -64;
	Vector_append(vector, &value);
	assert(*((int *)vector->items) == 42,
	       "Expected 42 as value of the first item");
	assert(*((int *)vector->items + 1) == 50,
	       "Expected 50 as value of the second item");
	assert(*((int *)vector->items + 2) == 56,
	       "Expected 56 as value of the third item");
	assert(*((int *)vector->items + 3) == -64,
	       "Expected -64 as value of the fourth item");
END_TEST}

START_TEST(Vector_append_growsCapacityWhenNeeded)
{
	Vector *vector = Vector_new(sizeof(int), 0, 0);
	int value = 42;
	assert(vector->size.capacity == 0,
	       "Expected the created vector's capacity to be 0");
	assert(Vector_append(vector, &value) == vector,
	       "Expected Vector_append to return the appended-to vector");
	assert(vector->size.capacity > 0,
	       "Expected the appended-to vector to have increased capacity");
	assert(vector->size.length == 1,
	       "Expected the vector's length to be 1 after appending to it");
	assert(*((int *)vector->items) == 42,
	       "Expected the first item of the vector to be 42");
END_TEST}

START_TEST(Vector_append_growsCapacityByNoMoreThanLimit)
{
	Vector *vector = Vector_new(sizeof(int), VECTOR_AUTO_GROW_LIMIT * 2,
				    VECTOR_AUTO_GROW_LIMIT * 2);
	int value = 1;
	Vector_append(vector, &value);
	assert(vector->size.length == VECTOR_AUTO_GROW_LIMIT * 2 + 1,
	       "Expected the vector's length to be incremented");
	assert(vector->size.capacity > vector->size.length,
	       "Expected the vector's capacity to be grown faster than its length");
	assert(vector->size.capacity == VECTOR_AUTO_GROW_LIMIT * 3,
	       "Expected the vector's capacity to be grown by the maximum allowed amount");
END_TEST}

START_TEST(Vector_pop_returnsNullForNullVectorOrValue)
{
	Vector *vector = Vector_new(sizeof(int), 0, 1);
	int value;
	assert(Vector_pop(NULL, &value) == NULL,
	       "Expected NULL returned for NULL input vector");
	assert(Vector_pop(vector, NULL) == NULL,
	       "Expected NULL returned for NULL value pointer");
END_TEST}

START_TEST(Vector_pop_returnsNullForEmptyVector)
{
	Vector *vector = Vector_new(sizeof(int), 0, 1);
	int value;
	assert(Vector_pop(vector, &value) == NULL,
	       "Expected NULL returned for empty input vector");
END_TEST}

START_TEST(Vector_pop_removesLastItemFromVector)
{
	Vector *vector = Vector_new(sizeof(int), 0, 2);
	int value;
	value = 16;
	Vector_append(vector, &value);
	value = 42;
	Vector_append(vector, &value);
	value = 0;
	assert(vector->size.length == 2,
	       "Expected vector's length to be 2 before calling Vector_pop");
	assert(Vector_pop(vector, &value) == vector,
	       "Expected Vector_pop to return the vector");
	assert(value == 42,
	       "Expected to receive the last value from Vector_pop");
	assert(vector->size.length == 1,
	       "Expected the vector's length to be decreased after calling Vector_pop");
	assert(Vector_pop(vector, &value) == vector,
	       "Expected Vector_pop to return the vector");
	assert(value == 16,
	       "Expected to receive the last value from Vector_pop");
	assert(vector->size.length == 0,
	       "Expected the vector's length to be decreased after calling Vector_pop");
END_TEST}

START_TEST(Vector_concat_rejectsNullInput)
{
	Vector *vector = Vector_new(sizeof(int), 0, 0);
	assert(Vector_concat(NULL, NULL) == NULL,
	       "Expected NULL result for NULL used as both inputs");
	assert(Vector_concat(vector, NULL) == NULL,
	       "Expected NULL result for NULL 2nd vector");
	assert(Vector_concat(NULL, vector) == NULL,
	       "Expected NULL result for NULL 1st vector");
END_TEST}

START_TEST(Vector_concat_rejectsNonMatchingItemSizeVectors)
{
	Vector *vector1 = Vector_new(sizeof(int), 0, 0);
	Vector *vector2 = Vector_new(sizeof(long), 0, 0);
	assert(Vector_concat(vector1, vector2) == NULL,
	       "Expected NULL for vectors of mismatched item size");
END_TEST}

START_TEST(Vector_concat_rejectsTooBigInput)
{
	Vector *vector1 = Vector_new(sizeof(int), 0, 0);
	Vector *vector2 = Vector_new(sizeof(long), 0, 0);
	vector1->size.length = ULONG_MAX / 2;
	vector2->size.length = vector1->size.length + 1;
	assert(Vector_concat(vector1, vector2) == NULL,
	       "Expected NULL result if resulting vector would have more that ULONG_MAX items");
END_TEST}

START_TEST(Vector_concat_returnsVectorOfCapacityMatchingLength)
{
	Vector *vector1 = Vector_new(sizeof(int), 2, 2);
	Vector *vector2 = Vector_new(sizeof(int), 3, 6);
	assert(Vector_concat(vector1, vector2)->size.length == 5,
	       "Expected the resulting vector to have length 5");
	assert(Vector_concat(vector1, vector2)->size.capacity == 5,
	       "Expected the resulting vector to have capacity 5");
END_TEST}

START_TEST(Vector_concat_returnsVectorOfInputItemsInOrder)
{
	Vector *vector1 = Vector_new(sizeof(int), 0, 3);
	Vector *vector2 = Vector_new(sizeof(int), 0, 2);
	Vector *concatenated = NULL;
	int *value = malloc(sizeof(int));

	*value = 1;
	Vector_append(vector1, value);
	*value = 2;
	Vector_append(vector1, value);
	*value = 3;
	Vector_append(vector1, value);
	*value = 4;
	Vector_append(vector2, value);
	*value = 5;
	Vector_append(vector2, value);

	concatenated = Vector_concat(vector1, vector2);

	Vector_get(concatenated, 0, value);
	assert(*value == 1, "Expected the 1st item from 1st vector on index 0");
	Vector_get(concatenated, 1, value);
	assert(*value == 2, "Expected the 2nd item from 1st vector on index 1");
	Vector_get(concatenated, 2, value);
	assert(*value == 3, "Expected the 3rd item from 1st vector on index 2");
	Vector_get(concatenated, 3, value);
	assert(*value == 4, "Expected the 1st item from 2nd vector on index 3");
	Vector_get(concatenated, 4, value);
	assert(*value == 5, "Expected the 2nd item from 2nd vector on index 4");
END_TEST}

START_TEST(Vector_concat_doesNotModifyInput)
{
	Vector *vector1 = Vector_new(sizeof(int), 0, 3);
	Vector *vector2 = Vector_new(sizeof(int), 0, 2);
	int *value = malloc(sizeof(int));

	*value = 1;
	Vector_append(vector1, value);
	*value = 2;
	Vector_append(vector1, value);
	*value = 3;
	Vector_append(vector1, value);
	*value = 4;
	Vector_append(vector2, value);
	*value = 5;
	Vector_append(vector2, value);

	Vector_concat(vector1, vector2);

	assert(vector1->size.itemSize == sizeof(int),
	       "Expected the 1st vector to not be modified");
	assert(vector1->size.length == 3,
	       "Expected the 1st vector to not be modified");
	assert(vector1->size.capacity == 3,
	       "Expected the 1st vector to not be modified");
	Vector_get(vector1, 0, value);
	assert(*value == 1, "Expected the 1st vector to not be modified");
	Vector_get(vector1, 1, value);
	assert(*value == 2, "Expected the 1st vector to not be modified");
	Vector_get(vector1, 2, value);
	assert(*value == 3, "Expected the 1st vector to not be modified");
	Vector_get(vector2, 0, value);
	assert(*value == 4, "Expected the 2nd vector to not be modified");
	Vector_get(vector2, 1, value);
	assert(*value == 5, "Expected the 2nd vector to not be modified");
END_TEST}

START_TEST(Vector_concat_acceptsSameVectorAsBothInputs)
{
	Vector *vector = Vector_new(sizeof(int), 0, 2);
	Vector *concatenated = NULL;
	int *value = malloc(sizeof(int));

	*value = 1;
	Vector_append(vector, value);
	*value = 2;
	Vector_append(vector, value);

	concatenated = Vector_concat(vector, vector);

	assert(concatenated->size.itemSize == vector->size.itemSize,
	       "Expected the itemSize to match");
	assert(concatenated->size.length == 4, "Expected the length to be 4");
	assert(concatenated->size.capacity == 4,
	       "Expected the capacity to be 4");

	Vector_get(concatenated, 0, value);
	assert(*value == 1,
	       "Expected the 1st item in concatenated vector to match the 1st input vector item");
	Vector_get(concatenated, 1, value);
	assert(*value == 2,
	       "Expected the 1st item in concatenated vector to match the 2nd input vector item");
	Vector_get(concatenated, 2, value);
	assert(*value == 1,
	       "Expected the 1st item in concatenated vector to match the 1st input vector item");
	Vector_get(concatenated, 3, value);
	assert(*value == 2,
	       "Expected the 1st item in concatenated vector to match the 2nd input vector item");
END_TEST}

START_TEST(Vector_bigSlice_returnsNullForNullInputOrFromLargerThanTo)
{
	Vector *vector = Vector_new(sizeof(int), 2, 2);
	assert(Vector_bigSlice(NULL, 0, 1) == NULL,
	       "Expected NULL returned for NULL input vector");
	assert(Vector_bigSlice(vector, 1, 0) == NULL,
	       "Expected NULL returned for 'from' index larger than 'to' index");
END_TEST}

START_TEST(Vector_bigSlice_returnsSliceWithDataCopyForValidInput)
{
	Vector *vector = Vector_new(sizeof(int), 0, 5);
	Vector *slice;
	int value = 1;
	int *vectorItems;
	int *sliceItems;
	Vector_append(vector, &value);
	value = 2;
	Vector_append(vector, &value);
	value = 3;
	Vector_append(vector, &value);
	value = 4;
	Vector_append(vector, &value);
	value = 5;
	Vector_append(vector, &value);

	slice = Vector_bigSlice(vector, 1, 4);
	assert(slice != NULL, "Expected a slice, received NULL");
	assert(slice->size.itemSize == vector->size.itemSize,
	       "The slice is incorrect item size");
	assert(slice->size.length == 3, "The slice has incorrect length");
	assert(slice->size.capacity == 3,
	       "The slice has incorrect capacity, it must match its length");
	sliceItems = (int *)slice->items;
	assert(*sliceItems == 2, "Expected the first item of slice to be 2");
	assert(*(sliceItems + 1) == 3,
	       "Expected the second item of slice to be 3");
	assert(*(sliceItems + 2) == 4,
	       "Expected the third item of slice to be 4");

	vectorItems = vector->items;
	*(vectorItems + 1) = 0;
	*(vectorItems + 2) = 0;
	*(vectorItems + 3) = 0;
	assert(*sliceItems == 2,
	       "Expected slice's items to occupy memory space separated from the source vector");
	assert(*(sliceItems + 1) == 3,
	       "Expected slice's items to occupy memory space separated from the source vector");
	assert(*(sliceItems + 2) == 4,
	       "Expected slice's items to occupy memory space separated from the source vector");
END_TEST}

START_TEST(Vector_bigSlice_capsToIndexToPreventOverflow)
{
	Vector *vector = Vector_new(sizeof(int), 0, 2);
	Vector *slice;
	int value = 1;
	Vector_append(vector, &value);
	value = 2;
	Vector_append(vector, &value);
	slice = Vector_bigSlice(vector, 1, 10);
	assert(slice->size.length == 1, "Expected the slice's length to be 1");
	assert(slice->size.capacity == 1,
	       "Expected the slice's capacity to match its length");
	assert(*((int *)slice->items) == 2,
	       "Expected the slice's first item to be match the vector's item at the from index");
END_TEST}

START_TEST(Vector_bigSlice_capsFromIndexToPreventOverflow)
{
	Vector *vector = Vector_new(sizeof(int), 2, 20);
	Vector *slice;
	slice = Vector_bigSlice(vector, 4, 10);
	assert(slice != NULL, "Expected non-NULL slice");
	assert(slice->size.length == 0, "Expected an empty slice");
	assert(slice->size.capacity == 0,
	       "Expected the slice's capacity to match its length");
END_TEST}

START_TEST(Vector_bigSlice_returnsEmptySliceIfFromMatchesTo)
{
	Vector *vector = Vector_new(sizeof(int), 5, 5);
	assert(Vector_bigSlice(vector, 3, 3)->size.length == 0,
	       "Expected an empty slice");
	assert(Vector_bigSlice(vector, 3, 3)->size.capacity == 0,
	       "Expected the slice's capacity to match its length");
END_TEST}

START_TEST(Vector_slice_returnsNullForNullVector)
{
	assert(Vector_slice(NULL, 0, 1) == NULL,
	       "Expected NULL for NULL input vector");
END_TEST}

START_TEST(Vector_slice_returnsSliceWithDataCopyForValidInput)
{
	Vector *vector = Vector_new(sizeof(int), 0, 5);
	Vector *slice;
	int value = 1;
	int *vectorItems;
	int *sliceItems;
	Vector_append(vector, &value);
	value = 2;
	Vector_append(vector, &value);
	value = 3;
	Vector_append(vector, &value);
	value = 4;
	Vector_append(vector, &value);
	value = 5;
	Vector_append(vector, &value);

	slice = Vector_slice(vector, 1, 4);
	assert(slice != NULL, "Expected a slice, received NULL");
	assert(slice->size.itemSize == vector->size.itemSize,
	       "The slice is incorrect item size");
	assert(slice->size.length == 3, "The slice has incorrect length");
	assert(slice->size.capacity == 3,
	       "The slice has incorrect capacity, it must match its length");
	sliceItems = (int *)slice->items;
	assert(*sliceItems == 2, "Expected the first item of slice to be 2");
	assert(*(sliceItems + 1) == 3,
	       "Expected the second item of slice to be 3");
	assert(*(sliceItems + 2) == 4,
	       "Expected the third item of slice to be 4");

	vectorItems = vector->items;
	*(vectorItems + 1) = 0;
	*(vectorItems + 2) = 0;
	*(vectorItems + 3) = 0;
	assert(*sliceItems == 2,
	       "Expected slice's items to occupy memory space separated from the source vector");
	assert(*(sliceItems + 1) == 3,
	       "Expected slice's items to occupy memory space separated from the source vector");
	assert(*(sliceItems + 2) == 4,
	       "Expected slice's items to occupy memory space separated from the source vector");
END_TEST}

START_TEST(Vector_slice_wrapsNegativeFromIndexAroundVectorLength)
{
	Vector *vector = Vector_new(sizeof(int), 0, 7);
	Vector *slice;
	int value = 1;
	Vector_append(vector, &(value));
	value++;
	Vector_append(vector, &(value));
	value++;
	Vector_append(vector, &(value));

	slice = Vector_slice(vector, -1, 3);
	assert(slice->size.length == 1, "Expected a single-item slice");
	assert(*(int *)slice->items == 3,
	       "Expected the slice to contain the third item from source vector");

	slice = Vector_slice(vector, -7, 3);
	assert(slice->size.length == 1, "Expected a single-item slice");
	assert(*(int *)slice->items == 3,
	       "Expected the slice to contain the third item from source vector");

	slice = Vector_slice(vector, -11, 3);
	assert(slice->size.length == 2, "Expected a two-item slice");
	assert(*(int *)slice->items == 2,
	       "Expected the slice to contain the second and third item from source vector");
	assert(*((int *)slice->items + 1) == 3,
	       "Expected the slice to contain the second and third item from source vector");
END_TEST}

START_TEST(Vector_slice_wrapsNegativeToIndexAroundVectorLength)
{
	Vector *vector = Vector_new(sizeof(int), 0, 7);
	Vector *slice;
	int value = 1;
	Vector_append(vector, &(value));
	value++;
	Vector_append(vector, &(value));
	value++;
	Vector_append(vector, &(value));

	slice = Vector_slice(vector, 1, -1);
	assert(slice->size.length == 1, "Expected a single-item slice");
	assert(*(int *)slice->items == 2,
	       "Expected the slice to contain the second item from source vector");

	slice = Vector_slice(vector, 1, -7);
	assert(slice->size.length == 1, "Expected a single-item slice");
	assert(*(int *)slice->items == 2,
	       "Expected the slice to contain the second item from source vector");

	slice = Vector_slice(vector, 0, -10);
	assert(slice->size.length == 2, "Expected a two-item slice");
	assert(*(int *)slice->items == 1,
	       "Expected the slice to contain the first and second item from source vector");
	assert(*((int *)slice->items + 1) == 2,
	       "Expected the slice to contain the first and second item from source vector");
END_TEST}

START_TEST
    (Vector_get_returnNullForNullVectorOrNullValuePointerOrOutOfBoundsIndex) {
	Vector *vector = Vector_new(sizeof(int), 0, 1);
	int value;
	assert(Vector_get(NULL, 0, &value) == NULL,
	       "Expected NULL returned for NULL input vector");
	assert(Vector_get(vector, 0, NULL) == NULL,
	       "Expected NULL returned for NULL value pointer");
	assert(Vector_get(vector, 0, &value) == NULL,
	       "Expected NULL returned for out-of-bounds index");
END_TEST}

START_TEST(Vector_get_extractsCopyOfIndexedItem)
{
	Vector *vector = Vector_new(sizeof(int), 0, 3);
	int value = 1;
	Vector_append(vector, &value);
	value++;
	Vector_append(vector, &value);
	value++;
	Vector_append(vector, &value);

	value = 0;
	assert(Vector_get(vector, 0, &value) == vector,
	       "Expected the vector itself returned on success");
	assert(value == 1, "Expected the first item of the vector");

	value = 0;
	Vector_get(vector, 0, &value);
	assert(value == 1, "Expected the first item of the vector");

	Vector_get(vector, 1, &value);
	assert(value == 2, "Expected the second item of the vector");
	Vector_get(vector, 2, &value);
	assert(value == 3, "Expected the third item of the vector");
END_TEST}

START_TEST
    (Vector_set_returnNullForNullVectorOrNullValuePointerOrOutOfBoundsIndex) {
	Vector *vector = Vector_new(sizeof(int), 0, 1);
	int value;
	assert(Vector_set(NULL, 0, &value) == NULL,
	       "Expected NULL returned for NULL input vector");
	assert(Vector_set(vector, 0, NULL) == NULL,
	       "Expected NULL returned for NULL value pointer");
	assert(Vector_set(vector, 0, &value) == NULL,
	       "Expected NULL returned for out-of-bounds index");
END_TEST}

START_TEST(Vector_set_setsAValueCopyToIndexedSlot)
{
	Vector *vector = Vector_new(sizeof(int), 3, 3);
	int value = 1;
	assert(Vector_set(vector, 0, &value) == vector,
	       "Expected the vector itself returned on success");
	value = 0;
	Vector_get(vector, 0, &value);
	assert(value == 1,
	       "Expected the first slot to be set to a copy of the passed value");
	value++;
	assert(Vector_set(vector, 1, &value) == vector,
	       "Expected the vector itself returned on success");
	value++;
	assert(Vector_set(vector, 2, &value) == vector,
	       "Expected the vector itself returned on success");

	Vector_get(vector, 1, &value);
	assert(value == 2,
	       "Expected the second slot to to be set to a copy of the passed value");
	Vector_get(vector, 2, &value);
	assert(value == 3,
	       "Expected the third slot to to be set to a copy of the passed value");
END_TEST}

START_TEST(Vector_clear_returnsNullForNullVector)
{
	assert(Vector_clear(NULL) == NULL,
	       "Expected NULL returned for NULL input vector");
END_TEST}

START_TEST(Vector_clear_clearsVectorOfAllData)
{
	Vector *vector = Vector_new(sizeof(int), 8, 8);
	Vector_clear(vector);
	assert(vector->size.length == 0,
	       "Expected the cleared vector to have 0 length");
	assert(vector->size.capacity == 0,
	       "Expected the cleared vector to have 0 capacity");
	assert(vector->items == NULL,
	       "Expected the cleared vector's items pointer to be NULL");
END_TEST}

START_TEST(Vector_grow_canGrowClearedVector)
{
	Vector *vector = Vector_new(sizeof(int), 4, 4);
	Vector_clear(vector);
	assert(Vector_grow(vector, 2) == vector,
	       "Expected the cleared vector to be growable");
	assert(vector->size.capacity == 2,
	       "Expected the cleared vector to appendable");
	assert(vector->items != NULL,
	       "Expected the cleared vector to appendable");
END_TEST}

START_TEST(Vector_append_canAppendToClearedVector)
{
	Vector *vector = Vector_new(sizeof(int), 4, 4);
	int value = 42;
	Vector_clear(vector);
	assert(Vector_append(vector, &value) == vector,
	       "Expected the cleared vector to appendable");
	assert(vector->size.length == 1,
	       "Expected the cleared vector to appendable");
	assert(*((int *)vector->items) == 42,
	       "Expected the cleared vector to appendable");
END_TEST}

START_TEST(Vector_free_acceptsNullVector)
{
	Vector_free(NULL);
END_TEST}

START_TEST(Vector_free_acceptsVectorWithData)
{
	Vector *vector = Vector_new(sizeof(int), 2, 2);
	Vector_free(vector);
END_TEST}

START_TEST(Vector_free_acceptsVectorWithNullItemsPointer)
{
	Vector *vector = Vector_new(sizeof(int), 2, 2);
	Vector_clear(vector);
	Vector_free(vector);
END_TEST}

static void all_tests()
{
	runTest(Vector_new_returnsNULLIfCapacityIsLessThanLength);
	runTest(Vector_new_createsAVectorIfLengthMatchesCapacity);
	runTest(Vector_new_createsAVectorOfRequestedProperties);
	runTest(Vector_from_rejectsZeroItemSize);
	runTest(Vector_from_rejectsNullValues);
	runTest(Vector_from_canCreateEmptyVector);
	runTest(Vector_from_createsVectorOfProvidedValues);
	runTest(Vector_of1_createsVectorOf1Value);
	runTest(Vector_of2_createsVectorOf2Values);
	runTest(Vector_of3_createsVectorOf3Values);
	runTest(Vector_of4_createsVectorOf4Values);
	runTest(Vector_of5_createsVectorOf5Values);
	runTest(Vector_of6_createsVectorOf6Values);
	runTest(Vector_of7_createsVectorOf7Values);
	runTest(Vector_of8_createsVectorOf8Values);
	runTest(Vector_grow_returnsNullForNullVector);
	runTest(Vector_grow_doesNotModifyVectorIfNotNeeded);
	runTest(Vector_grow_increasesCapacityWhilePreservingData);
	runTest(Vector_append_returnNullForNullVector);
	runTest(Vector_append_returnNullForNullValuePointer);
	runTest(Vector_append_appendsWithoutGrowingCapacityIfPossible);
	runTest(Vector_append_copiesTheProvidedValue);
	runTest(Vector_append_appendsMultipleValues);
	runTest(Vector_append_growsCapacityWhenNeeded);
	runTest(Vector_append_growsCapacityByNoMoreThanLimit);
	runTest(Vector_pop_returnsNullForNullVectorOrValue);
	runTest(Vector_pop_returnsNullForEmptyVector);
	runTest(Vector_pop_removesLastItemFromVector);
	runTest(Vector_concat_rejectsNullInput);
	runTest(Vector_concat_rejectsNonMatchingItemSizeVectors);
	runTest(Vector_concat_rejectsTooBigInput);
	runTest(Vector_concat_returnsVectorOfCapacityMatchingLength);
	runTest(Vector_concat_returnsVectorOfInputItemsInOrder);
	runTest(Vector_concat_doesNotModifyInput);
	runTest(Vector_concat_acceptsSameVectorAsBothInputs);
	runTest(Vector_bigSlice_returnsNullForNullInputOrFromLargerThanTo);
	runTest(Vector_bigSlice_returnsSliceWithDataCopyForValidInput);
	runTest(Vector_bigSlice_capsToIndexToPreventOverflow);
	runTest(Vector_bigSlice_capsFromIndexToPreventOverflow);
	runTest(Vector_bigSlice_returnsEmptySliceIfFromMatchesTo);
	runTest(Vector_slice_returnsNullForNullVector);
	runTest(Vector_slice_returnsSliceWithDataCopyForValidInput);
	runTest(Vector_slice_wrapsNegativeFromIndexAroundVectorLength);
	runTest(Vector_slice_wrapsNegativeToIndexAroundVectorLength);
	runTest
	    (Vector_get_returnNullForNullVectorOrNullValuePointerOrOutOfBoundsIndex);
	runTest(Vector_get_extractsCopyOfIndexedItem);
	runTest
	    (Vector_set_returnNullForNullVectorOrNullValuePointerOrOutOfBoundsIndex);
	runTest(Vector_set_setsAValueCopyToIndexedSlot);
	runTest(Vector_clear_returnsNullForNullVector);
	runTest(Vector_clear_clearsVectorOfAllData);
	runTest(Vector_grow_canGrowClearedVector);
	runTest(Vector_append_canAppendToClearedVector);
	runTest(Vector_free_acceptsNullVector);
	runTest(Vector_free_acceptsVectorWithData);
	runTest(Vector_free_acceptsVectorWithNullItemsPointer);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}
