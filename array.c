#include <stdlib.h>
#include <string.h>

#include "array.h"

Array make_array(int item_size){
	Array a;
	a.data = malloc(item_size*128);
	a.byte_count = item_size*128;
	a.item_size = item_size;
	a.item_count = 0;

	return a;
}

void add_item(Array *a, void *item){
	if( (a->item_count * a->item_size) > a->byte_count){
		a->byte_count *= 2;
		a->data = realloc(a->data, a->byte_count);
	}
	memcpy((a->data + a->item_count * a->item_size), item, a->item_size);
	a->item_count++;
}

//1 if popped, o if not
int pop_item(Array *a){
	if(a->item_count > 0){
		a->item_count--;
		return 1;
	}
	return 0;
}

Array_Iter make_array_iter(Array *a){
	Array_Iter at;
	at.array = a;
	at.index = 0;

	return at;
}

int has_next_item(Array_Iter *at){
	return at->index < at->array->item_count;
}

void* next_item(Array_Iter *at){
	if(!has_next_item(at)) return NULL;

	void *ret = at->array->data + (at->index * at->array->item_size);
	at->index++;
	return ret;
}
