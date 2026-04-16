/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mouad <mouad@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 20:49:29 by mouad             #+#    #+#             */
/*   Updated: 2026/04/16 20:53:47 by mouad            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	heapify_up(t_sim *sim)
{
	t_heap	*heap;
	t_coder	*coders;
	int		*current_size;
	int		*array;

	int (i), (parent), (swp);
	heap = &sim->heap;
	coders = sim->coders;
	array = heap->array;
	current_size = &heap->current_size;
	i = *current_size - 1;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (strcmp(sim->scheduler, "fifo") == 0)
		{
			if (coders[array[i]].waiting_since
				< coders[array[parent]].waiting_since)
			{
				swp = array[i];
				array[i] = array[parent];
				array[parent] = swp;
				i = parent;
			}
			else
				break ;
		}
		if (strcmp(sim->scheduler, "edf") == 0)
		{
			if (coders[array[i]].deadline
				< coders[array[parent]].deadline)
			{
				swp = array[i];
				array[i] = array[parent];
				array[parent] = swp;
				i = parent;
			}
			else
				break ;
		}
	}
}

void	heapify_down(t_sim *sim)
{
	t_heap	*heap;
	t_coder	*coders;
	int		*array;

	int (i), (left), (right), (best), (swp);
	heap = &sim->heap;
	coders = sim->coders;
	array = heap->array;
	i = 0;
	while (1)
	{
		left = (i * 2) + 1;
		right = (i * 2) + 2;
		best = i;
		if (strcmp(sim->scheduler, "fifo") == 0)
		{
			if (left < heap->current_size
				&& coders[array[left]].waiting_since
				< coders[array[best]].waiting_since)
				best = left;
			if (right < heap->current_size
				&& coders[array[right]].waiting_since
				< coders[array[best]].waiting_since)
				best = right;
		}
		if (strcmp(sim->scheduler, "edf") == 0)
		{
			if (left < heap->current_size
				&& coders[array[left]].deadline < coders[array[best]].deadline)
				best = left;
			if (right < heap->current_size
				&& coders[array[right]].deadline < coders[array[best]].deadline)
				best = right;
		}
		if (best == i)
			break ;
		swp = array[i];
		array[i] = array[best];
		array[best] = swp;
		i = best;
	}
}

void	heap_add(t_sim *sim, int idx)
{
	t_heap	*heap;
	int		*current_size;
	int		size;

	heap = &sim->heap;
	size = heap->size;
	current_size = &heap->current_size;
	if (*current_size < size)
	{
		heap->array[*current_size] = idx;
		(*current_size)++;
		heapify_up(sim);
	}
}

void	heap_rm_top(t_sim *sim)
{
	t_heap	*heap;
	int		*current_size;

	heap = &sim->heap;
	current_size = &heap->current_size;
	if (*current_size == 0)
		return ;
	if (*current_size == 1)
	{
		(*current_size)--;
		return ;
	}
	if (*current_size > 1)
	{
		heap->array[0] = heap->array[*current_size - 1];
		(*current_size)--;
		heapify_down(sim);
	}
}
