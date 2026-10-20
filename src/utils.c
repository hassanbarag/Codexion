/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hassamoh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 15:19:09 by hassamoh          #+#    #+#             */
/*   Updated: 2026/10/03 15:19:12 by hassamoh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Returns the current time in milliseconds, based on the system
** real-time clock (gettimeofday). This is an absolute value; it
** only becomes meaningful once compared to another call of this
** same function (e.g. env->start_time, a dongle's available_at,
** or a coder's last_compile_start).
*/
long	get_timestamp(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000L + tv.tv_usec / 1000L);
}
