/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mime.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jquinodo <jquinodo@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:44:36 by jquinodo          #+#    #+#             */
/*   Updated: 2025/10/03 17:15:30 by jquinodo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MIME_H
# define MIME_H

#include "Webserv.hpp"

class Mime	{

private:

	std::map<std::string, std::string> _types;

public:

	Mime();

	std::string getMimeType(std::string extension);

};

#endif
