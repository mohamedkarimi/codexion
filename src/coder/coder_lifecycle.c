
#include "codexion.h"

void	wait_for_duration(t_coder *coder, long duration)
{
	t_simulation		*simulation;
	struct timeval		now;
	struct timespec		deadline;
	int					wait_status;

	simulation = coder->simulation;
	gettimeofday(&now, NULL);
	deadline.tv_sec = now.tv_sec + duration / 1000;
	deadline.tv_nsec = now.tv_usec * 1000 + (duration % 1000) * 1000000;
	if (deadline.tv_nsec >= 1000000000)
	{
		deadline.tv_sec++;
		deadline.tv_nsec -= 1000000000;
	}
	if (pthread_mutex_lock(&simulation->scheduler_mutex) != 0)
		return ;
	while (!is_simulation_finished(simulation))
	{
		wait_status = pthread_cond_timedwait(&simulation->scheduler_cond,
				&simulation->scheduler_mutex, &deadline);
		if (wait_status != 0)
			break ;
	}
	pthread_mutex_unlock(&simulation->scheduler_mutex);
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
