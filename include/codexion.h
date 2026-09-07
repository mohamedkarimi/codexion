#ifndef CODEXION_H
#define CODEXION_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

typedef struct s_coder	t_coder;
typedef struct s_dongle	t_dongle;
typedef struct s_simulation	t_simulation;
typedef struct s_request	t_request;

typedef struct s_config
{
    int		number_of_coders;
    long	time_to_burnout;
    long	time_to_compile;
    long	time_to_debug;
    long	time_to_refactor;
    int		number_of_compiles_required;
    long	dongle_cooldown;
    char	*scheduler;
}   t_config;

struct s_coder
{
    int			id;
    int			compile_count;
    long		last_compile_start;
    pthread_t		thread;
    t_dongle		*left_dongle;
    t_dongle		*right_dongle;
    t_simulation	*simulation;
};

typedef struct s_dongle
{
    int			id;
    int			available;
    long		available_at;
    pthread_mutex_t	mutex;
    pthread_cond_t	cond;
}   t_dongle;

typedef struct s_request
{
	t_coder			*coder;
	unsigned long	arrival_order;
	long			deadline;
}   t_request;

typedef struct s_heap
{
	t_request	**items;
	int			size;
	int			capacity;
}   t_heap;

typedef struct s_simulation
{
    int			finished;
    t_config		config;
    t_coder			*coders;
    t_dongle			*dongles;
    pthread_mutex_t	state_mutex;
    pthread_t		monitor_thread;
    long			start_time;
    pthread_mutex_t	log_mutex;
    pthread_mutex_t	scheduler_mutex;
    pthread_cond_t	scheduler_cond;
    t_heap			scheduler_heap;
    unsigned long	next_arrival_order;
    int			coder_threads_created;
    int			monitor_created;
}   t_simulation;
int	parse_arguments(int argc, char **argv, t_config *config);
int	is_number(char *str);
void	print_error(void);
int	init_simulation(t_simulation *simulation);
void	cleanup_simulation(t_simulation *simulation);
int	create_coder_threads(t_simulation *simulation);
int	join_coder_threads(t_simulation *simulation);
int	is_simulation_finished(t_simulation *simulation);
void	set_simulation_finished(t_simulation *simulation);
int	take_both_dongles(t_coder *coder);
int	request_compile(t_coder *coder);
int	heap_init(t_heap *heap, int capacity);
void	heap_destroy(t_heap *heap);
long	get_time_ms(void);
void	sleep_ms(long milliseconds);
void	log_action(t_coder *coder, char *message);
void	*monitor_routine(void *arg);
int	create_monitor_thread(t_simulation *simulation);
int	join_monitor_thread(t_simulation *simulation);

#endif
