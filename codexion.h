/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhadir <mhadir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 12:15:35 by mhadir            #+#    #+#             */
/*   Updated: 2026/05/03 21:28:19 by mhadir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <limits.h>
# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_sim	t_sim;

typedef struct s_coder
{
	int					id;
	int					left;
	int					right;
	int					compiles;
	long				last_compile;
	long				deadline;
	long				waiting_since;
	t_sim				*sim;
	pthread_t			thread;
}						t_coder;

typedef struct s_heap
{
	int					*array;
	int					current_size;
	int					size;
}						t_heap;

struct					s_sim
{
	int					n;
	long				burnout;
	long				t_compile;
	long				t_debug;
	long				t_refactor;
	int					need;
	long				cooldown;
	char				*scheduler;
	t_coder				*coders;
	t_heap				heap;
	int					*dongle_taken;
	long				*dongle_free_at;
	pthread_mutex_t		lock;
	pthread_cond_t		cond;
	pthread_mutex_t		print_lock;
	pthread_mutex_t		*dongle_lock;
	pthread_t			monitor;
	int					stop;
	long				start_ms;
	long				start_micro;
};

long					now_time(t_sim *sim, char *unit);
void					ft_sleep(long ms);
void					print_log(t_sim *sim, int id, char *msg);
void					print_burnout(t_sim *sim, int id);
int						check_stop(t_sim *sim);
void					*coder_thread(void *arg);
void					*monitor_thread(void *arg);
int						parse(t_sim *sim, int ac, char **av);
int						heap_top(t_sim *sim);
void					heap_rm_top(t_sim *sim);
void					heap_add(t_sim *sim, int idx);
int						init(t_sim *sim);
void					cleanup(t_sim *sim, int free_all, int initialized);
void					find_first(t_coder *coder, int *first, int *second);
int						is_higher(t_sim *sim, int a, int b);

#endif
