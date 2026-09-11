
#include "codexion.h"

static int	all_coders_finished(t_simulation *simulation)
{
	int	index;
	int	finished;

	pthread_mutex_lock(&simulation->state_mutex);
	index = 0;
	finished = 1;
	while (index < simulation->config.number_of_coders)
	{
		if (simulation->coders[index].compile_count
			< simulation->config.number_of_compiles_required)
		{
			finished = 0;
			break ;
		}
		index++;
	}
	pthread_mutex_unlock(&simulation->state_mutex);
	return (finished);
}

static long	burnout_deadline(t_simulation *simulation, int index)
{
	if (simulation->coders[index].compile_count == 0)
		return (simulation->start_time + simulation->config.time_to_burnout);
	return (simulation->coders[index].last_compile_start
		+ simulation->config.time_to_burnout);
}

static int	check_burnout(t_simulation *simulation)
{
	int		index;
	long	deadline;
	int		burned_out;

	index = 0;
	while (index < simulation->config.number_of_coders)
	{
		pthread_mutex_lock(&simulation->state_mutex);
		deadline = burnout_deadline(simulation, index);
		burned_out = (!simulation->finished && get_time_ms() >= deadline);
		if (burned_out)
			simulation->finished = 1;
		pthread_mutex_unlock(&simulation->state_mutex);
		if (burned_out)
		{
			wake_waiting_coders(simulation);
			log_action(&simulation->coders[index], "burned out");
			return (1);
		}
		index++;
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_simulation	*simulation;

	simulation = (t_simulation *)arg;
	while (!is_simulation_finished(simulation))
	{
		if (all_coders_finished(simulation))
		{
			set_simulation_finished(simulation);
			break ;
		}
		if (check_burnout(simulation))
			break ;
		sleep_ms(1);
	}
	return (NULL);
}
