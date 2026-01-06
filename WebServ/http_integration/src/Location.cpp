/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jquinodo <jquinodo@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 14:18:42 by jquinodo          #+#    #+#             */
/*   Updated: 2025/10/08 16:29:48 by jquinodo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

/* Initialisation par défaut.
	Seule la méthode GET est activée par défaut.
 Les valeurs du vecteur _methods correspondent à :

 | Index | Méthode | Autorisée |
 |--------|----------|------------|
 |   0    | GET      |    (1)     |
 |   1    | POST     |    (0)     |
 |   2    | DELETE   |    (0)     |
 |--------------------------------| */
Location::Location()
{
	this->_path = "";
	this->_root = "";
	this->_autoindex = false;
	this->_index = "";
	this->_return = "";
	this->_alias = "";
	this->_client_max_body_size = MAX_CONTENT_LENGTH;
	this->_methods.reserve(3);
	this->_methods.push_back(1);
	this->_methods.push_back(0);
	this->_methods.push_back(0);
}

Location::~Location() {}

Location::Location(const Location &src)
{
	this->_path 				= src._path;
	this->_root 				= src._root;
	this->_autoindex 			= src._autoindex;
	this->_index 				= src._index;
	this->_cgi_path 			= src._cgi_path;
	this->_cgi_ext 				= src._cgi_ext;
	this->_return 				= src._return;
	this->_alias 				= src._alias;
    this->_methods 				= src._methods;
	this->_ext_path 			= src._ext_path;
	this->_client_max_body_size = src._client_max_body_size;
}

Location &Location::operator=(const Location &src)
{
	if (this != &src)
	{
		this->_path 				= src._path;
		this->_root 				= src._root;
		this->_autoindex 			= src._autoindex;
		this->_index 				= src._index;
		this->_cgi_path 			= src._cgi_path;
		this->_cgi_ext 				= src._cgi_ext;
		this->_return 				= src._return;
		this->_alias 				= src._alias;
		this->_methods				= src._methods;
		this->_ext_path 			= src._ext_path;
		this->_client_max_body_size = src._client_max_body_size;
	}
	return (*this);
}

/****** Set functions ******/

void Location::setMethods(std::vector<std::string> methods)
{
	this->_methods[0] = 0;
	this->_methods[1] = 0;
	this->_methods[2] = 0;

	for (size_t i = 0; i < methods.size(); i++)
	{
		if (methods[i] == "GET")
			this->_methods[0] = 1;
		else if (methods[i] == "POST")
			this->_methods[1] = 1;
		else if (methods[i] == "DELETE")
			this->_methods[2] = 1;
		else
			throw ServerConfig::ErrorException("Allow method not supported " + methods[i]);
	}
}

void Location::setPath(std::string parametr){
	this->_path = parametr;
}

void Location::setRootLocation(std::string parametr){
	if (ConfigFile::getTypePath(parametr) != 2)			// 2 = dossier
		throw ServerConfig::ErrorException("root of location");
	this->_root = parametr;
}

void Location::setAutoindex(std::string parametr){
	if (parametr == "on" || parametr == "off")
		this->_autoindex = (parametr == "on");			// Si parametr == "on" → _autoindex = true
	else
		throw ServerConfig::ErrorException("Wrong autoindex");
}

void Location::setIndexLocation(std::string parametr){
	this->_index = parametr;
}

void Location::setReturn(std::string parametr){
	this->_return = parametr;
}

void Location::setAlias(std::string parametr){
	this->_alias = parametr;
}

void Location::setCgiPath(std::vector<std::string> path){
	this->_cgi_path = path;
}

void Location::setCgiExtension(std::vector<std::string> extension){
	this->_cgi_ext = extension;
}

void Location::setMaxBodySize(std::string parametr){
	unsigned long body_size = 0;

	for (size_t i = 0; i < parametr.length(); i++)
	{
		if (parametr[i] < '0' || parametr[i] > '9')
			throw ServerConfig::ErrorException("Wrong syntax: client_max_body_size");
	}
	if (!ft_stoi(parametr))
		throw ServerConfig::ErrorException("Wrong syntax: client_max_body_size");
	body_size = ft_stoi(parametr);
	this->_client_max_body_size = body_size;
}

void Location::setMaxBodySize(unsigned long parametr){
	this->_client_max_body_size = parametr;
}

/***** GET fonctions *****/
const std::string &Location::getPath() const{
	return (this->_path);
}

const std::string &Location::getRootLocation() const{
	return (this->_root);
}

const std::string &Location::getIndexLocation() const{
	return (this->_index);
}

const std::vector<short> &Location::getMethods() const{
	return (this->_methods);
}

const std::vector<std::string> &Location::getCgiPath() const{
	return (this->_cgi_path);
}

const std::vector<std::string> &Location::getCgiExtension() const{
	return (this->_cgi_ext);
}

const bool &Location::getAutoindex() const{
	return (this->_autoindex);
}

const std::string &Location::getReturn() const{
	return (this->_return);
}

const std::string &Location::getAlias() const{
	return (this->_alias);
}

const std::map<std::string, std::string> &Location::getExtensionPath() const{
	return (this->_ext_path);
}

const unsigned long &Location::getMaxBodySize() const{
	return (this->_client_max_body_size);
}

/**** Pour imprimer les méthodes autorisées (pour contrôle)****/
std::string Location::getPrintMethods() const
{
	std::string res;
	if (_methods[2])
	{
		if (!res.empty())
			res.insert(0, ", ");
		res.insert(0, "DELETE");
	}
	if (_methods[1])
	{
		if (!res.empty())
			res.insert(0, ", ");
		res.insert(0, "POST");
	}
	if (_methods[0])
	{
		if (!res.empty())
			res.insert(0, ", ");
		res.insert(0, "GET");
	}
	return (res);
}

