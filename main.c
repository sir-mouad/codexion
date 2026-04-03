#include "codexion.h"

static int	is_valid(const char *s)
{
	int	i;

	i = 0;
	if (!s || !s[0])
		return (0);
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	parse(t_sim *sim, int ac, char **av)
{
	if (ac != 9)
		return (fprintf(stderr, "Usage: codexion n burnout compile"
				" debug refactor need cooldown fifo|edf\n"), 0);
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

static int	init(t_sim *sim)
{
	int				i;
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	sim->start = tv.tv_sec * 1000L + tv.tv_usec / 1000L;
	sim->stop = 0;
	sim->coders = malloc(sizeof(t_coder) * sim->n);
	sim->dongle_taken = malloc(sizeof(int) * sim->n);
	sim->dongle_free_at = malloc(sizeof(long) * sim->n);
	if (!sim->coders || !sim->dongle_taken || !sim->dongle_free_at)
		return (0);
	pthread_mutex_init(&sim->lock, NULL);
	pthread_mutex_init(&sim->print_lock, NULL);
	pthread_cond_init(&sim->cond, NULL);
	i = 0;
	while (i < sim->n)
	{
		sim->dongle_taken[i] = 0;
		sim->dongle_free_at[i] = 0;
		sim->coders[i].id = i + 1;
		sim->coders[i].left = i;
		sim->coders[i].right = (i + 1) % sim->n;
		sim->coders[i].state = WAITING;
		sim->coders[i].compiles = 0;
		sim->coders[i].last_compile = 0;
		sim->coders[i].waiting_since = 0;
		sim->coders[i].sim = sim;
		i++;
	}
	return (1);
}

static void	cleanup(t_sim *sim)
{
	pthread_mutex_destroy(&sim->lock);
	pthread_mutex_destroy(&sim->print_lock);
	pthread_cond_destroy(&sim->cond);
	free(sim->coders);
	free(sim->dongle_taken);
	free(sim->dongle_free_at);
}

int	main(int ac, char **av)
{
	t_sim	sim;
	int		i;

	memset(&sim, 0, sizeof(t_sim));
	if (!parse(&sim, ac, av))
		return (1);
	if (!init(&sim))
		return (fprintf(stderr, "Error: malloc failed\n"), 1);
	i = 0;
	while (i < sim.n)
	{
		pthread_create(&sim.coders[i].thread, NULL,
			coder_thread, &sim.coders[i]);
		i++;
	}
	pthread_create(&sim.monitor, NULL, monitor_thread, &sim);
	i = 0;
	while (i < sim.n)
	{
		pthread_join(sim.coders[i].thread, NULL);
		i++;
	}
	pthread_join(sim.monitor, NULL);
	cleanup(&sim);
	return (0);
}
