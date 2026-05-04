/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhadir <mhadir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:15:31 by mhadir            #+#    #+#             */
/*   Updated: 2026/05/04 10:16:47 by mhadir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	cleanup(t_sim *sim, int free_all, int initialized)
{
	if (!free_all)
	{
		free(sim->coders);
		free(sim->dongle_taken);
		free(sim->dongle_free_at);
		return ;
	}
	pthread_mutex_destroy(&sim->lock);
	pthread_mutex_destroy(&sim->print_lock);
	pthread_cond_destroy(&sim->cond);
	free(sim->coders);
	free(sim->dongle_taken);
	free(sim->dongle_free_at);
	free(sim->heap.array);
	while (initialized > 0)
	{
		initialized--;
		pthread_mutex_destroy(&sim->dongle_lock[initialized]);
	}
	free(sim->dongle_lock);
}

static void	signal_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->lock);
	sim->stop = 1;
	pthread_cond_broadcast(&sim->cond);
	pthread_mutex_unlock(&sim->lock);
}

static void	stop_simulation(t_sim *sim, int start, int end)
{
	signal_stop(sim);
	while (start < end)
	{
		pthread_join(sim->coders[start].thread, NULL);
		start++;
	}
}

static int	create_coder_thread(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->n)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_thread,
				&sim->coders[i]) != 0)
		{
			stop_simulation(sim, 0, i);
			return (fprintf(stderr, "Error: pthread_create failed\n"),
				cleanup(sim, 1, sim->n), 1);
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
	if (sim.need == 0)
		return (0);
	if (!init(&sim) || create_coder_thread(&sim))
		return (1);
	if (pthread_create(&sim.monitor, NULL, monitor_thread, &sim) != 0)
		return (stop_simulation(&sim, 0, sim.n), fprintf(stderr,
				"Error: monitor_create failed\n"), cleanup(&sim, 1, sim.n), 1);
	i = 0;
	while (i < sim.n)
	{
		pthread_join(sim.coders[i].thread, NULL);
		i++;
	}
	pthread_join(sim.monitor, NULL);
	return (cleanup(&sim, 1, sim.n), 0);
}
