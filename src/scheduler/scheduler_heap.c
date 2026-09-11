
#include "codexion.h"

int	request_before(t_request *first, t_request *second)
{
	t_simulation	*simulation;

	simulation = first->coder->simulation;
	if (strcmp(simulation->config.scheduler, "fifo") == 0)
		return (first->arrival_order < second->arrival_order);
	if (first->deadline != second->deadline)
		return (first->deadline < second->deadline);
	return (first->arrival_order < second->arrival_order);
}

void	swap_requests(t_request **first, t_request **second)
{
	t_request	*temporary;

	temporary = *first;
	*first = *second;
	*second = temporary;
}

void	heap_push(t_heap *heap, t_request *request)
{
	int	index;

	index = heap->size++;
	heap->items[index] = request;
	while (index > 0 && request_before(heap->items[index],
			heap->items[(index - 1) / 2]))
	{
		swap_requests(&heap->items[index], &heap->items[(index - 1) / 2]);
		index = (index - 1) / 2;
	}
}

void	heap_down(t_heap *heap, int index)
{
	int	child;

	while (index * 2 + 1 < heap->size)
	{
		child = index * 2 + 1;
		if (child + 1 < heap->size && request_before(heap->items[child + 1],
				heap->items[child]))
			child++;
		if (request_before(heap->items[index], heap->items[child]))
			break ;
		swap_requests(&heap->items[index], &heap->items[child]);
		index = child;
	}
}

int	heap_init(t_heap *heap, int capacity)
{
	heap->items = malloc(sizeof(t_request *) * capacity);
	if (!heap->items)
		return (1);
	heap->size = 0;
	heap->capacity = capacity;
	return (0);
}
