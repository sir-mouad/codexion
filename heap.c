/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mouad <mouad@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 20:49:29 by mouad             #+#    #+#             */
/*   Updated: 2026/04/18 16:03:53 by mouad            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	heap_up(t_sim *sim)
{
	t_heap	*heap;
	int		i;
	int		parent;
	int		swp;

	heap = &sim->heap;
	i = heap->current_size - 1;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!is_higher(sim, i, parent))
			break ;
		swp = heap->array[i];
		heap->array[i] = heap->array[parent];
		heap->array[parent] = swp;
		i = parent;
	}
}

static void	heap_down(t_sim *sim)
{
	t_heap	*heap;

	int (i), (left), (right), (best), (swp);
	heap = &sim->heap;
	i = 0;
	while (1)
	{
		left = (i * 2) + 1;
		right = (i * 2) + 2;
		best = i;
		if (left < heap->current_size && is_higher(sim, left, best))
			best = left;
		if (right < heap->current_size && is_higher(sim, right, best))
			best = right;
		if (best == i)
			break ;
		swp = heap->array[i];
		heap->array[i] = heap->array[best];
		heap->array[best] = swp;
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
		heap_up(sim);
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
		heap_down(sim);
	}
}

int	heap_top(t_sim *sim)
{
	t_heap	heap;

	heap = sim->heap;
	if (heap.current_size == 0)
		return (-1);
	return (heap.array[0]);
}
