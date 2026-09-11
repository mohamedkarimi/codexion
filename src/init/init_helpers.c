
#include "codexion.h"

static void	init_coder(t_coder *coder, t_simulation *simulation, int index)
{
	coder->id = index + 1;
	coder->compile_count = 0;
	coder->last_compile_start = 0;
	coder->simulation = simulation;
}

static int	init_dongle(t_dongle *dongle, long start_time, int index)
{
	dongle->id = index + 1;
	dongle->available = 1;
	dongle->available_at = start_time;
	if (pthread_mutex_init(&dongle->mutex, NULL) != 0)
		return (1);
	if (pthread_cond_init(&dongle->cond, NULL) != 0)
	{
		pthread_mutex_destroy(&dongle->mutex);
		return (1);
	}
	return (0);
}

int	init_coders_and_dongles(t_simulation *simulation)
{
	int	index;

	simulation->coders = malloc(sizeof(t_coder)
			* simulation->config.number_of_coders);
	simulation->dongles = malloc(sizeof(t_dongle)
			* simulation->config.number_of_coders);
	if (!simulation->coders || !simulation->dongles)
		return (1);
	index = 0;
	while (index < simulation->config.number_of_coders)
	{
		init_coder(&simulation->coders[index], simulation, index);
		if (init_dongle(&simulation->dongles[index], simulation->start_time,
				index) != 0)
		{
			destroy_dongles(simulation, index);
			free(simulation->coders);
			simulation->coders = NULL;
			return (1);
		}
		index++;
	}
	return (0);
}

void	link_coders_to_dongles(t_simulation *simulation)
{
	int	index;

	index = 0;
	while (index < simulation->config.number_of_coders)
	{
		simulation->coders[index].right_dongle = &simulation->dongles[index];
		simulation->coders[index].left_dongle = &simulation->dongles[(index - 1
				+ simulation->config.number_of_coders)
			% simulation->config.number_of_coders];
		index++;
	}
}
