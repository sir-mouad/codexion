/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parce_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mouad <mouad@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 20:55:38 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/11 23:06:24 by mouad            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	ft_atoi(const char *str, char *type)
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
