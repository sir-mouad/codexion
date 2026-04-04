/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhadir <mhadir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:15:16 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/04 12:25:04 by mhadir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Priority comparison.
** Only coders in WAITING state are competing for dongles.
** If neighbor is not WAITING (is compiling/debugging/refactoring),
** it is not competing so current coder wins by default.
** FIFO : whoever started waiting first wins.
** EDF  : whoever has the closest burnout deadline wins.
*/
static int	has_priority(t_sim *sim, int a, int b)
{
	t_coder	*ca;
	t_coder	*cb;

	ca = &sim->coders[a];
	cb = &sim->coders[b];
	if (cb->state != WAITING)
		return (1);
	if (sim->edf)
		return (ca->last_compile <= cb->last_compile);
	return (ca->waiting_since <= cb->waiting_since);
}

/*
** Can this coder grab both dongles right now?
** Checks: both dongles free + cooldown passed + beats both neighbors.
*/
static int	can_grab(t_sim *sim, int idx)
{
	int		left;
	int		right;
	int		ln;
	int		rn;
	long	ms;

	left = sim->coders[idx].left;
	right = sim->coders[idx].right;
	ln = (idx - 1 + sim->n) % sim->n;
	rn = (idx + 1) % sim->n;
	ms = now_ms(sim);
	if (sim->dongle_taken[left] || ms < sim->dongle_free_at[left])
		return (0);
	if (sim->dongle_taken[right] || ms < sim->dongle_free_at[right])
		return (0);
	if (!has_priority(sim, idx, ln))
		return (0);
	if (!has_priority(sim, idx, rn))
		return (0);
	return (1);
}

/*
** Grab both dongles.
** Coder sets state to WAITING, then sleeps until can_grab() is true.
** Both dongles are marked taken atomically inside the lock.
*/
static int	grab_dongles(t_coder *coder)
{
	t_sim			*sim;
	int				idx;
	struct timespec	ts;
	struct timeval	tv;

	sim = coder->sim;
	idx = coder->id - 1;
	pthread_mutex_lock(&sim->lock);
	coder->state = WAITING;
	coder->waiting_since = now_ms(sim);
	while (!sim->stop)
	{
		if (can_grab(sim, idx))
		{
			coder->state = COMPILING;
			sim->dongle_taken[coder->left] = 1;
			sim->dongle_taken[coder->right] = 1;
			pthread_mutex_unlock(&sim->lock);
			return (1);
		}
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
	coder->state = DEBUGGING;
	pthread_mutex_unlock(&sim->lock);
	return (0);
}

/*
** Release both dongles, record cooldown, wake up waiting neighbors.
*/
static void	release_dongles(t_coder *coder)
{
	t_sim	*sim;
	long	free_at;

	sim = coder->sim;
	free_at = now_ms(sim) + sim->cooldown;
	pthread_mutex_lock(&sim->lock);
	sim->dongle_taken[coder->left] = 0;
	sim->dongle_taken[coder->right] = 0;
	sim->dongle_free_at[coder->left] = free_at;
	sim->dongle_free_at[coder->right] = free_at;
	pthread_cond_broadcast(&sim->cond);
	pthread_mutex_unlock(&sim->lock);
}

/*
** The coder life loop:
**
**   WAITING → grab dongles
**   COMPILING → log + sleep t_compile → release
**   DEBUGGING → log + sleep t_debug
**   REFACTORING → log + sleep t_refactor
**   → back to WAITING
*/
void	*coder_thread(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;
	if (sim->n == 1)
	{
		while (!is_done(sim))
			ft_sleep(10);
		return (NULL);
	}
	while (!is_done(sim))
	{
		if (!grab_dongles(coder))
			break ;
		coder->last_compile = now_ms(sim);
		print_log(sim, coder->id, "has taken a dongle");
		print_log(sim, coder->id, "has taken a dongle");
		print_log(sim, coder->id, "is compiling");
		ft_sleep(sim->t_compile);
		coder->compiles++;
		release_dongles(coder);
		if (is_done(sim))
			break ;
		pthread_mutex_lock(&sim->lock);
		coder->state = DEBUGGING;
		pthread_mutex_unlock(&sim->lock);
		print_log(sim, coder->id, "is debugging");
		ft_sleep(sim->t_debug);
		if (is_done(sim))
			break ;
		pthread_mutex_lock(&sim->lock);
		coder->state = REFACTORING;
		pthread_mutex_unlock(&sim->lock);
		print_log(sim, coder->id, "is refactoring");
		ft_sleep(sim->t_refactor);
	}
	return (NULL);
}
