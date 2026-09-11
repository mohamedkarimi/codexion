
#include "codexion.h"

void	destroy_dongles(t_simulation *simulation, int count)
{
	while (count > 0)
	{
		count--;
		pthread_cond_destroy(&simulation->dongles[count].cond);
		pthread_mutex_destroy(&simulation->dongles[count].mutex);
	}
	free(simulation->dongles);
	simulation->dongles = NULL;
}

void	cleanup_simulation(t_simulation *simulation)
{
	destroy_dongles(simulation, simulation->config.number_of_coders);
	free(simulation->coders);
	simulation->coders = NULL;
	heap_destroy(&simulation->scheduler_heap);
	pthread_cond_destroy(&simulation->scheduler_cond);
	pthread_mutex_destroy(&simulation->scheduler_mutex);
	pthread_mutex_destroy(&simulation->log_mutex);
	pthread_mutex_destroy(&simulation->state_mutex);
}
