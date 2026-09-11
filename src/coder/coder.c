
#include "codexion.h"

static void	finish_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->simulation->state_mutex);
	if (!coder->simulation->finished)
		coder->compile_count++;
	pthread_mutex_unlock(&coder->simulation->state_mutex);
}

static void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (!is_simulation_finished(coder->simulation)
		&& coder_needs_compile(coder))
	{
		if (coder_compile(coder) != 0)
			break ;
		if (is_simulation_finished(coder->simulation))
			break ;
		coder_pause(coder, "is debugging",
			coder->simulation->config.time_to_debug);
		if (is_simulation_finished(coder->simulation))
			break ;
		coder_pause(coder, "is refactoring",
			coder->simulation->config.time_to_refactor);
		finish_compile(coder);
	}
	return (NULL);
}

int	create_coder_threads(t_simulation *simulation)
{
	int	index;

	index = 0;
	while (index < simulation->config.number_of_coders)
	{
		if (pthread_create(&simulation->coders[index].thread, NULL,
				coder_routine, &simulation->coders[index]) != 0)
			return (1);
		simulation->coder_threads_created++;
		index++;
	}
	return (0);
}

int	join_coder_threads(t_simulation *simulation)
{
	int	index;

	index = 0;
	while (index < simulation->coder_threads_created)
	{
		if (pthread_join(simulation->coders[index].thread, NULL) != 0)
			return (1);
		index++;
	}
	simulation->coder_threads_created = 0;
	return (0);
}

int	join_monitor_thread(t_simulation *simulation)
{
	if (simulation->monitor_created
		&& pthread_join(simulation->monitor_thread, NULL) != 0)
		return (1);
	simulation->monitor_created = 0;
	return (0);
}
