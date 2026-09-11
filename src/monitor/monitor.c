
#include "codexion.h"

void	wake_waiting_coders(t_simulation *simulation)
{
	int	index;

	pthread_mutex_lock(&simulation->scheduler_mutex);
	pthread_cond_broadcast(&simulation->scheduler_cond);
	pthread_mutex_unlock(&simulation->scheduler_mutex);
	index = 0;
	while (index < simulation->config.number_of_coders)
	{
		pthread_mutex_lock(&simulation->dongles[index].mutex);
		pthread_cond_broadcast(&simulation->dongles[index].cond);
		pthread_mutex_unlock(&simulation->dongles[index].mutex);
		index++;
	}
}

int	is_simulation_finished(t_simulation *simulation)
{
	int	finished;

	pthread_mutex_lock(&simulation->state_mutex);
	finished = simulation->finished;
	pthread_mutex_unlock(&simulation->state_mutex);
	return (finished);
}

void	set_simulation_finished(t_simulation *simulation)
{
	pthread_mutex_lock(&simulation->state_mutex);
	simulation->finished = 1;
	pthread_mutex_unlock(&simulation->state_mutex);
	wake_waiting_coders(simulation);
}

int	create_monitor_thread(t_simulation *simulation)
{
	if (pthread_create(&simulation->monitor_thread, NULL, monitor_routine,
			simulation) != 0)
		return (1);
	simulation->monitor_created = 1;
	return (0);
}
