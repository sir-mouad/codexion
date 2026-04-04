/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhadir <mhadir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:15:35 by mhadir            #+#    #+#             */
/*   Updated: 2026/04/04 12:23:55 by mhadir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/time.h>

# define WAITING    0
# define COMPILING  1
# define DEBUGGING  2
# define REFACTORING 3

typedef struct s_sim	t_sim;

typedef struct s_coder
{
	int			id;
	int			left;
	int			right;
	int			state;
	int			compiles;
	long		last_compile;
	long		waiting_since;
	t_sim		*sim;
	pthread_t	thread;
}	t_coder;

struct s_sim
{
	int				n;
	long			burnout;
	long			t_compile;
	long			t_debug;
	long			t_refactor;
	int				need;
	long			cooldown;
	int				edf;
	t_coder			*coders;
	int				*dongle_taken;
	long			*dongle_free_at;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
	pthread_mutex_t	print_lock;
	pthread_t		monitor;
	int				stop;
	long			start;
};

long	now_ms(t_sim *sim);
void	ft_sleep(long ms);
void	print_log(t_sim *sim, int id, char *msg);
void	print_burnout(t_sim *sim, int id);
int		is_done(t_sim *sim);
void	*coder_thread(void *arg);
void	*monitor_thread(void *arg);
int		is_valid(const char *s);
int		parse(t_sim *sim, int ac, char **av);

#endif
