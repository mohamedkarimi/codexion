
#include "codexion.h"

void	wait_for_duration(t_coder *coder, long duration)
{
	t_simulation		*simulation;
	long				end;

	simulation = coder->simulation;
	end = get_time_ms() + duration;
	while (get_time_ms() < end && !is_simulation_finished(simulation))
	{
		sleep_ms(1);
	}
}

int	coder_compile(t_coder *coder)
{
	int	finished;

	if (request_compile(coder) != 0)
		return (1);
	pthread_mutex_lock(&coder->simulation->state_mutex);
	finished = coder->simulation->finished;
	if (!finished)
		coder->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&coder->simulation->state_mutex);
	if (finished)
	{
		release_both_dongles(coder);
		return (1);
	}
	log_action(coder, "has taken a dongle");
	if (coder->left_dongle != coder->right_dongle)
		log_action(coder, "has taken a dongle");
	log_action(coder, "is compiling");
	wait_for_duration(coder, coder->simulation->config.time_to_compile);
	pthread_mutex_lock(&coder->simulation->state_mutex);
	finished = coder->simulation->finished;
	pthread_mutex_unlock(&coder->simulation->state_mutex);
	release_both_dongles(coder);
	return (finished);
}

int	coder_needs_compile(t_coder *coder)
{
	int	compile_count;

	pthread_mutex_lock(&coder->simulation->state_mutex);
	compile_count = coder->compile_count;
	pthread_mutex_unlock(&coder->simulation->state_mutex);
	return (compile_count
		< coder->simulation->config.number_of_compiles_required);
}

void	coder_pause(t_coder *coder, char *message, long duration)
{
	log_action(coder, message);
	wait_for_duration(coder, duration);
}
