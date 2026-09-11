
#include "codexion.h"

void	ordered_dongles(t_coder *coder, t_dongle **first, t_dongle **second)
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

int	dongles_available(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	int			ready;

	if (coder->left_dongle == coder->right_dongle)
		return (0);
	ordered_dongles(coder, &first, &second);
	pthread_mutex_lock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	ready = (first->available && second->available
			&& get_time_ms() >= first->available_at
			&& get_time_ms() >= second->available_at);
	pthread_mutex_unlock(&second->mutex);
	pthread_mutex_unlock(&first->mutex);
	return (ready);
}

int	take_both_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left_dongle == coder->right_dongle)
		return (1);
	ordered_dongles(coder, &first, &second);
	pthread_mutex_lock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	if (!first->available || !second->available
		|| get_time_ms() < first->available_at
		|| get_time_ms() < second->available_at)
	{
		pthread_mutex_unlock(&second->mutex);
		pthread_mutex_unlock(&first->mutex);
		return (1);
	}
	first->available = 0;
	second->available = 0;
	pthread_mutex_unlock(&second->mutex);
	pthread_mutex_unlock(&first->mutex);
	return (0);
}

long	next_dongle_cooldown(t_simulation *simulation)
{
	long	next;
	int		index;

	next = 0;
	index = 0;
	while (index < simulation->config.number_of_coders)
	{
		pthread_mutex_lock(&simulation->dongles[index].mutex);
		if (simulation->dongles[index].available
			&& simulation->dongles[index].available_at > get_time_ms()
			&& (!next || simulation->dongles[index].available_at < next))
			next = simulation->dongles[index].available_at;
		pthread_mutex_unlock(&simulation->dongles[index].mutex);
		index++;
	}
	return (next);
}
