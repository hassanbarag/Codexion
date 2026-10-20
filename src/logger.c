/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hassamoh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 15:18:04 by hassamoh          #+#    #+#             */
/*   Updated: 2026/10/03 15:18:06 by hassamoh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Prints one state-change line for a coder, formatted as required:
** "<elapsed_ms> <coder_id> <state>".
** The elapsed time is computed relative to env->start_time so that
** the very first event of the simulation is logged at time 0.
** env->log_lock guarantees that two threads can never interleave
** their output on the same line.
*/
void	log_state(t_env *env, int coder_id, const char *state)
{
	long	elapsed;

	pthread_mutex_lock(&env->log_lock);
	elapsed = get_timestamp() - env->start_time;
	printf("%ld %d %s\n", elapsed, coder_id, state);
	pthread_mutex_unlock(&env->log_lock);
}
