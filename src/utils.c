#include "codexion.h"

int	is_number(char *str)
{
	int	i;

	if (!str || !str[0])
		return (0);
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

void	print_error(void)
{
	fprintf(stderr, "Error: invalid arguments\n");
}

long	get_time_ms(void)
{
    struct timeval	time;

    gettimeofday(&time, NULL);
    return (time.tv_sec * 1000 + time.tv_usec / 1000);
}

void	sleep_ms(long milliseconds)
{
    usleep(milliseconds * 1000);
}

void	log_action(t_coder *coder, char *message)
{
    long	timestamp;

    timestamp = get_time_ms() - coder->simulation->start_time;
    pthread_mutex_lock(&coder->simulation->log_mutex);
    printf("%ld %d %s\n", timestamp, coder->id, message);
    pthread_mutex_unlock(&coder->simulation->log_mutex);
}
