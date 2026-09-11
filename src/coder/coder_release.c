
#include "codexion.h"

void	release_dongle(t_dongle *dongle, long cooldown)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->available_at = get_time_ms() + cooldown;
	dongle->available = 1;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void	release_both_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	ordered_dongles(coder, &first, &second);
	release_dongle(second, coder->simulation->config.dongle_cooldown);
	release_dongle(first, coder->simulation->config.dongle_cooldown);
	pthread_mutex_lock(&coder->simulation->scheduler_mutex);
	pthread_cond_broadcast(&coder->simulation->scheduler_cond);
	pthread_mutex_unlock(&coder->simulation->scheduler_mutex);
}
