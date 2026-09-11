
#include "codexion.h"

static int	init_mutexes(t_simulation *simulation)
{
	if (pthread_mutex_init(&simulation->state_mutex, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&simulation->log_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->state_mutex);
		return (1);
	}
	if (pthread_mutex_init(&simulation->scheduler_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->log_mutex);
		pthread_mutex_destroy(&simulation->state_mutex);
		return (1);
	}
	if (pthread_cond_init(&simulation->scheduler_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&simulation->scheduler_mutex);
		pthread_mutex_destroy(&simulation->log_mutex);
		pthread_mutex_destroy(&simulation->state_mutex);
		return (1);
	}
	return (0);
}

static void	init_failure(t_simulation *simulation, int dongles_ready)
{
	if (simulation->dongles)
		destroy_dongles(simulation, dongles_ready);
	free(simulation->coders);
	simulation->coders = NULL;
	heap_destroy(&simulation->scheduler_heap);
	pthread_cond_destroy(&simulation->scheduler_cond);
	pthread_mutex_destroy(&simulation->scheduler_mutex);
	pthread_mutex_destroy(&simulation->log_mutex);
	pthread_mutex_destroy(&simulation->state_mutex);
}

int	init_simulation(t_simulation *simulation)
{
	simulation->finished = 0;
	simulation->start_time = get_time_ms();
	simulation->next_arrival_order = 0;
	if (init_mutexes(simulation) != 0)
		return (1);
	if (heap_init(&simulation->scheduler_heap,
			simulation->config.number_of_coders) != 0)
	{
		init_failure(simulation, 0);
		return (1);
	}
	if (init_coders_and_dongles(simulation) != 0)
	{
		init_failure(simulation, 0);
		return (1);
	}
	link_coders_to_dongles(simulation);
	return (0);
}
