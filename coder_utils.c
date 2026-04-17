/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhadir <mhadir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 10:39:07 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/17 18:23:06 by mhadir           ###   ########.fr       */
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
	pthread_mutex_lock(&sim->dongle_lock[coder->left]);
	pthread_mutex_lock(&sim->dongle_lock[coder->right]);
	sim->dongle_taken[coder->left] = 0;
	sim->dongle_taken[coder->right] = 0;
	sim->dongle_free_at[coder->left] = free_at;
	sim->dongle_free_at[coder->right] = free_at;
	pthread_mutex_unlock(&sim->dongle_lock[coder->left]);
	pthread_mutex_unlock(&sim->dongle_lock[coder->right]);
	pthread_cond_broadcast(&sim->cond);
	pthread_mutex_unlock(&sim->lock);
}