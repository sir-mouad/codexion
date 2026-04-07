/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mouad <mouad@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 11:20:41 by mouad             #+#    #+#             */
/*   Updated: 2026/04/07 11:30:16 by mouad            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	release_dongles(t_coder *coder)
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

int	has_priority(t_sim *sim, int a, int b)
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
