/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hassamoh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 15:25:27 by hassamoh          #+#    #+#             */
/*   Updated: 2026/10/05 15:14:27 by hassamoh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <sys/time.h>
# include <errno.h>
# include <limits.h>

# define SCHED_FIFO_TYPE	0
# define SCHED_EDF_TYPE		1

typedef struct s_coder	t_coder;
typedef struct s_env	t_env;

# define DONGLE_QUEUE_CAP	2

typedef struct s_waiter
{
	long			key;
	long			seq;
}	t_waiter;

typedef struct s_heap
{
	t_waiter		data[DONGLE_QUEUE_CAP];
	int				size;
}	t_heap;

/*
** One USB dongle shared between two neighbouring coders.
** - lock/cond protect access and let coders wait for the dongle.
** - in_use tells whether the dongle is currently held.
** - available_at is the timestamp (ms) after which the dongle
**   may be taken again (cooldown); cooldown is its duration.
** - queue/next_seq hold the waiting requests (see t_heap).
*/
typedef struct s_dongle
{
	int				id;
	int				in_use;
	long			available_at;
	long			cooldown;
	long			next_seq;
	t_heap			queue;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
}	t_dongle;

/*
** One coder (one thread). Each coder knows its two neighbouring
** dongles and keeps a back-pointer to the shared environment.
*/
struct s_coder
{
	int				id;
	long			last_compile_start;
	int				compiles_done;
	t_dongle		*left;
	t_dongle		*right;
	pthread_t		thread;
	t_env			*env;
};

/*
** Shared environment: everything that would otherwise need to be
** a global variable lives here instead, and is passed around via
** pointer. Holds the simulation parameters, the shared arrays of
** dongles/coders, and the mutexes protecting cross-cutting state
** (the stop flag, the logger, and each coder's timing/progress
** fields, which the monitor thread reads while coders write them).
*/
struct s_env
{
	int				n_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	int				compiles_required;
	long			dongle_cooldown;
	int				scheduler;
	long			start_time;
	int				simulation_stop;
	pthread_mutex_t	stop_lock;
	pthread_mutex_t	log_lock;
	pthread_mutex_t	state_lock;
	t_dongle		*dongles;
	t_coder			*coders;
};

/* main.c */
int		main(int argc, char **argv);

/* parsing.c */
int		check_args(int argc, char **argv, t_env *env);

/* utils.c */
long	get_timestamp(void);

/* logger.c */
void	log_state(t_env *env, int coder_id, const char *state);

/* init.c */
int		init_env(t_env *env);
void	cleanup_env(t_env *env);

/* dongle.c */
int		take_dongles(t_coder *coder);
void	release_dongles(t_coder *coder);

/* heap.c */
int		heap_push(t_heap *heap, t_waiter waiter);
void	heap_remove(t_heap *heap, long seq);

/* scheduler.c */
int		dongle_wait(t_dongle *dongle, t_env *env, int policy, long deadline);
void	dongle_release(t_dongle *dongle);

/* coder.c */
void	*coder_routine(void *arg);

/* monitor.c */
void	*monitor_routine(void *arg);
#endif
