
#include "codexion.h"

void	scheduler_wait_for_resources(t_simulation *simulation)
{
	struct timespec	deadline;
	long			end;

	end = next_dongle_cooldown(simulation);
	if (!end)
	{
		pthread_cond_wait(&simulation->scheduler_cond,
			&simulation->scheduler_mutex);
		return ;
	}
	deadline.tv_sec = end / 1000;
	deadline.tv_nsec = (end % 1000) * 1000000;
	pthread_cond_timedwait(&simulation->scheduler_cond,
		&simulation->scheduler_mutex, &deadline);
}
