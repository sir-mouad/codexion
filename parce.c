/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parce.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mouad <mouad@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:20:05 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/17 14:20:05 by mouad            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_valid(const char *s)
{
	int	i;

	i = 0;
	if (!s || !s[0])
		return (0);
	if (s[i] == '+' && s[i + 1] >= '0' && s[i + 1] <= '9')
		i++;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	check_overflow(t_sim *sim)
{
	if ((sim->n == -1) || (sim->burnout == -1) || (sim->t_compile == -1)
		|| (sim->t_debug == -1) || (sim->t_refactor == -1) || (sim->need == -1)
		|| (sim->cooldown == -1))
		return (1);
	return (0);
}

static long	my_atoi(const char *str, char *type)
{
	int				i;
	unsigned long	res;

	i = 0;
	res = 0;
	while (str[i] == 32 || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '+')
		i++;
	while (str[i] >= '0' && str[i] <= '9')
	{
		res = (res * 10) + (str[i] - '0');
		if (strcmp(type, "int") == 0)
			if (res > INT_MAX)
				return (-1);
		if (strcmp(type, "long") == 0)
			if (res > LONG_MAX)
				return (-1);
		i++;
	}
	return (res);
}

int	parse(t_sim *sim, int ac, char **av)
{
	if (ac != 9)
		return (fprintf(stderr, "./codexion n burnout compile"
				" debug refactor need cooldown [fifo|edf]\n"), 0);
	if (!is_valid(av[1]) || !is_valid(av[2]) || !is_valid(av[3])
		|| !is_valid(av[4]) || !is_valid(av[5]) || !is_valid(av[6])
		|| !is_valid(av[7]))
		return (fprintf(stderr, "Error: invalid argument\n"), 0);
	sim->n = my_atoi(av[1], "int");
	sim->burnout = my_atoi(av[2], "long");
	sim->t_compile = my_atoi(av[3], "long");
	sim->t_debug = my_atoi(av[4], "long");
	sim->t_refactor = my_atoi(av[5], "long");
	sim->need = my_atoi(av[6], "int");
	sim->cooldown = my_atoi(av[7], "long");
	if (check_overflow(sim))
		return (fprintf(stderr, "Error: overflow problem\n"), 0);
	if (strcmp(av[8], "fifo") == 0)
		sim->scheduler = "fifo";
	else if (strcmp(av[8], "edf") == 0)
		sim->scheduler = "edf";
	else
		return (fprintf(stderr, "Error: scheduler must be fifo or edf\n"), 0);
	if (sim->n < 1)
		return (fprintf(stderr, "Error: need at least 1 coder\n"), 0);
	return (1);
}
