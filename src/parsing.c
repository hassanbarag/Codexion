/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hassamoh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 15:18:45 by hassamoh          #+#    #+#             */
/*   Updated: 2026/10/03 16:12:10 by hassamoh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	str_to_long(const char *str, long *out)
{
	char	*end;
	long	value;

	if (str == NULL || *str == '\0')
		return (0);
	errno = 0;
	value = strtol(str, &end, 10);
	if (*end != '\0' || errno == ERANGE)
		return (0);
	*out = value;
	return (1);
}

/*
** Parses one argument that must be an integer.
** "min" is the smallest value accepted (0 or 1 depending on the
** field). Prints a clear error message on failure.
*/
static int	parse_field(const char *str, const char *name, long min,
	long *out)
{
	if (str_to_long(str, out) == 0)
	{
		fprintf(stderr, "codexion: error: '%s' must be an integer "
			"(got \"%s\")\n", name, str);
		return (0);
	}
	if (*out < min)
	{
		fprintf(stderr, "codexion: error: '%s' must be >= %ld "
			"(got %ld)\n", name, min, *out);
		return (0);
	}
	return (1);
}

/*
** Parses the "scheduler" argument, which must be exactly
** "fifo" or "edf".
*/
static int	parse_scheduler(const char *str, int *out)
{
	if (strcmp(str, "fifo") == 0)
		*out = SCHED_FIFO_TYPE;
	else if (strcmp(str, "edf") == 0)
		*out = SCHED_EDF_TYPE;
	else
	{
		fprintf(stderr, "codexion: error: 'scheduler' must be "
			"\"fifo\" or \"edf\" (got \"%s\")\n", str);
		return (0);
	}
	return (1);
}

/*
** Validates argc/argv and fills env with the parsed values.
** Returns 1 on success, 0 on the first invalid argument found.
*/
static int	parse_times(char **argv, t_env *env)
{
	if (parse_field(argv[2], "time_to_burnout", 1,
			&env->time_to_burnout) == 0)
		return (0);
	if (parse_field(argv[3], "time_to_compile", 1,
			&env->time_to_compile) == 0)
		return (0);
	if (parse_field(argv[4], "time_to_debug", 1,
			&env->time_to_debug) == 0)
		return (0);
	if (parse_field(argv[5], "time_to_refactor", 1,
			&env->time_to_refactor) == 0)
		return (0);
	return (1);
}

int	check_args(int argc, char **argv, t_env *env)
{
	long	tmp;

	if (argc != 9)
	{
		fprintf(stderr, "Usage: %s number_of_coders time_to_burnout "
			"time_to_compile time_to_debug time_to_refactor "
			"number_of_compiles_required dongle_cooldown scheduler\n",
			argv[0]);
		return (0);
	}
	memset(env, 0, sizeof(*env));
	if (parse_field(argv[1], "number_of_coders", 1, &tmp) == 0)
		return (0);
	env->n_coders = (int)tmp;
	if (parse_times(argv, env) == 0)
		return (0);
	if (parse_field(argv[6], "number_of_compiles_required", 1, &tmp) == 0)
		return (0);
	env->compiles_required = (int)tmp;
	if (parse_field(argv[7], "dongle_cooldown", 0,
			&env->dongle_cooldown) == 0)
		return (0);
	if (parse_scheduler(argv[8], &env->scheduler) == 0)
		return (0);
	return (1);
}
