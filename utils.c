/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhadir <mhadir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:15:42 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/04 12:15:43 by mhadir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	now_ms(t_sim *sim)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000L + tv.tv_usec / 1000L - sim->start);
}

void	ft_sleep(long ms)
{
	usleep(ms * 1000);
}

void	print_log(t_sim *sim, int id, char *msg)
{
	pthread_mutex_lock(&sim->print_lock);
	if (!sim->stop)
		printf("%ld %d %s\n", now_ms(sim), id, msg);
	pthread_mutex_unlock(&sim->print_lock);
}

void	print_burnout(t_sim *sim, int id)
{
	pthread_mutex_lock(&sim->print_lock);
	printf("%ld %d burned out\n", now_ms(sim), id);
	pthread_mutex_unlock(&sim->print_lock);
}

int	is_done(t_sim *sim)
{
	int	result;

	pthread_mutex_lock(&sim->lock);
	result = sim->stop;
	pthread_mutex_unlock(&sim->lock);
	return (result);
}
