#include "codexion.h"

static int	all_compiled(t_sim *sim)
{
	int	i;

	if (sim->need == 0)
		return (1);
	i = 0;
	while (i < sim->n)
	{
		if (sim->coders[i].compiles < sim->need)
			return (0);
		i++;
	}
	return (1);
}

static void	wake_everyone(t_sim *sim)
{
	pthread_mutex_lock(&sim->lock);
	sim->stop = 1;
	pthread_cond_broadcast(&sim->cond);
	pthread_mutex_unlock(&sim->lock);
}

void	*monitor_thread(void *arg)
{
	t_sim	*sim;
	long	elapsed;
	int		i;

	sim = (t_sim *)arg;
	while (1)
	{
		usleep(1000);
		if (sim->stop)
			break ;
		if (all_compiled(sim))
		{
			wake_everyone(sim);
			break ;
		}
		i = 0;
		while (i < sim->n)
		{
			elapsed = now_ms(sim) - sim->coders[i].last_compile;
			if (elapsed >= sim->burnout)
			{
				print_burnout(sim, sim->coders[i].id);
				wake_everyone(sim);
				return (NULL);
			}
			i++;
		}
	}
	return (NULL);
}
