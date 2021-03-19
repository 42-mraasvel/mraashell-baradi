/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   call_builtins.c                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: mraasvel <mraasvel@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2021/03/16 08:42:06 by mraasvel      #+#    #+#                 */
/*   Updated: 2021/03/19 12:06:27 by mraasvel      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h> // rm
#include <stdlib.h>
#include "executor.h"
#include "libft.h"
#include "proto.h"

int	isbuiltin(char *arg)
{
	int					i;
	static const char	*builtins[] = {
		"echo",
		"cd",
		"pwd",
		"export",
		"unset",
		"env",
		"exit",
		NULL
	};

	i = 0;
	while (builtins[i] != NULL)
	{
		if (ft_strcmp(builtins[i], arg) == 0)
			return (i);
		i++;
	}
	return (-1);
}

static t_builtin	get_builtin(char *name)
{
	int						i;
	static const t_builtin	builtins[] = {
		ft_echo,
		NULL,
		ft_pwd,
		ft_export,
		ft_unset,
		ft_env,
		ft_exit
	};

	i = isbuiltin((name));
	if (i < 0 || i > 6)
		return (NULL);
	return (builtins[i]);
}

/*
** What are the errors?
** How do we handle these errors?
** Print an error message and continue?
** Or exit the program (probably not this one)
**
**	1. Save replaces FDs
**	2. Replace STD FDs
**	3. Close the one in node because they're unused
*/

//! When back: slack + error checking these syscalls (dup, dup2, close)
static int	set_fds_builtin(t_data *data, t_node *node, int fds[2])
{
	if (node->fds[0] != -1)
	{
		fds[0] = dup(STDIN_FILENO);
		if (fds[0] == -1)
			return (-1);
		if (dup2(node->fds[0], STDIN_FILENO) == -1)
		{
			close(fds[0]);
			fds[0] = -1;
			return (-1);
		}
		close(node->fds[0]);
	}
	if (node->fds[1] != -1)
	{
		fds[1] = dup(STDOUT_FILENO);
		if (fds[1] == -1)
			return (-1);
		if (dup2(node->fds[1], STDOUT_FILENO) == -1)
		{
			close(fds[1]);
			fds[1] = -1;
			return (-1);
		}
		close(node->fds[1]);
	}
}

static int	reset_fds_builtin(int fds[2])
{
	if (fds[0] != -1)
	{
		dup2(fds[0], STDIN_FILENO);
		close(fds[0]);
	}
	if (fds[1] != -1)
	{
		dup2(fds[1], STDOUT_FILENO);
		close(fds[1]);
	}
}

/*
** This should dup2 file descriptors
** Not fork the process though
*/

int	exec_builtin(t_node *node, t_data *data)
{
	t_builtin	fnct;
	int			fd_copy[2];

	fnct = get_builtin(node->args[0]);
	if (fnct == NULL)
		return (-1);
	set_fds_builtin(data, node, fd_copy);
	data->exit_status = fnct(data, node->args);
	reset_fds_builtin(fd_copy);
	if (data->error.errnum != success)
		;//! Handle error output
	return (data->exit_status);
}

/*
** Array of pointers would be nice I guess
*/

int	call_builtins(t_data *data, t_node *node)
{
	if (execve(node->args[0], node->args, data->envp) == -1)
	{
		perror("-bash");
		exit_program(syscall_error, "Execve Error in child process");
	}
	return (0);
}
