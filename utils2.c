/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils2.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhadir <mhadir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 08:13:06 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/20 08:14:06 by mhadir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	find_first(t_coder *coder, int *first, int *second)
{
	int	left;
	int	right;

	left = coder->left;
	right = coder->right;
	if (left < right)
	{
		*first = left;
		*second = right;
	}
	else
	{
		*first = right;
		*second = left;
	}
}

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
	{
		if (coders[array[a]].deadline == coders[array[b]].deadline)
		{
			if (coders[array[a]].waiting_since
				== coders[array[b]].waiting_since)
				return (coders[array[a]].id < coders[array[b]].id);
			return (coders[array[a]].waiting_since
				< coders[array[b]].waiting_since);
		}
		return (coders[array[a]].deadline
			< coders[array[b]].deadline);
	}
	return (0);
}
