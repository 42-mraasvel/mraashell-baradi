/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   tree.c                                             :+:    :+:            */
/*                                                     +:+                    */
/*   By: mraasvel <mraasvel@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2021/03/12 11:28:38 by tel-bara      #+#    #+#                 */
/*   Updated: 2021/03/16 14:21:29 by mraasvel      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h> // rm
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "proto.h"
#include "structs.h"
#include "libft.h"
#include "libvect.h"

int	create_args(t_vect *tokens, size_t start, size_t end, t_node *node)
{
	t_token	token;
	int		count;
	int		i;
	int		j;

	i = start;
	while (i < end)
	{
		token = ((t_token*)tokens->table)[i];
		if ((token.type != operator && !(i - start)) || (token.type != operator && ((t_token*)tokens->table)[i - 1].type != operator))
			count++;
		i++;
	}
	node->args = malloc_guard(malloc((1 + count) * sizeof(char *)));
	if (node->args == 0)
		return (0);
	i = start;
	j = 0;
	while (i < end)
	{
		token = ((t_token*)tokens->table)[i];
		if (token.optype == redirect_in)
		{
			if (node->fds[0] != -1)
				if (close(node->fds[0]) == -1)
					return (0);
			i++;
			token = ((t_token*)tokens->table)[i];
			node->fds[0] = open(token.start, O_RDONLY);
			if (node->fds[0] == -1)
				return (0);
			free(token.start);
		}
		else if (token.optype == redirect_out)
		{
			if (node->fds[1] != -1)
				if (close(node->fds[1]) == -1)
					return (0);
			i++;
			token = ((t_token*)tokens->table)[i];
			node->fds[1] = open(token.start, (O_WRONLY | O_CREAT), 0644);
			if (node->fds[1] == -1)
				return (0);
			free(token.start);
		}
		else if (token.optype == redirect_append)
		{
			if (node->fds[1] != -1)
				if (close(node->fds[1]) == -1)
					return (0);
			i++;
			token = ((t_token*)tokens->table)[i];
			node->fds[1] = open(token.start, (O_WRONLY | O_APPEND | O_CREAT), 0644);
			if (node->fds[1] == -1)
				return (0);
			free(token.start);
		}
		else if (token.type == word)
		{
			node->args[j] = token.start;
			j++;
		}
		i++;
	}
	node->args[j] = 0;
	return (1);
}

t_node	*add_node(t_vect *tokens, size_t start, size_t end)
{
	t_node	*node;
	size_t	i;
	t_token	token;
	int		give_birth;
	int		count;

	if (start == end)
		return (NULL);
	node = (t_node*)malloc_guard(malloc(1 * sizeof(t_node)));
	if (node == 0)
		return (0);
	node->fds[0] = -1;
	node->fds[1] = -1;
	i = start;
	give_birth = 0;
	while (i < end && !give_birth)
	{
		if (((t_token*)tokens->table)[i].optype == o_semicolon)
		{
			give_birth = 1;
			node->left = add_node(tokens, start, i);
			node->right = add_node(tokens, i + 1, end);
			node->rule = semicolon;
		}
		i++;
	}
	i = start;
	while (i < end && !give_birth)
	{
		if (((t_token*)tokens->table)[i].optype == o_pipe)
		{
			give_birth = 1;
			node->left = add_node(tokens, start, i);
			node->right = add_node(tokens, i + 1, end);
			node->rule = t_pipe;
		}
		i++;
	}
	if (!give_birth)
	{
		node->rule = command;
		node->left = 0;
		node->right = 0;
		if (create_args(tokens, start, end, node) == 0)
			return (0);
	}
	return (node);
}

t_node	*create_tree(t_vect *tokens)
{
	t_node	*tree;

	tree = add_node(tokens, 0, tokens->nmemb);
	return (tree);
}
