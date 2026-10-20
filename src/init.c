/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hassamoh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 15:17:47 by hassamoh          #+#    #+#             */
/*   Updated: 2026/10/03 17:13:37 by hassamoh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Initialises one dongle: no one holds it, it is immediately
** available (no cooldown pending), its cooldown duration is copied
** from env, and its waiting queue starts empty.
*/
static void	init_dongle(t_dongle *dongle, int id, t_env *env)
{
	dongle->id = id;
	dongle->in_use = 0;
	dongle->available_at = env->start_time;
	dongle->cooldown = env->dongle_cooldown;
	dongle->next_seq = 0;
	dongle->queue.size = 0;
	pthread_mutex_init(&dongle->lock, NULL);
	pthread_cond_init(&dongle->cond, NULL);
}

/*
** Initialises one coder. Coders and dongles sit in a circle:
** coder i (0-indexed) uses dongles[i] as its left dongle and
** dongles[(i + 1) % n] as its right dongle. With a single coder
** both point to the same (only) dongle, as required.
*/
static void	init_coder(t_env *env, int index)
{
	t_coder	*coder;

	coder = &env->coders[index];
	coder->id = index + 1;
	coder->last_compile_start = env->start_time;
	coder->compiles_done = 0;
	coder->left = &env->dongles[index];
	coder->right = &env->dongles[(index + 1) % env->n_coders];
	coder->env = env;
}

/*
** Allocates the dongles/coders arrays and initialises every
** mutex/cond and starting value needed by the simulation.
** Returns 1 on success, 0 on allocation failure.
*/
static int	allocate_env(t_env *env)
{
	env->dongles = malloc(sizeof(t_dongle) * env->n_coders);
	env->coders = malloc(sizeof(t_coder) * env->n_coders);
	if (env->dongles == NULL || env->coders == NULL)
	{
		fprintf(stderr, "codexion: error: allocation failed\n");
		free(env->dongles);
		free(env->coders);
		return (0);
	}
	return (1);
}

int	init_env(t_env *env)
{
	int	i;

	env->start_time = get_timestamp();
	env->simulation_stop = 0;
	pthread_mutex_init(&env->stop_lock, NULL);
	pthread_mutex_init(&env->log_lock, NULL);
	pthread_mutex_init(&env->state_lock, NULL);
	if (allocate_env(env) == 0)
		return (0);
	i = 0;
	while (i < env->n_coders)
	{
		init_dongle(&env->dongles[i], i, env);
		i++;
	}
	i = 0;
	while (i < env->n_coders)
	{
		init_coder(env, i);
		i++;
	}
	return (1);
}

/*
** Destroys every mutex/cond created by init_env and frees the
** dongles/coders arrays. Must be called exactly once, after all
** threads have been joined.
*/
void	cleanup_env(t_env *env)
{
	int	i;

	i = 0;
	while (i < env->n_coders)
	{
		pthread_mutex_destroy(&env->dongles[i].lock);
		pthread_cond_destroy(&env->dongles[i].cond);
		i++;
	}
	pthread_mutex_destroy(&env->stop_lock);
	pthread_mutex_destroy(&env->log_lock);
	pthread_mutex_destroy(&env->state_lock);
	free(env->dongles);
	free(env->coders);
}
