/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhadir <mhadir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:15:31 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/18 20:27:16 by mhadir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	cleanup(t_sim *sim, int free_all)
{
	int	i;

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
	i = 0;
	while (i < sim->n)
	{
		pthread_mutex_destroy(&sim->dongle_lock[i]);
		i++;
	}
	free(sim->dongle_lock);
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
		return (fprintf(stderr, "Error: monitor_create failed\n"), cleanup(&sim,
				1), 1);
	i = 0;
	while (i < sim.n)
	{
		if (pthread_join(sim.coders[i].thread, NULL) != 0)
			return (fprintf(stderr, "Error: pthread_join failed\n"), 1);
		i++;
	}
	if (pthread_join(sim.monitor, NULL) != 0)
		return (fprintf(stderr, "Error: pthread_join failed\n"), 1);
	return (cleanup(&sim, 1), 0);
}
