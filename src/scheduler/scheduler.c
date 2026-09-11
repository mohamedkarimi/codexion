
#include "codexion.h"

static t_request	*new_request(t_coder *coder)
{
	t_request	*request;

	request = malloc(sizeof(t_request));
	if (!request)
		return (NULL);
	request->coder = coder;
	pthread_mutex_lock(&coder->simulation->state_mutex);
	if (coder->compile_count == 0)
		request->deadline = coder->simulation->start_time
			+ coder->simulation->config.time_to_burnout;
	else
		request->deadline = coder->last_compile_start
			+ coder->simulation->config.time_to_burnout;
	pthread_mutex_unlock(&coder->simulation->state_mutex);
	return (request);
}

static t_request	*next_ready_request(t_simulation *simulation)
{
	t_request	*next;
	int			index;

	next = NULL;
	index = 0;
	while (index < simulation->scheduler_heap.size)
	{
		if (dongles_available(simulation->scheduler_heap.items[index]->coder))
		{
			if (!next || request_before(simulation->scheduler_heap.items[index],
					next))
				next = simulation->scheduler_heap.items[index];
		}
		index++;
	}
	return (next);
}

static int	wait_for_turn(t_simulation *simulation, t_request *request)
{
	while (!is_simulation_finished(simulation))
	{
		if (next_ready_request(simulation) == request
			&& take_both_dongles(request->coder) == 0)
			return (0);
		scheduler_wait_for_resources(simulation);
	}
	return (1);
}

int	request_compile(t_coder *coder)
{
	t_simulation	*simulation;
	t_request		*request;
	int				result;

	simulation = coder->simulation;
	request = new_request(coder);
	if (!request || scheduler_add_request(simulation, request) != 0)
	{
		free(request);
		return (1);
	}
	if (wait_for_turn(simulation, request) != 0)
	{
		scheduler_remove_request(simulation, request);
		return (1);
	}
	result = 0;
	scheduler_remove_request(simulation, request);
	return (result);
}
