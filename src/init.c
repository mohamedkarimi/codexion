#include "codexion.h"

int	parse_arguments(int argc, char **argv, t_config *config)
{
	if (argc != 9 || !is_number(argv[1]) || !is_number(argv[2])
		|| !is_number(argv[3]) || !is_number(argv[4]) || !is_number(argv[5])
		|| !is_number(argv[6]) || !is_number(argv[7])
		|| (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0))
	{
		print_error();
		return (1);
	}
	config->number_of_coders = atoi(argv[1]);
	config->time_to_burnout = atoi(argv[2]);
	config->time_to_compile = atoi(argv[3]);
	config->time_to_debug = atoi(argv[4]);
	config->time_to_refactor = atoi(argv[5]);
	config->number_of_compiles_required = atoi(argv[6]);
	config->dongle_cooldown = atoi(argv[7]);
	config->scheduler = argv[8];
	if (config->number_of_coders <= 0 || config->time_to_burnout < 0
		|| config->time_to_compile < 0 || config->time_to_debug < 0
		|| config->time_to_refactor < 0 || config->number_of_compiles_required <= 0
		|| config->dongle_cooldown < 0)
	{
		print_error();
		return (1);
	}
	return (0);
}

static void	destroy_dongles(t_simulation *simulation, int count)
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
	int	index;

	simulation->finished = 0;
	simulation->start_time = get_time_ms();
	simulation->next_arrival_order = 0;
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
	if (heap_init(&simulation->scheduler_heap,
			simulation->config.number_of_coders) != 0)
	{
		pthread_cond_destroy(&simulation->scheduler_cond);
		pthread_mutex_destroy(&simulation->scheduler_mutex);
		pthread_mutex_destroy(&simulation->log_mutex);
		pthread_mutex_destroy(&simulation->state_mutex);
		return (1);
	}
	simulation->coders = malloc(sizeof(t_coder)
			* simulation->config.number_of_coders);
	simulation->dongles = malloc(sizeof(t_dongle)
			* simulation->config.number_of_coders);
	if (!simulation->coders || !simulation->dongles)
		return (init_failure(simulation, 0), 1);
	index = 0;
	while (index < simulation->config.number_of_coders)
	{
		simulation->coders[index].id = index + 1;
		simulation->coders[index].compile_count = 0;
		simulation->coders[index].last_compile_start = 0;
		simulation->coders[index].simulation = simulation;
		simulation->dongles[index].id = index + 1;
		simulation->dongles[index].available = 1;
		simulation->dongles[index].available_at = simulation->start_time;
		if (pthread_mutex_init(&simulation->dongles[index].mutex, NULL) != 0)
			return (init_failure(simulation, index), 1);
		if (pthread_cond_init(&simulation->dongles[index].cond, NULL) != 0)
		{
			pthread_mutex_destroy(&simulation->dongles[index].mutex);
			return (init_failure(simulation, index), 1);
		}
		index++;
	}
	index = 0;
	while (index < simulation->config.number_of_coders)
	{
		simulation->coders[index].right_dongle = &simulation->dongles[index];
		simulation->coders[index].left_dongle = &simulation->dongles[(index - 1
			+ simulation->config.number_of_coders)
			% simulation->config.number_of_coders];
		index++;
	}
	return (0);
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
