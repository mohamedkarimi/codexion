
#include "codexion.h"

void	heap_remove(t_heap *heap, t_request *request)
{
	int	index;

	index = 0;
	while (index < heap->size && heap->items[index] != request)
		index++;
	if (index == heap->size)
		return ;
	heap->size--;
	if (index == heap->size)
		return ;
	heap->items[index] = heap->items[heap->size];
	if (index > 0 && request_before(heap->items[index],
			heap->items[(index - 1) / 2]))
	{
		while (index > 0 && request_before(heap->items[index],
				heap->items[(index - 1) / 2]))
		{
			swap_requests(&heap->items[index],
				&heap->items[(index - 1) / 2]);
			index = (index - 1) / 2;
		}
	}
	else
		heap_down(heap, index);
}

void	heap_destroy(t_heap *heap)
{
	int	index;

	index = 0;
	while (index < heap->size)
		free(heap->items[index++]);
	free(heap->items);
	heap->items = NULL;
	heap->size = 0;
	heap->capacity = 0;
}
