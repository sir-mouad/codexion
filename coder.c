/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mouad <mouad@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:15:16 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/15 16:29:14 by mouad            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	can_take(t_sim *sim, int id)
{
	int		left;
	int		right;
	int		ln;
	int		rn;
	long	ms;

	left = sim->coders[id].left;
	right = sim->coders[id].right;
	ln = (id - 1 + sim->n) % sim->n;
	rn = (id + 1) % sim->n;
	ms = now_ms(sim);
	if (sim->dongle_taken[left] || ms < sim->dongle_free_at[left])
		return (0);
	if (sim->dongle_taken[right] || ms < sim->dongle_free_at[right])
		return (0);
	if (!can_go_first(sim, id, ln))
		return (0);
	if (!can_go_first(sim, id, rn))
		return (0);
	return (1);
}

static int	lock_dongles(t_coder *coder, t_sim *sim, int id)
{
	if (can_take(sim, id))
	{
		coder->state = "COMPILING";
		coder->last_compile = now_ms(sim);
		sim->dongle_taken[coder->left] = 1;
		sim->dongle_taken[coder->right] = 1;
		return (1);
	}
	return (0);
}

static int	take_dongles(t_coder *coder)
{
	t_sim			*sim;
	struct timespec	ts;
	struct timeval	tv;

	sim = coder->sim;
	pthread_mutex_lock(&sim->lock);
	coder->state = "WAITING";
	coder->waiting_since = now_ms(sim);
	while (!sim->stop)
	{
		if (lock_dongles(coder, sim, coder->id - 1))
			return (pthread_mutex_unlock(&sim->lock), 1);
		gettimeofday(&tv, NULL);
		ts.tv_sec = tv.tv_sec;
		ts.tv_nsec = (tv.tv_usec + 2000) * 1000;
		if (ts.tv_nsec >= 1000000000L)
		{
			ts.tv_sec += 1;
			ts.tv_nsec -= 1000000000L;
		}
		pthread_cond_timedwait(&sim->cond, &sim->lock, &ts);
	}
	pthread_mutex_unlock(&sim->lock);
	return (0);
}

static int	coder_cycle(t_coder *coder, t_sim *sim)
{
	if (!take_dongles(coder))
		return (0);
	print_log(sim, coder->id, "has taken a dongle");
	print_log(sim, coder->id, "has taken a dongle");
	print_log(sim, coder->id, "is compiling");
	ft_sleep(sim->t_compile);
	pthread_mutex_lock(&sim->lock);
	coder->compiles++;
	pthread_mutex_unlock(&sim->lock);
	unlock_dongles(coder);
	if (check_stop(sim))
		return (0);
	pthread_mutex_lock(&sim->lock);
	coder->state = "DEBUGGING";
	pthread_mutex_unlock(&sim->lock);
	print_log(sim, coder->id, "is debugging");
	ft_sleep(sim->t_debug);
	if (check_stop(sim))
		return (0);
	pthread_mutex_lock(&sim->lock);
	coder->state = "REFACTORING";
	pthread_mutex_unlock(&sim->lock);
	print_log(sim, coder->id, "is refactoring");
	ft_sleep(sim->t_refactor);
	return (1);
}

void	*coder_thread(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;
	if (sim->n == 1)
	{
		while (!check_stop(sim))
			ft_sleep(10);
		return (NULL);
	}
	while (!check_stop(sim))
	{
		if (!coder_cycle(coder, sim))
			break ;
	}
	return (NULL);
}
