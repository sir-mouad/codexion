/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhadir <mhadir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:15:16 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/17 18:44:35 by mhadir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	can_take(t_sim *sim, int idx)
{
	int		left;
	int		right;
	long	ms;

	left = sim->coders[idx].left;
	right = sim->coders[idx].right;
	ms = now_time(sim, "ms");
	if (left < right)
	{
		pthread_mutex_lock(&sim->dongle_lock[left]);
		pthread_mutex_lock(&sim->dongle_lock[right]);
		if (sim->dongle_taken[left] || ms < sim->dongle_free_at[left])
			return (pthread_mutex_unlock(&sim->dongle_lock[right]),
					pthread_mutex_unlock(&sim->dongle_lock[left]), 0);
		if (sim->dongle_taken[right] || ms < sim->dongle_free_at[right])
			return (pthread_mutex_unlock(&sim->dongle_lock[right]),
					pthread_mutex_unlock(&sim->dongle_lock[left]), 0);
		pthread_mutex_unlock(&sim->dongle_lock[right]);
		pthread_mutex_unlock(&sim->dongle_lock[left]);
	}
	else
	{
		pthread_mutex_lock(&sim->dongle_lock[right]);
		pthread_mutex_lock(&sim->dongle_lock[left]);
		if (sim->dongle_taken[left] || ms < sim->dongle_free_at[left])
			return (pthread_mutex_unlock(&sim->dongle_lock[left]),
					pthread_mutex_unlock(&sim->dongle_lock[right]), 0);
		if (sim->dongle_taken[right] || ms < sim->dongle_free_at[right])
			return (pthread_mutex_unlock(&sim->dongle_lock[left]),
					pthread_mutex_unlock(&sim->dongle_lock[right]), 0);
		pthread_mutex_unlock(&sim->dongle_lock[left]);
		pthread_mutex_unlock(&sim->dongle_lock[right]);
	}
	if (heap_top(sim) != idx)
		return (0);
	return (1);
}

static int	lock_dongles(t_coder *coder, t_sim *sim, int idx)
{
	if (can_take(sim, idx))
	{
		heap_rm_top(sim);
		coder->state = "COMPILING";
		coder->last_compile = now_time(sim, "ms");
		coder->deadline = coder->last_compile + sim->burnout;
		if (coder-> left < coder->right)
		{
			pthread_mutex_lock(&sim->dongle_lock[coder->left]);
			pthread_mutex_lock(&sim->dongle_lock[coder->right]);
			sim->dongle_taken[coder->left] = 1;
			sim->dongle_taken[coder->right] = 1;
			pthread_mutex_unlock(&sim->dongle_lock[coder->right]);
			pthread_mutex_unlock(&sim->dongle_lock[coder->left]);
		}
		else
		{
			pthread_mutex_lock(&sim->dongle_lock[coder->right]);
			pthread_mutex_lock(&sim->dongle_lock[coder->left]);
			sim->dongle_taken[coder->left] = 1;
			sim->dongle_taken[coder->right] = 1;
			pthread_mutex_unlock(&sim->dongle_lock[coder->left]);
			pthread_mutex_unlock(&sim->dongle_lock[coder->right]);
		}
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
	coder->waiting_since = now_time(sim, "micro");
	heap_add(sim, coder->id - 1);
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
