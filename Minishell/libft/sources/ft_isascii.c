/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_isascii.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vfuster- <vfuster-@student.42perpignan.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/02 09:52:36 by vfuster-          #+#    #+#             */
/*   Updated: 2023/11/02 07:48:16 by vfuster-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/libft.h"

/******************************************************************************
 *
 * booleen indiquant si le caractere passe en argument est
 * un caractere ASCII valide. Si c'est le cas renvoie 1. Sinon renvoie 0
 *
 *****************************************************************************/
int	ft_isascii(int c)
{
	return (c >= 0 && c <= 127);
}
