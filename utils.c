/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mouad <mouad@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:15:42 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/16 20:49:14 by mouad            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	now_time(t_sim *sim, char *unit)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	if (strcmp(unit, "ms") == 0)
		return (tv.tv_sec * 1000L + tv.tv_usec / 1000L - sim->start_ms);
	else
		return (tv.tv_sec * 1000000L + tv.tv_usec - sim->start_micro);
}

void	ft_sleep(long ms)
{
	usleep(ms * 1000);
}

void	print_log(t_sim *sim, int id, char *msg)
{
	pthread_mutex_lock(&sim->lock);
	pthread_mutex_lock(&sim->print_lock);
	if (!sim->stop)
		printf("%ld %d %s\n", now_time(sim, "ms"), id, msg);
	pthread_mutex_unlock(&sim->print_lock);
	pthread_mutex_unlock(&sim->lock);
}

void	print_burnout(t_sim *sim, int id)
{
	pthread_mutex_lock(&sim->print_lock);
	printf("%ld %d burned out\n", now_time(sim, "ms"), id);
	pthread_mutex_unlock(&sim->print_lock);
}

int	check_stop(t_sim *sim)
{
	int	result;

	pthread_mutex_lock(&sim->lock);
	result = sim->stop;
	pthread_mutex_unlock(&sim->lock);
	return (result);
}
