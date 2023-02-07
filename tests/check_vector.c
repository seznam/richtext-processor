#include "../src/vector.h"
#include "unit.h"

/*
   This file does not bother to free heap-allocated memory because it a suite
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
