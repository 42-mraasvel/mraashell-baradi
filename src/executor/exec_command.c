/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   exec_command.c                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: mraasvel <mraasvel@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2021/03/20 08:39:24 by mraasvel      #+#    #+#                 */
/*   Updated: 2021/03/22 18:54:29 by mraasvel      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h> // rm
#include <errno.h> // rm
#include <stdlib.h>
#include <sys/wait.h>
#include "header.h"
#include "libft.h"
#include "executor.h"
#include "structs.h"
#include "proto.h"

void	print_command(t_node *node) // rm this function
{
	int	i;

	i = 0;
	while (node->args[i] != NULL)
	{
		if (i != 0)
			printf(" ");
		printf("%s", node->args[i]);
		i++;
	}
	printf("\n");
	if (node->exec_path != NULL)
		printf("%s\n", node->exec_path);
	printf("\tFD[0][1] : %d | %d\n", node->fds[0], node->fds[1]);
}

static void	close_fds(t_node *node)
{
	if (node->fds[0] != -1)
		if (close(node->fds[0]) == -1)
			ft_perror("close");
	if (node->fds[1] != -1)
		if (close(node->fds[1]) == -1)
			ft_perror("close");
	node->fds[0] = -1;
	node->fds[1] = -1;
}

static int	cmd_findpath(t_node *node, t_data *data)
{
	node->exec_path = NULL;
	if (ft_strchr(node->args[0], '/') != NULL)
		node->exec_path = malloc_guard(ft_strdup(node->args[0]));
	else
		search_path(data, &node->exec_path, node->args[0]);
	return (0);
}

static void	set_redirection(t_node *node)
{
	if (node->fds[0] != -1)
	{
		if (dup2(node->fds[0], STDIN_FILENO) == -1)
			exit_perror(GENERAL_ERROR, "dup2");
		if (close(node->fds[0]) == -1)
			ft_perror("close");
		node->fds[0] = -1;
	}
	if (node->fds[1] != -1)
	{
		if (dup2(node->fds[1], STDOUT_FILENO) == -1)
			exit_perror(GENERAL_ERROR, "dup2");
		if (close(node->fds[1]) == -1)
			ft_perror("close");
		node->fds[1] = -1;
	}
}

static int	finalize_cmd(t_node *node, t_data *data)
{
	int	pid;
	int	status;

	pid = fork();
	if (pid < 0)
	{
		ft_perror("fork");
		close_fds(node);
		return (-1);
	}
	if (pid == 0)
	{
		set_redirection(node);
		if (execve(node->exec_path, node->args, data->envp) == -1)
			exit_perror(GENERAL_ERROR, node->exec_path);
	}
	close_fds(node);
	waitpid(pid, &status, 0);
	if (WIFEXITED(status))
		data->exit_status = WEXITSTATUS(status);
	return (0);
}

int	exec_command(t_node *node, t_data *data)
{
	node->exec_path = NULL;
	if (isbuiltin(node->args[0]) != -1)
		return (exec_builtin(node, data));
	cmd_findpath(node, data);
	if (!ft_strchr(node->exec_path, '/') && getenv("PATH") || !file_exists(node->exec_path))
	{
		data->exit_status = CMD_NOT_FOUND;
		close_fds(node);
		return (set_error_vec(data, cmd_not_found, node->exec_path, 0));
	}
	return (finalize_cmd(node, data));
}
