/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhadir <mhadir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:15:39 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/18 20:26:57 by mhadir           ###   ########.fr       */
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

static void	stop_all(t_sim *sim)
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
	long	time_passed;

	pthread_mutex_lock(&sim->lock);
	i = 0;
	while (i < sim->n)
	{
		time_passed = now_time(sim, "ms") - sim->coders[i].last_compile;
		if (time_passed >= sim->burnout)
		{
			id = sim->coders[i].id;
			pthread_mutex_unlock(&sim->lock);
			stop_all(sim);
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
		ft_sleep(1);
		pthread_mutex_lock(&sim->lock);
		if (sim->stop)
			return (pthread_mutex_unlock(&sim->lock), NULL);
		if (all_compiled(sim))
		{
			pthread_mutex_unlock(&sim->lock);
			stop_all(sim);
			return (NULL);
		}
		pthread_mutex_unlock(&sim->lock);
		if (check_burnout(sim))
			break ;
	}
	return (NULL);
}
