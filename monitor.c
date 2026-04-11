/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mouad <mouad@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:15:39 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/07 11:36:06 by mouad            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

static int	check_burnout(t_sim *sim)
{
	int		i;
	int		id;
	long	elapsed;

	pthread_mutex_lock(&sim->lock);
	i = 0;
	while (i < sim->n)
	{
		elapsed = now_ms(sim) - sim->coders[i].last_compile;
		if (elapsed >= sim->burnout)
		{
			id = sim->coders[i].id;
			pthread_mutex_unlock(&sim->lock);
			wake_everyone(sim);
			print_burnout(sim, id);
			return (1);
		}
		i++;
	}
	pthread_mutex_unlock(&sim->lock);
	return (0);
}

void	*monitor_thread(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (1)
	{
		usleep(1000);
		pthread_mutex_lock(&sim->lock);
		if (sim->stop)
			return (pthread_mutex_unlock(&sim->lock), NULL);
		if (all_compiled(sim))
		{
			pthread_mutex_unlock(&sim->lock);
			wake_everyone(sim);
			return (NULL);
		}
		pthread_mutex_unlock(&sim->lock);
		if (check_burnout(sim))
			break ;
	}
	return (NULL);
}
