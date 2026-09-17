/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   get_next_line_bonus.c                             :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: kseltenr <kseltenr@student.42.fr>         #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/09/11 03:06:54 by kseltenr         #+#    #+#              */
/*   Updated: 2026/09/17 22:39:52 by kseltenr        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line_bonus.h"
#include <stdlib.h>
#include <unistd.h>

int	find_line(buffer buf)
{
	int	i;

	if (!buf.b_buf)
		return (1);
	i = 0;
	while (buf.b_buf[i])
	{
		if (buf.b_buf[i] == '\n')
			return (0);
		i++;
	}
	return (1);
}

void	read_to_buf(int fd, buffer *buf)
{
	char	*temp;
	int		i;

	i = 0;
	temp = malloc(sizeof(char) * (BUFFER_SIZE + 1));
	if (!temp)
		return ;
	while (find_line(*buf))
	{
		i = read(fd, temp, BUFFER_SIZE);
		if (i == -1)
		{
			free(temp);
			free(buf->b_buf);
			buf->b_buf = NULL;
			return ;
		}
		else if (i == 0)
			break ;
		temp[i] = '\0';
		buf->b_buf = ft_strjoin(&*buf, temp, i);
	}
	free(temp);
	return ;
}

buffer	*create_node(int fd)
{
	buffer	*new_node;

	new_node = malloc(sizeof(buffer));
	if (!new_node)
		return (NULL);
	new_node->b_cap = 256;
	new_node->b_len = 0;
	new_node->b_id = fd;
	new_node->next = NULL;
	new_node->b_buf = malloc(new_node->b_cap);
	if (!new_node->b_buf)
	{
		free(new_node);
		return (NULL);
	}
	new_node->b_buf[0] = '\0';
	return (new_node);
}

buffer	*find_node(int fd, buffer *buf)
{
	buffer	*current;

	current = buf;
	while (current->b_id != fd)
	{
		if (current->next == NULL)
		{
			current->next = create_node(fd);
			if (!current->next)
				return (NULL);
			current = current->next;
			break ;
		}
		current = current->next;
	}
	if (!current->b_buf)
	{
		current->b_cap = 256;
		current->b_len = 0;
		current->b_buf = malloc(current->b_cap);
		if (!current->b_buf)
			return (NULL);
		current->b_buf[0] = '\0';
	}
	return (current);
}

char	*get_next_line(int fd)
{
	static buffer		buf;
	buffer				*current;
	char				*ret;

	current = find_node(fd, &buf);
	if (!current->b_buf)
		return (NULL);
	read_to_buf(fd, current);
	if (!current->b_buf)
		return (NULL);
	if (current->b_buf[0] == '\0')
	{
		free(current->b_buf);
		current->b_buf = NULL;
		return (NULL);
	}
	ret = return_line(*current);
	current->b_buf = delete_line(current, ft_strlen(ret));
	return (ret);
}
