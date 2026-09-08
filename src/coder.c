#include "codexion.h"

static void	release_dongle(t_dongle *dongle, long cooldown);
static int	coder_compile(t_coder *coder);
static int	coder_needs_compile(t_coder *coder);
static void	coder_debug(t_coder *coder);
static void	coder_refactor(t_coder *coder);

static void	wait_for_duration(t_coder *coder, long duration)
{
	t_simulation		*simulation;
	struct timeval	now;
	struct timespec	deadline;
	int				wait_status;

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

static int	wait_for_cooldown(t_dongle *dongle, long available_at)
{
	struct timeval	now;
	struct timespec	deadline;
	long			wait_time;

	gettimeofday(&now, NULL);
	wait_time = available_at - (now.tv_sec * 1000 + now.tv_usec / 1000);
	if (wait_time < 1)
		wait_time = 1;
	deadline.tv_sec = now.tv_sec + wait_time / 1000;
	deadline.tv_nsec = now.tv_usec * 1000 + (wait_time % 1000) * 1000000;
	if (deadline.tv_nsec >= 1000000000)
	{
		deadline.tv_sec++;
		deadline.tv_nsec -= 1000000000;
	}
	return (pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &deadline));
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
		coder_debug(coder);
		if (is_simulation_finished(coder->simulation))
			break ;
		coder_refactor(coder);
		pthread_mutex_lock(&coder->simulation->state_mutex);
		if (!coder->simulation->finished)
			coder->compile_count++;
		pthread_mutex_unlock(&coder->simulation->state_mutex);
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

static int	take_dongle(t_dongle *dongle, t_simulation *simulation)
{
	int	wait_status;
	long	now;

	if (pthread_mutex_lock(&dongle->mutex) != 0)
		return (1);
	while (!is_simulation_finished(simulation))
	{
		now = get_time_ms();
		if (dongle->available && now >= dongle->available_at)
		{
			dongle->available = 0;
			return (0);
		}
		if (dongle->available)
			wait_status = wait_for_cooldown(dongle, dongle->available_at);
		else
			wait_status = pthread_cond_wait(&dongle->cond, &dongle->mutex);
		if (wait_status != 0 && get_time_ms() < dongle->available_at)
		{
			pthread_mutex_unlock(&dongle->mutex);
			return (1);
		}
	}
	pthread_mutex_unlock(&dongle->mutex);
	return (1);
}

static void	ordered_dongles(t_coder *coder, t_dongle **first,
		t_dongle **second)
{
	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		*first = coder->left_dongle;
		*second = coder->right_dongle;
	}
	else
	{
		*first = coder->right_dongle;
		*second = coder->left_dongle;
	}
}

int	take_both_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left_dongle == coder->right_dongle)
		return (take_dongle(coder->left_dongle, coder->simulation));
	ordered_dongles(coder, &first, &second);
	if (take_dongle(first, coder->simulation) != 0)
		return (1);
	if (take_dongle(second, coder->simulation) != 0)
	{
		release_dongle(first, coder->simulation->config.dongle_cooldown);
		return (1);
	}
	return (0);
}

static void	release_dongle(t_dongle *dongle, long cooldown)
{
	dongle->available_at = get_time_ms() + cooldown;
	dongle->available = 1;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

static void	release_both_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left_dongle == coder->right_dongle)
	{
		release_dongle(coder->left_dongle,
			coder->simulation->config.dongle_cooldown);
		return ;
	}
	ordered_dongles(coder, &first, &second);
	release_dongle(second, coder->simulation->config.dongle_cooldown);
	release_dongle(first, coder->simulation->config.dongle_cooldown);
}

static int	coder_compile(t_coder *coder)
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

static int	coder_needs_compile(t_coder *coder)
{
	int	compile_count;

	pthread_mutex_lock(&coder->simulation->state_mutex);
	compile_count = coder->compile_count;
	pthread_mutex_unlock(&coder->simulation->state_mutex);
	return (compile_count < coder->simulation->config.number_of_compiles_required);
}

static void	coder_debug(t_coder *coder)
{
	log_action(coder, "is debugging");
	wait_for_duration(coder, coder->simulation->config.time_to_debug);
}

static void	coder_refactor(t_coder *coder)
{
	log_action(coder, "is refactoring");
	wait_for_duration(coder, coder->simulation->config.time_to_refactor);
}

int	create_monitor_thread(t_simulation *simulation)
{
	if (pthread_create(&simulation->monitor_thread, NULL, monitor_routine,
			simulation) != 0)
		return (1);
	simulation->monitor_created = 1;
	return (0);
}
