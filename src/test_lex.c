/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   test_lex.c                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: mraasvel <mraasvel@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2021/03/02 23:24:47 by mraasvel      #+#    #+#                 */
/*   Updated: 2021/03/14 21:24:15 by mraasvel      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include "lexer.h"
#include "libft.h"
#include "proto.h"

char	*skipspace(char *line)
{
	//! Skip only	 space + tabs, or all whitespace as defined in POSIX?
	while (ft_isspace(*line))
		line++;
	// while (*line == ' ' || *line == '\t')
	// 	line++;
	return (line);
}

/*
<metacharacter>	::=	::=	'|'
					|	'<'
					|	'>'
					|	';'
					|	<blank>
					|	'\n' (newline)
				NOT IN SUBJECT:
					|	'&'
					|	'('
					|	')'
*/

t_bool	ismeta(char c)
{
	static const char	*meta = "|<>; \t\n";

	if (ft_strchr(meta, c) == NULL)
		return (false);
	return (true);
}

t_optype	get_optype(char c)
{
	const char	*optypes;
	int			i;

	optypes = "><;|";
	i = 0;
	while (optypes[i])
	{
		if (c == optypes[i])
			break ;
		i++;
	}
	return (i);
}

t_token	parse_operator(char *line)
{
	t_token	token;

	token.start = line;
	token.length = 0;
	token.type = operator;
	token.length = 1;
	if (*line == '>' && *(line + 1) == '>')
	{
		token.length += 1;
		token.optype = redirect_append;
	}
	else
		token.optype = get_optype(*line);
	return (token);
}

t_token	parse_error(char *line, t_token token)
{
	token.type = bad_token;
	token.length = ft_strlen(line);
	return (token);
}

t_token	parse_quotes(char *line, t_token token)
{
	char	*ptr;

	ptr = ft_strchr(line + 1, *line);
	if (ptr == NULL)
		return (parse_error(line, token));
	token.length = ptr - line + 1;
	if (*line == '\'')
		token.type = singlequoted;
	else if (*line == '"')
		token.type = doublequoted;
	return (token);
}

int	isquote(char c)
{
	if (c == '\'' || c == '"')
		return (1);
	return (0);
}

t_token	parse_word(char *line)
{
	t_token	token;
	char	*ptr;

	token.start = line;
	token.length = 0;
	token.type = unquoted;
	if (*line == '\'' || *line == '"')
		return (parse_quotes(line, token));
	while (!ismeta(*(line + token.length)))
	{
		if (isquote(*(line + token.length)))
		{
			ptr = ft_strchr(line + token.length + 1, *(line + token.length));
			if (ptr == NULL)
				return (parse_error(line, token));
			token.length = ptr - line;
		}
		token.length++;
	}
	return (token);
}

t_vect	*test_lexer(char *line)
{
	t_vect	*tokens;
	t_token	token;

	tokens = malloc_guard(vect_init(0, sizeof(t_token)));
	while (*line != '\0')
	{
		line = skipspace(line);
		if (*line == '\0')
			return (tokens);
		if (ismeta(*line))
			token = parse_operator(line);
		else
			token = parse_word(line);
		if (vect_pushback(tokens, &token) == -1)
		{
			vect_free(tokens, NULL);
			exit_program(error, "Pushback failed\n");
		}
		line += token.length;
	}
	return (tokens);
}

void	test_lex(void)
{
	int		ret;
	char	*line;
	t_vect	*tokens;
	t_node	*root;

	ret = 1;
	while (ret > 0)
	{
		write(1, MINISHELL_PROMPT, sizeof(MINISHELL_PROMPT));
		ret = ft_getline(0, &line);
		if (ret < 0)
			exit_program(error, "Getline error");
		if (ret != 0)
		{
			tokens = test_lexer(line);
			if (tokens->nmemb != 0)
			{
				print_tokens(tokens);
				root = create_tree(tokens);
				if (root == NULL)
					perror("-bash");
				print_tree_depth(root, 0);
				execute_tree(root);
				tree_free(root);
			}
			vect_free(tokens, NULL);
		}
		free(line);
	}
}
