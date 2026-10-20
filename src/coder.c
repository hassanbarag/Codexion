/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hassamoh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 15:15:16 by hassamoh          #+#    #+#             */
/*   Updated: 2026/10/03 15:15:18 by hassamoh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Thread-safe read of env->simulation_stop.
** Every coder checks this at the top of its loop so it stops as
** soon as the monitor thread flags the end of the simulation,
** without needing any global variable.
*/
static int	is_running(t_env *env)
{
	int	stop;

	pthread_mutex_lock(&env->stop_lock);
	stop = env->simulation_stop;
	pthread_mutex_unlock(&env->stop_lock);
	return (!stop);
}

/*
** One full compile/debug/refactor cycle for a coder:
** - take both dongles (blocks until available)
** - record last_compile_start the instant compiling truly starts
**   (under state_lock: the monitor thread reads this concurrently)
** - hold the dongles for time_to_compile ms, then release them
** - spend time_to_debug ms debugging, then time_to_refactor ms
**   refactoring, logging each state change as required
*/
static void	run_cycle(t_coder *coder)
{
	t_env	*env;

	env = coder->env;
	if (take_dongles(coder) == 0)
		return ;
	pthread_mutex_lock(&env->state_lock);
	coder->last_compile_start = get_timestamp();
	pthread_mutex_unlock(&env->state_lock);
	log_state(env, coder->id, "is compiling");
	usleep(env->time_to_compile * 1000);
	release_dongles(coder);
	pthread_mutex_lock(&env->state_lock);
	coder->compiles_done++;
	pthread_mutex_unlock(&env->state_lock);
	log_state(env, coder->id, "is debugging");
	usleep(env->time_to_debug * 1000);
	log_state(env, coder->id, "is refactoring");
	usleep(env->time_to_refactor * 1000);
}

/*
** Thread entry point for one coder: runs cycles back-to-back until
** the simulation is flagged as stopped.
*/
void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (is_running(coder->env))
		run_cycle(coder);
	return (NULL);
}
