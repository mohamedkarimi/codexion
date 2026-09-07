#include "codexion.h"

static int	request_before(t_request *first, t_request *second)
{
	t_simulation	*simulation;

	simulation = first->coder->simulation;
	if (strcmp(simulation->config.scheduler, "fifo") == 0)
		return (first->arrival_order < second->arrival_order);
	if (first->deadline != second->deadline)
		return (first->deadline < second->deadline);
	return (first->arrival_order < second->arrival_order);
}

static void	swap_requests(t_request **first, t_request **second)
{
	t_request	*temporary;

	temporary = *first;
	*first = *second;
	*second = temporary;
}

static void	heap_push(t_heap *heap, t_request *request)
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

static void	heap_down(t_heap *heap, int index)
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

static void	heap_remove(t_heap *heap, t_request *request)
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
			swap_requests(&heap->items[index], &heap->items[(index - 1) / 2]);
			index = (index - 1) / 2;
		}
	}
	else
		heap_down(heap, index);
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

static t_request	*new_request(t_coder *coder)
{
	t_request	*request;

	request = malloc(sizeof(t_request));
	if (!request)
		return (NULL);
	request->coder = coder;
	pthread_mutex_lock(&coder->simulation->state_mutex);
	if (coder->compile_count == 0)
		request->deadline = coder->simulation->start_time
			+ coder->simulation->config.time_to_burnout;
	else
		request->deadline = coder->last_compile_start
			+ coder->simulation->config.time_to_burnout;
	pthread_mutex_unlock(&coder->simulation->state_mutex);
	return (request);
}

int	request_compile(t_coder *coder)
{
	t_simulation	*simulation;
	t_request		*request;
	int			result;

	simulation = coder->simulation;
	request = new_request(coder);
	if (!request || pthread_mutex_lock(&simulation->scheduler_mutex) != 0)
	{
		free(request);
		return (1);
	}
	if (is_simulation_finished(simulation) || simulation->scheduler_heap.size
		>= simulation->scheduler_heap.capacity)
	{
		pthread_mutex_unlock(&simulation->scheduler_mutex);
		free(request);
		return (1);
	}
	request->arrival_order = simulation->next_arrival_order++;
	heap_push(&simulation->scheduler_heap, request);
	while (!is_simulation_finished(simulation)
		&& simulation->scheduler_heap.items[0] != request)
	{
		if (pthread_cond_wait(&simulation->scheduler_cond,
				&simulation->scheduler_mutex) != 0)
			break ;
	}
	if (is_simulation_finished(simulation)
		|| simulation->scheduler_heap.items[0] != request)
	{
		heap_remove(&simulation->scheduler_heap, request);
		pthread_cond_broadcast(&simulation->scheduler_cond);
		pthread_mutex_unlock(&simulation->scheduler_mutex);
		free(request);
		return (1);
	}
	pthread_mutex_unlock(&simulation->scheduler_mutex);
	result = take_both_dongles(coder);
	pthread_mutex_lock(&simulation->scheduler_mutex);
	heap_remove(&simulation->scheduler_heap, request);
	pthread_cond_broadcast(&simulation->scheduler_cond);
	pthread_mutex_unlock(&simulation->scheduler_mutex);
	free(request);
	return (result);
}
