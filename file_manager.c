#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define MAX_THREAD_CAPACITY 5
#define MAX_FILE_CAPACITY 10
#define TRUE 1
#define FALSE 0

typedef struct
{
	int size;
	int capacity;
	void **items; // array of void pointers
} ArrayList;

ArrayList *create_list(int maxCapacity);
void destroy_list(ArrayList *list);
int add_item(ArrayList *list, void *item);
void remove_item(ArrayList *list, int index);
void *get_item(ArrayList *list, int index);

int main(int argc, char const *argv[])
{
	ArrayList *threadList = create_list(MAX_THREAD_CAPACITY);
	ArrayList *fileList = create_list(MAX_FILE_CAPACITY);

	destroy_list(threadList);
	destroy_list(fileList);

	return 0;
}

ArrayList *create_list(int maxCapacity)
{
	ArrayList *list = malloc(sizeof(ArrayList));
	list->size = 0;
	list->capacity = maxCapacity;
	list->items = malloc(sizeof(void *) * list->capacity);

	for (int i = 0; i < list->capacity; i++)
	{
		list->items[i] = NULL;
	}

	return list;
}

void destroy_list(ArrayList *list)
{
	free(list->items);
	free(list);
}

int add_item(ArrayList *list, void *item)
{
	for (int i = 0; i < list->capacity; i++)
	{
		if (list->items[i] == NULL)
		{
			list->items[i] = item;
			return 1;
		}
	}
	return 0;
}

void remove_item(ArrayList *list, int index)
{
	list->items[index] = NULL;
}

void *get_item(ArrayList *list, int index)
{
	return list->items[index];
}
