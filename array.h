#ifndef _H_ARRAY
#define _H_ARRAY

#define INDEX(a, i) (a.data + (a.item_size * i))

typedef struct Array{
	void *data;
	int byte_count;

	int item_size;
	int item_count;
} Array;

Array make_array(int item_size);
void add_item(Array *a, void *item);
int pop_item(Array *a);
void* get_item(Array *a, unsigned int index);


void free_array(Array *a);
typedef struct Array_Iter{
	Array *array;
	int index;
} Array_Iter;

Array_Iter make_array_iter(Array *a);

int has_next_item(Array_Iter *at);
void* next_item(Array_Iter *at);


#endif
