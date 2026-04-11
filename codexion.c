/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mouad <mouad@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:15:31 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/07 11:01:56 by mouad            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	cleanup(t_sim *sim, int free_all)
{
	if (!free_all)
	{
		free(sim->coders);
		free(sim->dongle_taken);
		free(sim->dongle_free_at);
	}
	else
	{
		pthread_mutex_destroy(&sim->lock);
		pthread_mutex_destroy(&sim->print_lock);
		pthread_cond_destroy(&sim->cond);
		free(sim->coders);
		free(sim->dongle_taken);
		free(sim->dongle_free_at);
	}
}

static void	sim_init(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->n)
	{
		sim->dongle_taken[i] = 0;
		sim->dongle_free_at[i] = 0;
		sim->coders[i].id = i + 1;
		sim->coders[i].left = i;
		sim->coders[i].right = (i + 1) % sim->n;
		sim->coders[i].state = WAITING;
		sim->coders[i].compiles = 0;
		sim->coders[i].last_compile = 0;
		sim->coders[i].waiting_since = 0;
		sim->coders[i].sim = sim;
		i++;
	}
}

static int	init(t_sim *sim)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	sim->start = tv.tv_sec * 1000L + tv.tv_usec / 1000L;
	sim->stop = 0;
	sim->coders = malloc(sizeof(t_coder) * sim->n);
	sim->dongle_taken = malloc(sizeof(int) * sim->n);
	sim->dongle_free_at = malloc(sizeof(long) * sim->n);
	if (!sim->coders || !sim->dongle_taken || !sim->dongle_free_at)
		return (fprintf(stderr, "Error: malloc failed\n"), 0);
	sim_init(sim);
	if (pthread_mutex_init(&sim->lock, NULL) != 0
		|| pthread_mutex_init(&sim->print_lock, NULL) != 0
		|| pthread_cond_init(&sim->cond, NULL) != 0)
		return (cleanup(sim, 0), fprintf(stderr,
				"Error: mutex or cond failed\n"), 0);
	return (1);
}

static int	create_coder_thread(t_sim *sim)
{
	int (i), (j);
	i = 0;
	j = 0;
	while (i < sim->n)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_thread,
				&sim->coders[i]) != 0)
		{
			pthread_mutex_lock(&sim->lock);
			sim->stop = 1;
			pthread_cond_broadcast(&sim->cond);
			pthread_mutex_unlock(&sim->lock);
			j = 0;
			while (j < i)
			{
				pthread_join(sim->coders[j].thread, NULL);
				j++;
			}
			return (fprintf(stderr, "Error: pthread_create failed\n"),
				cleanup(sim, 1), 1);
		}
		i++;
	}
	return (0);
}

int	main(int ac, char **av)
{
	t_sim	sim;
	int		i;

	memset(&sim, 0, sizeof(t_sim));
	if (!parse(&sim, ac, av))
		return (1);
	if (!init(&sim))
		return (1);
	if (create_coder_thread(&sim))
		return (1);
	if (pthread_create(&sim.monitor, NULL, monitor_thread, &sim) != 0)
		return (fprintf(stderr, "Error: monitor_create failed\n"), 1);
	i = 0;
	while (i < sim.n)
	{
		if (pthread_join(sim.coders[i].thread, NULL))
			return (fprintf(stderr, "Error: pthread_join failed\n"), 1);
		i++;
	}
	if (pthread_join(sim.monitor, NULL) != 0)
		return (fprintf(stderr, "Error: pthread_join failed\n"), 1);
	return (cleanup(&sim, 1), 0);
}
