#include "codexion.h"

int	main(int argc, char **argv)
{
	t_config		config;
	t_simulation	simulation;

	if (parse_arguments(argc, argv, &config) != 0)
		return (1);
	memset(&simulation, 0, sizeof(t_simulation));
	simulation.config = config;
	if (init_simulation(&simulation) != 0)
		return (1);
	if (create_coder_threads(&simulation) != 0)
	{
		set_simulation_finished(&simulation);
		join_coder_threads(&simulation);
		cleanup_simulation(&simulation);
		return (1);
	}
	if (create_monitor_thread(&simulation) != 0)
	{
		set_simulation_finished(&simulation);
		join_coder_threads(&simulation);
		cleanup_simulation(&simulation);
		return (1);
	}
	if (join_coder_threads(&simulation) != 0
		|| join_monitor_thread(&simulation) != 0)
		return (1);
	cleanup_simulation(&simulation);
	return (0);
}
