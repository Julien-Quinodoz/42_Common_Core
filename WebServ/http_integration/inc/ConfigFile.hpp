/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jquinodo <jquinodo@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 14:14:44 by jquinodo          #+#    #+#             */
/*   Updated: 2025/10/03 17:15:20 by jquinodo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGFILE_H
# define CONFIGFILE_H

#include "Webserv.hpp"

class ConfigFile {

private:
	std::string	_path;
	size_t		_size;

public:
	ConfigFile();
	~ConfigFile();
	ConfigFile(std::string const path);

	std::string getPath();
			int	getSize();

	static	int getTypePath(std::string const path); 			// définir si le chemin est un fichier (1), un dossier (2) ou autre chose (3)
	static	int checkFile(std::string const path, int mode);	// vérifie si le fichier existe et est accessible
	std::string	readFile(std::string path);
	static	int isFileExistAndReadable(std::string const path, std::string const index);

};

#endif
