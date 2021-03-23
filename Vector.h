#ifndef HEADER_Vector
#define HEADER_Vector
/*
htop - Vector.h
(C) 2004-2011 Hisham H. Muhammad
Released under the GNU GPLv2, see the COPYING file
in the source distribution for its full text.
*/

#include "Object.h"

#include <stdbool.h>


#ifndef DEFAULT_SIZE
#define DEFAULT_SIZE 10
#endif


typedef struct Vector_ {
   Object** array;
   const ObjectClass* type;
   size_t arraySize;
   size_t growthRate;
   size_t items;
   bool owner;
} Vector;

Vector* Vector_new(const ObjectClass* type, bool owner, size_t size);

void Vector_delete(Vector* this);

void Vector_prune(Vector* this);

void Vector_quickSortCustomCompare(Vector* this, Object_Compare compare);
static inline void Vector_quickSort(Vector* this) {
   Vector_quickSortCustomCompare(this, this->type->compare);
}

void Vector_insertionSort(Vector* this);

void Vector_insert(Vector* this, size_t idx, void* data_);

Object* Vector_take(Vector* this, size_t idx);

Object* Vector_remove(Vector* this, size_t idx);

void Vector_moveUp(Vector* this, size_t idx);

void Vector_moveDown(Vector* this, size_t idx);

void Vector_set(Vector* this, size_t idx, void* data_);

#ifndef NDEBUG

Object* Vector_get(const Vector* this, size_t idx);
size_t Vector_size(const Vector* this);
size_t Vector_count(const Vector* this);

#else /* NDEBUG */

static inline Object* Vector_get(const Vector* this, size_t idx) {
   return this->array[idx];
}

static inline size_t Vector_size(const Vector* this) {
   return this->items;
}

#endif /* NDEBUG */

static inline const ObjectClass* Vector_type(const Vector* this) {
    return this->type;
}

void Vector_add(Vector* this, void* data_);

ssize_t Vector_indexOf(const Vector* this, const void* search_, Object_Compare compare);

void Vector_splice(Vector* this, Vector* from);

#endif
