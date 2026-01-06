/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jquinodo <jquinodo@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 14:14:34 by jquinodo          #+#    #+#             */
/*   Updated: 2025/12/11 17:59:42 by jquinodo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigFile.hpp"

ConfigFile::ConfigFile() : _size(0) {}

ConfigFile::~ConfigFile() {}

ConfigFile::ConfigFile(std::string const path) : _path(path), _size(0) {}

// définir si le chemin est un fichier (1), un dossier (2) ou autre chose (3)
int ConfigFile::getTypePath(std::string const path)
{
	struct stat	buffer;						//stat récupère les informations (métadonnées) ici dans une struct
	int			result;

	result = stat(path.c_str(), &buffer);
	if (result == 0)						// 0 → succès (le fichier existe)
	{										// Vérification du type avec **-> st_mode <-**
		if (buffer.st_mode & S_IFREG)		// S_IFREG : Regular file (fichier normal)
			return (1);
		else if (buffer.st_mode & S_IFDIR)	// S_IFDIR : Directory (dossier)
			return (2);
		else								// // Autre (lien symbolique, socket, pipe...)
			return (3);
	}
	else
		return (-1);
}

/* vérifie si le fichier existe et est accessible */
int	ConfigFile::checkFile(std::string const path, int mode) // mode spécifiant le type de vérification à effectuer (existence, lecture, écriture, etc.).
{
	//  F_OK	L'existence du fichier				0
	//  R_OK	La permission de Lecture (Read)		1
	//  W_OK	La permission d'Écriture (Write)	2
	//  X_OK	La permission d'Exécution (Execute) 3
	//  access --> vérifier les autorisations d'accès
	return (access(path.c_str(), mode));
}

int ConfigFile::isFileExistAndReadable(std::string const path, std::string const index)
{
	if (getTypePath(index) == 1 && checkFile(index, 4) == 0)	//index est peut-être déjà un chemin complet : index = "/var/www/html/index.html"
		return (0);												// sinon, concatène avec path pour construire le chemin : path = "/var/www/html/" + index = "index.html"
	if (getTypePath(path + index) == 1 && checkFile(path + index, 4) == 0)
		return (0);
	return (-1);
}

/* lecture du fichier vers une chaîne */
std::string	ConfigFile::readFile(std::string path)
{
	if (path.empty() || path.length() == 0)			// Vérifie que le chemin n'est pas vide
		return (NULL);

	std::ifstream config_file(path.c_str());		// .c_str() convertit le std::string en const char* (requis par le constructeur)
	if (!config_file || !config_file.is_open())		// Si le fichier n'existe pas ou n'a pas pu être ouvert → retourne NULL
		return (NULL);

	std::stringstream stream_binding;				// buffer pour manipuler des strings
	stream_binding << config_file.rdbuf();			// surcharge de << = copie tout le contenu du fichier dans le stringstream en une seule opération / rdbuf() = retourne le buffer brut du fichier
	return (stream_binding.str());					// Convertit le stringstream en std::string et le retourne
}

/* Fonctions Get */
std::string ConfigFile::getPath() {
	return (this->_path);
}

int ConfigFile::getSize() {
	return (this->_size);
}

