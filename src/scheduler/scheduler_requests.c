
#include "codexion.h"

int	scheduler_add_request(t_simulation *simulation, t_request *request)
{
	if (pthread_mutex_lock(&simulation->scheduler_mutex) != 0)
		return (1);
	if (is_simulation_finished(simulation) || simulation->scheduler_heap.size
		>= simulation->scheduler_heap.capacity)
	{
		pthread_mutex_unlock(&simulation->scheduler_mutex);
		return (1);
	}
	request->arrival_order = simulation->next_arrival_order++;
	heap_push(&simulation->scheduler_heap, request);
	return (0);
}

void	scheduler_remove_request(t_simulation *simulation, t_request *request)
{
	heap_remove(&simulation->scheduler_heap, request);
	pthread_cond_broadcast(&simulation->scheduler_cond);
	pthread_mutex_unlock(&simulation->scheduler_mutex);
	free(request);
}
