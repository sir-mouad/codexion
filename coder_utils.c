/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mouad <mouad@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 10:39:07 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/16 20:44:04 by mouad            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	unlock_dongles(t_coder *coder)
{
	t_sim	*sim;
	long	free_at;

	sim = coder->sim;
	free_at = now_time(sim, "ms") + sim->cooldown;
	pthread_mutex_lock(&sim->lock);
	sim->dongle_taken[coder->left] = 0;
	sim->dongle_taken[coder->right] = 0;
	sim->dongle_free_at[coder->left] = free_at;
	sim->dongle_free_at[coder->right] = free_at;
	pthread_cond_broadcast(&sim->cond);
	pthread_mutex_unlock(&sim->lock);
}

int	can_go_first(t_sim *sim, int a, int b)
{
	t_coder	*ca;
	t_coder	*cb;

	ca = &sim->coders[a];
	cb = &sim->coders[b];
	if (strcmp(cb->state, "WAITING") != 0)
		return (1);
	if (strcmp(sim->scheduler, "edf") == 0)
		return (ca->last_compile <= cb->last_compile);
	return (ca->waiting_since <= cb->waiting_since);
}
