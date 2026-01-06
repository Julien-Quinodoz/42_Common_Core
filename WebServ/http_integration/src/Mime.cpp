/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mime.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jquinodo <jquinodo@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:45:07 by jquinodo          #+#    #+#             */
/*   Updated: 2025/11/11 17:19:40 by jquinodo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Mime.hpp"

/* Mime::Mime() est une classe qui mappe les extensions de fichiers
				(.html, .css, .png, etc.)
	vers leurs types MIME pour les headers HTTP Content-Type de retour
				("text/html", "image/png",...) */

Mime::Mime()
{
	_types[".html"] = "text/html";
	_types[".htm"] = "text/html";
	_types[".css"] = "text/css";
	_types[".ico"] = "image/x-icon";
	_types[".avi"] = "video/x-msvideo";
	_types[".bmp"] = "image/bmp";
	_types[".doc"] = "application/msword";
	_types[".gif"] = "image/gif";
	_types[".gz"] = "application/x-gzip";
	_types[".ico"] = "image/x-icon";
	_types[".jpg"] = "image/jpeg";
	_types[".jpeg"] = "image/jpeg";
	_types[".png"] = "image/png";
	_types[".txt"] = "text/plain";
	_types[".mp3"] = "audio/mp3";
	_types[".pdf"] = "application/pdf";
	_types["default"] = "text/html";
}

std::string Mime::getMimeType(std::string extension)
{
	if (_types.count(extension))
		return (_types[extension]);
	return (_types["default"]);
}

