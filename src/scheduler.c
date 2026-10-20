/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hassamoh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 15:18:59 by hassamoh          #+#    #+#             */
/*   Updated: 2026/10/05 12:02:10 by hassamoh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Converts an absolute timestamp in ms into a struct timespec, as
** required by pthread_cond_timedwait (CLOCK_REALTIME, the same clock
** gettimeofday uses).
*/
static void	ms_to_timespec(long ms, struct timespec *ts)
{
	ts->tv_sec = ms / 1000;
	ts->tv_nsec = (ms % 1000) * 1000000L;
}

/*
** Thread-safe read of env->simulation_stop.
*/
static int	is_stopped(t_env *env)
{
	int	stop;

	pthread_mutex_lock(&env->stop_lock);
	stop = env->simulation_stop;
	pthread_mutex_unlock(&env->stop_lock);
	return (stop);
}

static int	wait_for_turn(t_dongle *dongle, t_env *env, long seq)
{
	struct timespec	deadline;
	int				my_turn;

	while (1)
	{
		my_turn = (dongle->in_use == 0 && dongle->queue.size > 0
				&& dongle->queue.data[0].seq == seq);
		if (is_stopped(env))
			return (0);
		if (my_turn && get_timestamp() >= dongle->available_at)
			return (1);
		if (my_turn)
		{
			ms_to_timespec(dongle->available_at, &deadline);
			pthread_cond_timedwait(&dongle->cond, &dongle->lock, &deadline);
		}
		else
			pthread_cond_wait(&dongle->cond, &dongle->lock);
	}
}

int	dongle_wait(t_dongle *dongle, t_env *env, int policy, long deadline)
{
	t_waiter	waiter;

	pthread_mutex_lock(&dongle->lock);
	waiter.seq = dongle->next_seq;
	dongle->next_seq++;
	waiter.key = waiter.seq;
	if (policy == SCHED_EDF_TYPE)
		waiter.key = deadline;
	heap_push(&dongle->queue, waiter);
	if (wait_for_turn(dongle, env, waiter.seq) == 0)
	{
		heap_remove(&dongle->queue, waiter.seq);
		pthread_mutex_unlock(&dongle->lock);
		return (0);
	}
	heap_remove(&dongle->queue, waiter.seq);
	dongle->in_use = 1;
	pthread_mutex_unlock(&dongle->lock);
	return (1);
}

/*
** Frees the dongle, arms its cooldown (it cannot be taken again
** before available_at) and wakes every waiter so that each one
** re-checks whether it is now the next to be served.
*/
void	dongle_release(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->lock);
	dongle->in_use = 0;
	dongle->available_at = get_timestamp() + dongle->cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->lock);
}
