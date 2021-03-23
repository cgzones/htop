/*
htop - Vector.c
(C) 2004-2011 Hisham H. Muhammad
Released under the GNU GPLv2, see the COPYING file
in the source distribution for its full text.
*/

#include "Vector.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "XUtils.h"


Vector* Vector_new(const ObjectClass* type, bool owner, size_t size) {
   Vector* this;

   assert(size > 0);
   this = xMalloc(sizeof(Vector));
   this->growthRate = size;
   this->array = (Object**) xCalloc(size, sizeof(Object*));
   this->arraySize = size;
   this->items = 0;
   this->type = type;
   this->owner = owner;
   return this;
}

void Vector_delete(Vector* this) {
   if (this->owner) {
      for (size_t i = 0; i < this->items; i++) {
         if (this->array[i]) {
            Object_delete(this->array[i]);
         }
      }
   }
   free(this->array);
   free(this);
}

#ifndef NDEBUG

static bool Vector_isConsistent(const Vector* this) {
   assert(this->items <= this->arraySize);

   if (this->owner) {
      for (size_t i = 0; i < this->items; i++) {
         if (!this->array[i]) {
            return false;
         }
      }
   }

   return true;
}

size_t Vector_count(const Vector* this) {
   size_t items = 0;
   for (size_t i = 0; i < this->items; i++) {
      if (this->array[i]) {
         items++;
      }
   }
   assert(items == this->items);
   return items;
}

Object* Vector_get(const Vector* this, size_t idx) {
   assert(idx < this->items);
   assert(this->array[idx]);
   assert(Object_isA(this->array[idx], this->type));
   return this->array[idx];
}

size_t Vector_size(const Vector* this) {
   assert(Vector_isConsistent(this));
   return this->items;
}

#endif /* NDEBUG */

void Vector_prune(Vector* this) {
   assert(Vector_isConsistent(this));
   if (this->owner) {
      for (size_t i = 0; i < this->items; i++)
         if (this->array[i]) {
            Object_delete(this->array[i]);
            //this->array[i] = NULL;
         }
   }
   this->items = 0;
}

//static int comparisons = 0;

static void swap(Object** array, size_t indexA, size_t indexB) {
   Object* tmp = array[indexA];
   array[indexA] = array[indexB];
   array[indexB] = tmp;
}

static size_t partition(Object** array, size_t left, size_t right, size_t pivotIndex, Object_Compare compare) {
   const Object* pivotValue = array[pivotIndex];
   swap(array, pivotIndex, right);
   size_t storeIndex = left;
   for (size_t i = left; i < right; i++) {
      //comparisons++;
      if (compare(array[i], pivotValue) <= 0) {
         swap(array, i, storeIndex);
         storeIndex++;
      }
   }
   swap(array, storeIndex, right);
   return storeIndex;
}

static void quickSort(Object** array, size_t left, size_t right, Object_Compare compare) {
   if (left >= right)
      return;

   size_t pivotIndex = (left + right) / 2;
   size_t pivotNewIndex = partition(array, left, right, pivotIndex, compare);
   quickSort(array, left, pivotNewIndex ? (pivotNewIndex - 1) : 0, compare);
   quickSort(array, pivotNewIndex + 1, right, compare);
}

// If I were to use only one sorting algorithm for both cases, it would probably be this one:
/*

static void combSort(Object** array, int left, int right, Object_Compare compare) {
   int gap = right - left;
   bool swapped = true;
   while ((gap > 1) || swapped) {
      if (gap > 1) {
         gap = (int)((double)gap / 1.247330950103979);
      }
      swapped = false;
      for (int i = left; gap + i <= right; i++) {
         comparisons++;
         if (compare(array[i], array[i+gap]) > 0) {
            swap(array, i, i+gap);
            swapped = true;
         }
      }
   }
}

*/

static void insertionSort(Object** array, size_t left, size_t right, Object_Compare compare) {
   for (size_t i = left + 1; i <= right; i++) {
      Object* t = array[i];
      size_t j = i;
      while (j > left) {
         //comparisons++;
         if (compare(array[j - 1], t) <= 0)
            break;

         array[j] = array[j - 1];

         j--;
      }
      array[j] = t;
   }
}

void Vector_quickSortCustomCompare(Vector* this, Object_Compare compare) {
   assert(compare);
   assert(Vector_isConsistent(this));

   if (this->items <= 1)
      return;

   quickSort(this->array, 0, this->items - 1, compare);
   assert(Vector_isConsistent(this));
}

void Vector_insertionSort(Vector* this) {
   assert(this->type->compare);
   assert(Vector_isConsistent(this));

   if (this->items <= 1)
      return;

   insertionSort(this->array, 0, this->items - 1, this->type->compare);
   assert(Vector_isConsistent(this));
}

static void Vector_checkArraySize(Vector* this) {
   assert(Vector_isConsistent(this));
   if (this->items >= this->arraySize) {
      //int i;
      //i = this->arraySize;
      this->arraySize = this->items + this->growthRate;
      this->array = (Object**) xRealloc(this->array, sizeof(Object*) * this->arraySize);
      //for (; i < this->arraySize; i++)
      //   this->array[i] = NULL;
   }
   assert(Vector_isConsistent(this));
}

void Vector_insert(Vector* this, size_t idx, void* data_) {
   Object* data = data_;
   assert(Object_isA(data, this->type));
   assert(Vector_isConsistent(this));

   if (idx > this->items) {
      idx = this->items;
   }

   Vector_checkArraySize(this);
   //assert(this->array[this->items] == NULL);
   if (idx < this->items) {
      memmove(&this->array[idx + 1], &this->array[idx], (this->items - idx) * sizeof(this->array[0]));
   }
   this->array[idx] = data;
   this->items++;
   assert(Vector_isConsistent(this));
}

Object* Vector_take(Vector* this, size_t idx) {
   assert(idx < this->items);
   assert(Vector_isConsistent(this));
   Object* removed = this->array[idx];
   assert(removed);
   this->items--;
   if (idx < this->items) {
      memmove(&this->array[idx], &this->array[idx + 1], (this->items - idx) * sizeof(this->array[0]));
   }
   //this->array[this->items] = NULL;
   assert(Vector_isConsistent(this));
   return removed;
}

Object* Vector_remove(Vector* this, size_t idx) {
   Object* removed = Vector_take(this, idx);
   if (this->owner) {
      Object_delete(removed);
      return NULL;
   } else {
      return removed;
   }
}

void Vector_moveUp(Vector* this, size_t idx) {
   assert(idx < this->items);
   assert(Vector_isConsistent(this));

   if (idx == 0)
      return;

   Object* temp = this->array[idx];
   this->array[idx] = this->array[idx - 1];
   this->array[idx - 1] = temp;
}

void Vector_moveDown(Vector* this, size_t idx) {
   assert(idx < this->items);
   assert(Vector_isConsistent(this));

   if (idx == this->items - 1)
      return;

   Object* temp = this->array[idx];
   this->array[idx] = this->array[idx + 1];
   this->array[idx + 1] = temp;
}

void Vector_set(Vector* this, size_t idx, void* data_) {
   Object* data = data_;
   assert(Object_isA(data, this->type));
   assert(Vector_isConsistent(this));

   Vector_checkArraySize(this);
   if (idx >= this->items) {
      this->items = idx + 1;
   } else {
      if (this->owner) {
         Object* removed = this->array[idx];
         assert (removed != NULL);
         Object_delete(removed);
      }
   }
   this->array[idx] = data;
   assert(Vector_isConsistent(this));
}

/*

static void Vector_merge(Vector* this, Vector* v2) {
   int i;
   assert(Vector_isConsistent(this));

   for (i = 0; i < v2->items; i++)
      Vector_add(this, v2->array[i]);
   v2->items = 0;
   Vector_delete(v2);
   assert(Vector_isConsistent(this));
}

*/

void Vector_add(Vector* this, void* data_) {
   Object* data = data_;
   assert(Object_isA(data, this->type));
   assert(Vector_isConsistent(this));
   size_t i = this->items;
   Vector_set(this, this->items, data);
   assert(this->items == i + 1); (void)(i);
   assert(Vector_isConsistent(this));
}

ssize_t Vector_indexOf(const Vector* this, const void* search_, Object_Compare compare) {
   const Object* search = search_;
   assert(Object_isA(search, this->type));
   assert(compare);
   assert(Vector_isConsistent(this));
   for (size_t i = 0; i < this->items; i++) {
      const Object* o = this->array[i];
      assert(o);
      if (compare(search, o) == 0) {
         return (ssize_t)i;
      }
   }
   return -1;
}

void Vector_splice(Vector* this, Vector* from) {
   assert(Vector_isConsistent(this));
   assert(Vector_isConsistent(from));
   assert(!(this->owner && from->owner));

   size_t olditems = this->items;
   this->items += from->items;
   Vector_checkArraySize(this);
   for (size_t j = 0; j < from->items; j++) {
      this->array[olditems + j] = from->array[j];
   }
}
