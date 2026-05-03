/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhadir <mhadir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 14:46:55 by mouad             #+#    #+#             */
/*   Updated: 2026/05/03 21:28:14 by mhadir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	sim_init(t_sim *sim)
{
	int	i;

	i = 0;
	sim->stop = 0;
	while (i < sim->n)
	{
		sim->dongle_taken[i] = 0;
		sim->dongle_free_at[i] = 0;
		sim->coders[i].id = i + 1;
		sim->coders[i].left = i;
		sim->coders[i].right = (i + 1) % sim->n;
		sim->coders[i].compiles = 0;
		sim->coders[i].last_compile = 0;
		sim->coders[i].deadline = sim->burnout;
		sim->coders[i].waiting_since = 0;
		sim->coders[i].sim = sim;
		i++;
	}
	sim->heap.current_size = 0;
	sim->heap.size = sim->n;
}

static int	mutex_cond_init(t_sim *sim)
{
	int	i;

	i = 0;
	if (pthread_mutex_init(&sim->lock, NULL) != 0
		|| pthread_mutex_init(&sim->print_lock, NULL) != 0
		|| pthread_cond_init(&sim->cond, NULL) != 0)
		return (cleanup(sim, 0, 0), free(sim->heap.array), fprintf(stderr,
				"Error: mutex or cond failed\n"), 0);
	while (i < sim->n)
	{
		if (pthread_mutex_init(&sim->dongle_lock[i], NULL) != 0)
			return (fprintf(stderr, "Error: mutex failed\n"),
				cleanup(sim, 1, i), 0);
		i++;
	}
	return (1);
}

static void	free_mallocs(t_sim *sim)
{
	free(sim->coders);
	free(sim->dongle_taken);
	free(sim->dongle_free_at);
	free(sim->dongle_lock);
	free(sim->heap.array);
}

int	init(t_sim *sim)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	sim->start_ms = tv.tv_sec * 1000L + tv.tv_usec / 1000L;
	sim->start_micro = tv.tv_sec * 1000000L + tv.tv_usec;
	sim->coders = malloc(sizeof(t_coder) * sim->n);
	sim->dongle_taken = malloc(sizeof(int) * sim->n);
	sim->dongle_free_at = malloc(sizeof(long) * sim->n);
	sim->dongle_lock = malloc(sizeof(pthread_mutex_t) * sim->n);
	if (!sim->coders || !sim->dongle_taken || !sim->dongle_free_at
		|| !sim->dongle_lock)
		return (fprintf(stderr, "Error: malloc failed\n"),
			free_mallocs(sim), 0);
	sim_init(sim);
	sim->heap.array = malloc(sizeof(int) * sim->n);
	if (!sim->heap.array)
		return (fprintf(stderr, "Error: malloc failed\n"),
			cleanup(sim, 0, sim->n), 0);
	if (!mutex_cond_init(sim))
		return (0);
	return (1);
}
