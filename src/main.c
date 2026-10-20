/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hassamoh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 15:18:15 by hassamoh          #+#    #+#             */
/*   Updated: 2026/10/03 15:18:17 by hassamoh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_env		env;
	int			i;
	pthread_t	monitor;

	if (check_args(argc, argv, &env) == 0)
		return (1);
	if (init_env(&env) == 0)
		return (1);
	i = 0;
	while (i < env.n_coders)
	{
		pthread_create(&env.coders[i].thread, NULL, coder_routine,
			&env.coders[i]);
		i++;
	}
	pthread_create(&monitor, NULL, monitor_routine, &env);
	pthread_join(monitor, NULL);
	i = 0;
	while (i < env.n_coders)
	{
		pthread_join(env.coders[i].thread, NULL);
		i++;
	}
	cleanup_env(&env);
	return (0);
}
