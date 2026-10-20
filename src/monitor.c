/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hassamoh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 15:14:56 by hassamoh          #+#    #+#             */
/*   Updated: 2026/10/05 13:48:41 by hassamoh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Thread-safe snapshot of one coder's last_compile_start.
*/
static long	read_last_compile_start(t_env *env, int index)
{
	long	value;

	pthread_mutex_lock(&env->state_lock);
	value = env->coders[index].last_compile_start;
	pthread_mutex_unlock(&env->state_lock);
	return (value);
}

/*
** Checks every coder for burnout: if "now" is more than
** time_to_burnout ms past their last_compile_start, they have
** burned out. Logs the mandatory message and returns 1 on the
** first burnout found, 0 if everyone is still within their
** deadline.
*/
static int	check_burnout(t_env *env)
{
	int		i;
	long	last;

	i = 0;
	while (i < env->n_coders)
	{
		last = read_last_compile_start(env, i);
		if (get_timestamp() - last > env->time_to_burnout)
		{
			log_state(env, env->coders[i].id, "burned out");
			return (1);
		}
		i++;
	}
	return (0);
}

/*
** Wakes up every coder currently blocked waiting on any dongle, so
** that each one immediately re-checks the stop flag (see
** wait_for_turn in scheduler.c) instead of only finding out the
** simulation ended once some unrelated event happens to wake it.
*/
static void	wake_all_dongles(t_env *env)
{
	int	i;

	i = 0;
	while (i < env->n_coders)
	{
		pthread_mutex_lock(&env->dongles[i].lock);
		pthread_cond_broadcast(&env->dongles[i].cond);
		pthread_mutex_unlock(&env->dongles[i].lock);
		i++;
	}
}

/*
** Thread-safe check of whether every coder has compiled at least
** compiles_required times.
*/
static int	all_compiled_enough(t_env *env)
{
	int	i;
	int	done;

	i = 0;
	while (i < env->n_coders)
	{
		pthread_mutex_lock(&env->state_lock);
		done = (env->coders[i].compiles_done >= env->compiles_required);
		pthread_mutex_unlock(&env->state_lock);
		if (done == 0)
			return (0);
		i++;
	}
	return (1);
}

/*
** Thread entry point for the monitor. Polls every 1ms, which keeps
** burnout detection well within the mandatory 10ms precision
** window. On burnout it terminates the whole process immediately:
** another coder thread may be permanently blocked (e.g. the n=1
** case, or simply mid-cooldown) and could never be safely joined,
*/
void	*monitor_routine(void *arg)
{
	t_env	*env;

	env = (t_env *)arg;
	while (1)
	{
		if (check_burnout(env))
		{
			pthread_mutex_lock(&env->stop_lock);
			env->simulation_stop = 1;
			pthread_mutex_unlock(&env->stop_lock);
			wake_all_dongles(env);
			return (NULL);
		}
		if (all_compiled_enough(env))
		{
			pthread_mutex_lock(&env->stop_lock);
			env->simulation_stop = 1;
			pthread_mutex_unlock(&env->stop_lock);
			wake_all_dongles(env);
			return (NULL);
		}
		usleep(1000);
	}
}
