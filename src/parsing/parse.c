
#include "codexion.h"

static int	valid_arguments(int argc, char **argv)
{
	if (argc != 9 || !is_number(argv[1]) || !is_number(argv[2])
		|| !is_number(argv[3]) || !is_number(argv[4]) || !is_number(argv[5])
		|| !is_number(argv[6]) || !is_number(argv[7]))
		return (0);
	if (strcmp(argv[8], "fifo") != 0 && strcmp(argv[8], "edf") != 0)
		return (0);
	return (1);
}

static int	valid_config(t_config *config)
{
	if (config->number_of_coders <= 0 || config->time_to_burnout < 0
		|| config->time_to_compile < 0 || config->time_to_debug < 0
		|| config->time_to_refactor < 0
		|| config->number_of_compiles_required <= 0
		|| config->dongle_cooldown < 0)
		return (0);
	return (1);
}

int	parse_arguments(int argc, char **argv, t_config *config)
{
	if (!valid_arguments(argc, argv))
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
	if (!valid_config(config))
	{
		print_error();
		return (1);
	}
	return (0);
}
