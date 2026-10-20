/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hassamoh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 15:16:22 by hassamoh          #+#    #+#             */
/*   Updated: 2026/10/05 12:58:41 by hassamoh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Picks the acquisition order for one coder's two dongles: the
** dongle with the smaller id is always requested first, regardless
** of whether it is physically "left" or "right".
*/
static void	ordered_pair(t_coder *coder, t_dongle **first, t_dongle **second)
{
	if (coder->left->id < coder->right->id)
	{
		*first = coder->left;
		*second = coder->right;
	}
	else
	{
		*first = coder->right;
		*second = coder->left;
	}
}

/*
** The coder's burnout deadline: last_compile_start + time_to_burnout.
** It is the priority key used by the edf scheduler (read under
** state_lock, since the monitor and the coder itself share it).
*/
static long	get_deadline(t_coder *coder)
{
	long	deadline;

	pthread_mutex_lock(&coder->env->state_lock);
	deadline = coder->last_compile_start + coder->env->time_to_burnout;
	pthread_mutex_unlock(&coder->env->state_lock);
	return (deadline);
}

/*
** Acquires both of the coder's dongles (lower id first, see
** ordered_pair) through the active scheduler (fifo or edf),
** logging a "has taken a dongle" line after each one, as required
** by the mandatory log format.
** Returns 1 once both are held. Returns 0 if the simulation stops
*/
int	take_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	t_env		*env;
	long		deadline;

	env = coder->env;
	ordered_pair(coder, &first, &second);
	deadline = get_deadline(coder);
	if (dongle_wait(first, env, env->scheduler, deadline) == 0)
		return (0);
	log_state(env, coder->id, "has taken a dongle");
	if (dongle_wait(second, env, env->scheduler, deadline) == 0)
	{
		dongle_release(first);
		return (0);
	}
	log_state(env, coder->id, "has taken a dongle");
	return (1);
}

/*
** Releases both of the coder's dongles after compiling. Release
** order does not matter: releasing can never block or deadlock.
*/
void	release_dongles(t_coder *coder)
{
	dongle_release(coder->left);
	dongle_release(coder->right);
}
