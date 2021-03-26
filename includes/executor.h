/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   executor.h                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: mraasvel <mraasvel@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2020/12/26 11:39:49 by mraasvel      #+#    #+#                 */
/*   Updated: 2021/03/26 10:42:39 by mraasvel      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXECUTOR_H
# define EXECUTOR_H

# include "structs.h"

typedef int	(*t_builtin)(t_data *, char **);
typedef int	(*t_redirfct)(t_node *, char *);

int		isbuiltin(char *arg);
int		exec_builtin(t_node *node, t_data *data);

int		cmd_redirects(t_node *node);
void	search_path(t_data *data, char **dst, char *name);

#endif
