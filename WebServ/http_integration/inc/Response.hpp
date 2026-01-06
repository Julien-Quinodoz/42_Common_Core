/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jquinodo <jquinodo@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:42:54 by jquinodo          #+#    #+#             */
/*   Updated: 2025/10/13 16:19:33 by jquinodo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_H
# define RESPONSE_H

# include "HttpRequest.hpp"
# include "Mime.hpp"
# include "CgiHandler.hpp"
# include "ServerConfig.hpp"

/*	Création et stockage de la réponse. Une fois prête, elle
	sera stockée dans _response_content et pourra être utilisée par la fonction getRes(). */
class Response
{
private:
	ServerConfig		_server;
	std::string			_target_file;
	std::vector<uint8_t>_body;
	size_t				_body_length;
	std::string			_response_body;
	std::string			_location;
	short				_code;
	int					_cgi;
	int					_cgi_fd[2];
	size_t				_cgi_response_length;
	bool				_auto_index;

	int		buildBody();
	void	setStatusLine();
	void	setHeaders();
	void	setServerDefaultErrorPages(); 
	int		readFile();
	void	contentType();
	void	contentLength();
	void	connection();
	void	server();	
	void	location();	
	void	date();
	int		handleTarget();
	void	buildErrorBody();
	bool	reqError();
	int		handleCgi(std::string &);
	int		handleCgiTemp(std::string &);

public:
	static	Mime 	mime;    // Objet Mime pour la gestion des types de contenu.
	CgiHandler		cgi_obj; // Objet CgiHandler pour la gestion des CGI.
	HttpRequest		request; // Objet HttpRequest pour la gestion des requêtes.

	 Response();
	~Response();
	Response(HttpRequest&);

/* getters */
	std::string	getRes();
	size_t		getLen() const;
	int			getCode() const;

/* setters */
	void	setRequest(HttpRequest &);
	void	setServer(ServerConfig &);

/* construction de la réponse */
	void	buildResponse();
	void	clear();
	void	cutRes(size_t);
	int		getCgiState();
	void	setCgiState(int);
	void	setErrorResponse(short code);

/* gestion des CGI */
	std::string	removeBoundary(std::string &body, std::string &boundary);
	std::string	response_content;

};

#endif

