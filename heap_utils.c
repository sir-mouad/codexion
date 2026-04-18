/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhadir <mhadir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 16:01:46 by mouad             #+#    #+#             */
/*   Updated: 2026/04/18 20:27:08 by mhadir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_higher(t_sim *sim, int a, int b)
{
	t_coder	*coders;
	int		*array;

	coders = sim->coders;
	array = sim->heap.array;
	if (strcmp(sim->scheduler, "fifo") == 0)
		return (coders[array[a]].waiting_since
			< coders[array[b]].waiting_since);
	if (strcmp(sim->scheduler, "edf") == 0)
		return (coders[array[a]].deadline
			< coders[array[b]].deadline);
	return (0);
}
