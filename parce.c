/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parce.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhadir <mhadir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:20:05 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/04 12:20:57 by mhadir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_valid(const char *s)
{
	int	i;

	i = 0;
	if (!s || !s[0])
		return (0);
	if (s[i] == '+')
		i++;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

int	parse(t_sim *sim, int ac, char **av)
{
	if (ac != 9)
		return (fprintf(stderr, "./codexion n burnout compile"
				" debug refactor need cooldown [fifo|edf]\n"), 0);
	if (!is_valid(av[1]) || !is_valid(av[2]) || !is_valid(av[3])
		|| !is_valid(av[4]) || !is_valid(av[5])
		|| !is_valid(av[6]) || !is_valid(av[7]))
		return (fprintf(stderr, "Error: invalid argument\n"), 0);
	sim->n = atoi(av[1]);
	sim->burnout = atoi(av[2]);
	sim->t_compile = atoi(av[3]);
	sim->t_debug = atoi(av[4]);
	sim->t_refactor = atoi(av[5]);
	sim->need = atoi(av[6]);
	sim->cooldown = atoi(av[7]);
	if (strcmp(av[8], "fifo") == 0)
		sim->edf = 0;
	else if (strcmp(av[8], "edf") == 0)
		sim->edf = 1;
	else
		return (fprintf(stderr, "Error: scheduler must be fifo or edf\n"), 0);
	if (sim->n < 1)
		return (fprintf(stderr, "Error: need at least 1 coder\n"), 0);
	return (1);
}
