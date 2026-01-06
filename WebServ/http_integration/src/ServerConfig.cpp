/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jquinodo <jquinodo@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 14:15:29 by jquinodo          #+#    #+#             */
/*   Updated: 2025/11/19 10:46:28 by jquinodo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig()
{
	this->_port = 0;
	this->_host = 0;
	this->_server_name = "";
	this->_root = "";
	this->_client_max_body_size = MAX_CONTENT_LENGTH;
	this->_index = "";
	this->_listen_fd = 0;
	this->_autoindex = false;
	this->initErrorPages();
}

ServerConfig::~ServerConfig() { }

/* constructeur de copie */
ServerConfig::ServerConfig(const ServerConfig &src)
{
	if (this != &src)
	{
		this->_server_name 			= src._server_name;
		this->_root 				= src._root;
		this->_host 				= src._host;
		this->_port 				= src._port;
		this->_client_max_body_size = src._client_max_body_size;
		this->_index 				= src._index;
		this->_error_pages 			= src._error_pages;
		this->_locations 			= src._locations;
		this->_listen_fd 			= src._listen_fd;
		this->_autoindex 			= src._autoindex;
		this->_server_address 		= src._server_address;
	}
	return ;
}

/* operateur assignement =  */
ServerConfig &ServerConfig::operator = (const ServerConfig &src)
{
	if (this != &src)
	{
		this->_server_name 			= src._server_name;
		this->_root 				= src._root;
		this->_port 				= src._port;
		this->_host 				= src._host;
		this->_client_max_body_size = src._client_max_body_size;
		this->_index 				= src._index;
		this->_error_pages 			= src._error_pages;
		this->_locations 			= src._locations;
		this->_listen_fd 			= src._listen_fd;
		this->_autoindex 			= src._autoindex;
		this->_server_address 		= src._server_address;
	}
	return (*this);
}

/* initialisation des pages error par défaut */
void ServerConfig::initErrorPages(void)
{
	_error_pages[301] = "";
	_error_pages[302] = "";
	_error_pages[400] = "";
	_error_pages[401] = "";
	_error_pages[402] = "";
	_error_pages[403] = "";
	_error_pages[404] = "";
	_error_pages[405] = "";
	_error_pages[406] = "";
	_error_pages[500] = "";
	_error_pages[501] = "";
	_error_pages[502] = "";
	_error_pages[503] = "";
	_error_pages[504] = "";
	_error_pages[505] = "";
}
/* 	╔═══════════════════════════════════════════════════════════════╗
	║				CODES HTTP - PLAGES VALIDES						║
 	╠═══════════════════════════════════════════════════════════════╣
 	║ 1xx (100-199) │ Informations    │ Ex: 100 Continue			║
 	║ 2xx (200-299) │ Succès          │ Ex: 200 OK					║
 	║ 3xx (300-399) │ Redirections    │ Ex: 301, 302				║
 	║ 4xx (400-499) │ Erreurs client  │ Ex: 404, 403				║
 	║ 5xx (500-599) │ Erreurs serveur │ Ex: 500, 502				║
 	╚═══════════════════════════════════════════════════════════════╝ */

/*** Set functions ***/

void ServerConfig::setServerName(std::string server_name)
{
	checkToken(server_name);
	this->_server_name = server_name;
}

void ServerConfig::setHost(std::string parametr)
{
	checkToken(parametr);
	if (parametr == "localhost")
		parametr = "127.0.0.1";
	if (!isValidHost(parametr))
		throw ErrorException("Wrong syntax: host");
	this->_host = inet_addr(parametr.data());		// convertit une adresse IP au format texte (par exemple, "192.168.1.1") en une adresse IP au format binaire
}

void ServerConfig::setRoot(std::string root)
{
	checkToken(root);
	if (ConfigFile::getTypePath(root) == 2)			// définir si le chemin est un un dossier (2)
	{
		this->_root = root;
		return ;
	}
	char dir[1024];
	getcwd(dir, 1024);								// obtient le répertoire courant et le stocke dans le tableau dir
	std::string full_root = dir + root;				// Le chemin root est concaténé au répertoire courant pour former un chemin complet full_root.
	if (ConfigFile::getTypePath(full_root) != 2)
		throw ErrorException("Wrong syntax: root");
	this->_root = full_root;
}

void ServerConfig::setPort(std::string parametr)
{
	unsigned int port;

	port = 0;
	checkToken(parametr);
	for (size_t i = 0; i < parametr.length(); i++)
	{
		if (!std::isdigit(parametr[i]))
			throw ErrorException("Wrong syntax: port");
	}
	port = ft_stoi((parametr));
	if (port < 1 || port > 65535)
		throw ErrorException("Wrong syntax: port");
	this->_port = (uint16_t) port;
}

void ServerConfig::setClientMaxBodySize(std::string parametr)
{
	unsigned long body_size;

	body_size = 0;
	checkToken(parametr);
	for (size_t i = 0; i < parametr.length(); i++)
	{
		if (parametr[i] < '0' || parametr[i] > '9')		//isnum
			throw ErrorException("Wrong syntax: client_max_body_size");
	}
	if (!ft_stoi(parametr))							// ft_stoi conversion de chaîne en entier non signé qui retourne un booléen indiquant si la conversion a réussi ou échoué.
		throw ErrorException("Wrong syntax: client_max_body_size");
	body_size = ft_stoi(parametr);
	this->_client_max_body_size = body_size;
}

void ServerConfig::setIndex(std::string index)
{
	checkToken(index);
	this->_index = index;
}

// Configure la propriété _autoindex (on/off) et active l'indexation automatique si "on".
void ServerConfig::setAutoindex(std::string autoindex)
{
	checkToken(autoindex);
	if (autoindex != "on" && autoindex != "off")
		throw ErrorException("Wrong syntax: autoindex");
	if (autoindex == "on")
		this->_autoindex = true;
}

/* vérifie si un code d'erreur par défaut existe.
	Si c'est le cas, le chemin d'accès au fichier est remplacer.
	 Sinon, une nouvelle paire est créée : code d'erreur - chemin d'accès au fichier. */
void ServerConfig::setErrorPages(std::vector<std::string> &parametr)
{
	if (parametr.empty())
		return;
	if (parametr.size() % 2 != 0)								// Vérifie que le nombre d’éléments dans parametr est pair ( les codes d’erreur et chemins doivent former des paires)
		throw ErrorException ("Error page initialization faled");
	for (size_t i = 0; i < parametr.size() - 1; i++)			// Parcourt la partie code de la paire
	{
		for (size_t j = 0; j < parametr[i].size(); j++) {
			if (!std::isdigit(parametr[i][j]))					// doit'être un nombre digital
				throw ErrorException("Error code is invalid");
		}
		if (parametr[i].size() != 3)							// 3 chiffres exactement
			throw ErrorException("Error code is invalid");

		short code_error = ft_stoi(parametr[i]);				//short est généralement deux fois plus petit qu'un int

		if (statusCodeString(code_error)  == "Undefined" || code_error < 400)	// Si code_error < 400, ce n’est pas un code d’erreur
			throw ErrorException ("Incorrect error code: " + parametr[i]);

		i++;	// Parcourt parametr par incréments de 2 (chaque itération traite une paire code/chemin) --> Passe au prochain élément de parametr (le chemin, par exemple, "/404.html")

		std::string path = parametr[i];							// Stocke le chemin dans path

		checkToken(path);
		if (ConfigFile::getTypePath(path) != 2)					// !2 si ce n'est pas un dossier
		{
			if (ConfigFile::getTypePath(this->_root + path) != 1) // Vérifie le chemin complet --> exception si ce n'est pas un fichier
				throw ErrorException ("Incorrect path for error page file: " + this->_root + path);
			if (ConfigFile::checkFile(this->_root + path, 0) == -1 || ConfigFile::checkFile(this->_root + path, 4) == -1) // : Vérifient si le fichier existe (0) et est lisible (4). Si l’un échoue, une exception est levée.
				throw ErrorException ("Error page file :" + this->_root + path + " is not accessible");
		}
		/* Recherche si code_error existe dans _error_pages (une std::map associant codes à chemins).
				Si trouvé (it != end()), remplace le chemin existant par path.
						Sinon, insère une nouvelle paire (code_error, path). */
		std::map<short, std::string>::iterator it = this->_error_pages.find(code_error);
		if (it != _error_pages.end())
			this->_error_pages[code_error] = path;
		else
			this->_error_pages.insert(std::make_pair(code_error, path));
	}
}

/* analyse et définition des emplacements
	--> prend un chemin et une liste de paramètres, puis crée et configure un objet Location avec ces paramètres. */
void ServerConfig::setLocation(std::string path, std::vector<std::string> parametr)
{
	Location new_location;									// Crée un nouvel objet et initialise des flags pour éviter les duplications de paramètres
	std::vector<std::string> methods;
	bool flag_methods = false;
	bool flag_autoindex = false;
	bool flag_max_size = false;
	int valid;

	new_location.setPath(path);

	for (size_t i = 0; i < parametr.size(); i++)
	{
		if (parametr[i] == "root" && (i + 1) < parametr.size())		//  Définit le répertoire racine
		{
			if (!new_location.getRootLocation().empty())
			{
				throw ErrorException("Root of location is duplicated");
			}
			checkToken(parametr[++i]);
			if (ConfigFile::getTypePath(parametr[i]) == 2)			// Si le chemin est absolu (type 2), l'utilise tel quel
				new_location.setRootLocation(parametr[i]);
			else
				new_location.setRootLocation(this->_root + parametr[i]); // Sinon, le concatène avec _root du serveur
		}

		else if ((parametr[i] == "allow_methods" || parametr[i] == "methods") && (i + 1) < parametr.size()) // Définit les méthodes HTTP autorisées
		{
			if (flag_methods)
				throw ErrorException("Allow_methods of location is duplicated");

			std::vector<std::string> methods;

			while (++i < parametr.size())
			{
				if (parametr[i].find(";") != std::string::npos)			// Lit plusieurs valeurs jusqu'à trouver un ;
				{
					checkToken(parametr[i]);
					methods.push_back(parametr[i]);
					break ;
				}
				else
				{
					methods.push_back(parametr[i]);
					if (i + 1 >= parametr.size())
						throw ErrorException("Token is invalid");
				}
			}
			new_location.setMethods(methods);							// Stocke GET, POST, DELETE, etc.
			flag_methods = true;
		}

		else if (parametr[i] == "autoindex" && (i + 1) < parametr.size()) // Active/désactive le listing des répertoires automatique
		{
			if (path == "/cgi-bin")
				throw ErrorException("Parametr autoindex not allow for CGI");
			if (flag_autoindex)
				throw ErrorException("Autoindex of location is duplicated");

			checkToken(parametr[++i]);
			new_location.setAutoindex(parametr[i]);
			flag_autoindex = true;
		}

		else if (parametr[i] == "index" && (i + 1) < parametr.size()) // Définit le fichier index par défaut: index.html
		{
			if (!new_location.getIndexLocation().empty())
				throw ErrorException("Index of location is duplicated");

			checkToken(parametr[++i]);
			new_location.setIndexLocation(parametr[i]);
		}

		else if (parametr[i] == "return" && (i + 1) < parametr.size()) //  Définit une redirection
		{
			if (path == "/cgi-bin")
				throw ErrorException("Parametr return not allow for CGI"); // interdite pour "/cgi-bin"
			if (!new_location.getReturn().empty())
				throw ErrorException("Return of location is duplicated");

			checkToken(parametr[++i]);
			new_location.setReturn(parametr[i]);
		}

		else if (parametr[i] == "alias" && (i + 1) < parametr.size()) //  Crée un alias de chemin
		{
			if (path == "/cgi-bin")
				throw ErrorException("Parametr alias not allow for CGI");// interdite pour "/cgi-bin"
			if (!new_location.getAlias().empty())
			{
				throw ErrorException("Alias of location is duplicated");
				checkToken(parametr[++i]);
			}
			new_location.setAlias(parametr[i]);
		}

		else if (parametr[i] == "cgi_ext" && (i + 1) < parametr.size()) // Extensions CGI autorisées
		{
			std::vector<std::string> extension;
			while (++i < parametr.size())
			{
				if (parametr[i].find(";") != std::string::npos)
				{
					checkToken(parametr[i]);
					extension.push_back(parametr[i]);
					break ;
				}
				else
				{
					extension.push_back(parametr[i]);
					if (i + 1 >= parametr.size())
						throw ErrorException("Token is invalid");
				}
			}

			new_location.setCgiExtension(extension); //insertion extensions
		}

		else if (parametr[i] == "cgi_path" && (i + 1) < parametr.size()) //  Chemins des interpréteurs CGI
		{
			std::vector<std::string> path;

			while (++i < parametr.size())
			{
				if (parametr[i].find(";") != std::string::npos)
				{
					checkToken(parametr[i]);
					path.push_back(parametr[i]);
					break ;
				}
				else
				{
					path.push_back(parametr[i]);
					if (i + 1 >= parametr.size())
						throw ErrorException("Token is invalid");
				}
																	// Doit contenir /python ou /bash
				if (parametr[i].find("/python") == std::string::npos && parametr[i].find("/bash") == std::string::npos)
					throw ErrorException("cgi_path is invalid");
			}

			new_location.setCgiPath(path);
		}

		else if (parametr[i] == "client_max_body_size" && (i + 1) < parametr.size()) // Taille maximale du corps de requête
		{
			if (flag_max_size)
				throw ErrorException("Maxbody_size of location is duplicated");

			checkToken(parametr[++i]);
			new_location.setMaxBodySize(parametr[i]);
			flag_max_size = true;
		}

		else if (i < parametr.size())
			throw ErrorException("Parametr in a location is invalid: " + parametr[i]);
	}
																// Si pas de index spécifié → utilise celui du serveur
	if (new_location.getPath() != "/cgi-bin" && new_location.getIndexLocation().empty())
		new_location.setIndexLocation(this->_index);
	if (!flag_max_size)											// Si pas de client_max_body_size → utilise celui du serveur
		new_location.setMaxBodySize(this->_client_max_body_size);

	valid = isValidLocation(new_location);
/* Vérifie :
1 : CGI invalide
2 : Chemin invalide
3 : Fichier de redirection invalide
4 : Fichier alias invalide */
	if (valid == 1)
		throw ErrorException("Failed CGI validation");
	else if (valid == 2)
		throw ErrorException("Failed path in locaition validation");
	else if (valid == 3)
		throw ErrorException("Failed redirection file in locaition validation");
	else if (valid == 4)
		throw ErrorException("Failed alias file in locaition validation");

	this->_locations.push_back(new_location); 	// Ajout à la liste
}

void	ServerConfig::setFd(int fd)
{
	this->_listen_fd = fd;
}

/* Valide le format d'une adresse IPv4 (ex: 192.168.1.1) */
bool ServerConfig::isValidHost(std::string host) const
{
	struct sockaddr_in sockaddr;			// Structure qui stocke une adresse socket Internet Contient notamment sin_addr (l'adresse IP)
	int result = inet_pton(AF_INET, host.c_str(), &(sockaddr.sin_addr)); // Vérifie tous les critères d'une IPv4
	return (result == 1); 					//conversion réussit (adresse valide)
}

bool ServerConfig::isValidErrorPages()
{
	std::map<short, std::string>::const_iterator it;
	for (it = this->_error_pages.begin(); it != this->_error_pages.end(); it++)
	{
		if (it->first < 100 || it->first > 599) 	// Vérifie que le code error d' HTTP est valide (100-599)
			return (false);							// Vérifie que le fichier existe et est lisible ( 0 -- 4 )
		if (ConfigFile::checkFile(getRoot() + it->second, 0) < 0 || ConfigFile::checkFile(getRoot() + it->second, 4) < 0)
			return (false);
	}
	return (true);
}

/*
	valide une location (bloc de configuration) selon son type : /cgi-bin (CGI) ou
		location normale (fichiers statiques).

 * Retourne :
 * 0 : Validation réussie
 * 1 : Erreur de configuration CGI
 * 2 : Erreur de chemin de location
 * 3 : Erreur de fichier de redirection
 * 4 : Erreur de fichier alias
 * 5 : Erreur de fichier index
 */
int ServerConfig::isValidLocation(Location &location) const
{			/* Validation CGI */
	if (location.getPath() == "/cgi-bin")
	{													// Vérifie que les paramètres CGI essentiels sont présents
		if (location.getCgiPath().empty() || location.getCgiExtension().empty() || location.getIndexLocation().empty())
			return (1);

		if (ConfigFile::checkFile(location.getIndexLocation(), 4) < 0)	// Vérifie si le fichier index est accessible
		{
			std::string path = location.getRootLocation() + location.getPath() + "/" + location.getIndexLocation(); // Construit le chemin complet : root + path + index
			if (ConfigFile::getTypePath(path) != 1)						// Si le chemin n'est pas un fichier régulier (type != 1)
			{
				std::string root = getcwd(NULL, 0);						// Utilise le répertoire courant comme racine
				location.setRootLocation(root);
				path = root + location.getPath() + "/" + location.getIndexLocation();
			}
																		// Dernière vérification : fichier existe et est lisible
			if (path.empty() || ConfigFile::getTypePath(path) != 1 || ConfigFile::checkFile(path, 4) < 0)
				return (1);
		}
				// Le nombre d'extensions doit correspondre au nombre d'interpréteurs --> ex: cgi_ext .py .sh; = cgi_path /usr/bin/python3 /bin/bash;
		if (location.getCgiPath().size() != location.getCgiExtension().size())
			return (1);

		std::vector<std::string>::const_iterator it;					// Vérifie que tous les chemins d'interpréteurs existent
		for (it = location.getCgiPath().begin(); it != location.getCgiPath().end(); ++it)
		{
			if (ConfigFile::getTypePath(*it) < 0)
				return (1);
		}
						/* Association extensions → interpréteurs */
		std::vector<std::string>::const_iterator it_path;
		for (it = location.getCgiExtension().begin(); it != location.getCgiExtension().end(); ++it)
		{
			std::string tmp = *it;										// Accepte uniquement .py, .sh, *.py, *.sh
			if (tmp != ".py" && tmp != ".sh" && tmp != "*.py" && tmp != "*.sh")
				return (1);
																		// Associe chaque extension à son interpréteur
			for (it_path = location.getCgiPath().begin(); it_path != location.getCgiPath().end(); ++it_path)
			{
				std::string tmp_path = *it_path;
				if (tmp == ".py" || tmp == "*.py")
				{
					if (tmp_path.find("python") != std::string::npos)	// Si extension Python → cherche "python" dans le chemin
						location._ext_path.insert(std::make_pair(".py", tmp_path));
				}
				else if (tmp == ".sh" || tmp == "*.sh")
				{
					if (tmp_path.find("bash") != std::string::npos)		// Si extension Bash → cherche "bash" dans le chemin
						location._ext_path[".sh"] = tmp_path;
				}
			}
		}
		if (location.getCgiPath().size() != location.getExtensionPath().size()) // Vérifie que toutes les extensions ont trouvé leur interpréteur
			return (1);
	}

	else		/* Validation location normale */
	{
		if (location.getPath()[0] != '/')								// Le chemin doit commencer par '/'
			return (2);
		if (location.getRootLocation().empty())							 // Si pas de root spécifié, utilise celui du serveur
		{
			location.setRootLocation(this->_root);
		}																// Vérifie que le fichier index existe et est lisible
		if (ConfigFile::isFileExistAndReadable(location.getRootLocation() + location.getPath() + "/", location.getIndexLocation()))
			return (5);

		if (!location.getReturn().empty())
		{																// Si redirection configurée, vérifie le fichier cible
			if (ConfigFile::isFileExistAndReadable(location.getRootLocation(), location.getReturn()))
				return (3);
		}
		if (!location.getAlias().empty())								 // Si alias configuré, vérifie le fichier cible
		{
			if (ConfigFile::isFileExistAndReadable(location.getRootLocation(), location.getAlias()))
			 	return (4);
		}
	}
	return (0);
}

/*** Les fonctions GET ***/
const std::string &ServerConfig::getServerName(){
	return (this->_server_name);
}
const std::string &ServerConfig::getRoot(){
	return (this->_root);
}

const bool &ServerConfig::getAutoindex(){
	return (this->_autoindex);
}

const in_addr_t &ServerConfig::getHost(){
	return (this->_host);
}

const uint16_t &ServerConfig::getPort(){
	return (this->_port);
}

const size_t &ServerConfig::getClientMaxBodySize(){
	return (this->_client_max_body_size);
}

const std::vector<Location> &ServerConfig::getLocations(){
	return (this->_locations);
}

const std::map<short, std::string> &ServerConfig::getErrorPages(){
	return (this->_error_pages);
}

const std::string &ServerConfig::getIndex(){
	return (this->_index);
}

int	ServerConfig::getFd(){
	return (this->_listen_fd);
}

/* Récupère le chemin de la page d'erreur pour un code HTTP donné
		Utilisée lors de la génération de réponse(Phase RUNTIME), pas pendant le parsing
		--> afficher une page d'erreur personnalisée*/
const std::string &ServerConfig::getPathErrorPage(short key)
{
	std::map<short, std::string>::iterator it = this->_error_pages.find(key);
	if (it == this->_error_pages.end())
		throw ErrorException("Error_page does not exist");
	return (it->second);						// Retourne le chemin du fichier
}

/* Trouve une location par son chemin
 		Utilisée par le gestionnaire de serveur(Phase RUNTIME), pas pendant le parsing
		--> pour router vers la bonne location*/
const std::vector<Location>::iterator ServerConfig::getLocationKey(std::string key)
{
	std::vector<Location>::iterator it;
	for (it = this->_locations.begin(); it != this->_locations.end(); it++)
	{
		if (it->getPath() == key)
			return (it);						//  itérateur vers la location trouvée
	}
	throw ErrorException("Error: path to location not found");
}

/* vérifier si la FIN du paramètre est correcte et le suprime */
void ServerConfig::checkToken(std::string &parametr)
{
	size_t pos = parametr.rfind(';');
	if (pos != parametr.size() - 1)
		throw ErrorException("Token is invalid");
	parametr.erase(pos);
}

/* Vérifier s’il existe des doublons de “location” (même chemin) dans la configuration du serveur. */
bool ServerConfig::checkLocations() const
{
	if (this->_locations.size() < 2)					// moins de deux éléments -> impossible d’avoir un doublon
		return (false);
	std::vector<Location>::const_iterator it1;
	std::vector<Location>::const_iterator it2;
	for (it1 = this->_locations.begin(); it1 != this->_locations.end() - 1; it1++) {
		for (it2 = it1 + 1; it2 != this->_locations.end(); it2++) {
			if (it1->getPath() == it2->getPath())		// On compare leurs chemins avec it1->getPath() == it2->getPath().
				return (true);							// Si deux Location ont le même chemin, alors il y a un doublon → la fonction retourne true.
		}
	}
	return (false);
}

/* configuration et liaison du socket pour qu’il puisse écouter les connexions entrantes */
void	ServerConfig::setupServer(void)
{
	if ((_listen_fd = socket(AF_INET, SOCK_STREAM, 0) )  == -1 )
	{
		std::string error_msg = "socket() failed: ";
		error_msg += strerror(errno);
		throw ErrorException(error_msg);
	}

	int option_value = 1;
	setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));

	int flags = fcntl(_listen_fd, F_GETFL, 0);
	fcntl(_listen_fd, F_SETFL, flags | O_NONBLOCK);

	memset(&_server_address, 0, sizeof(_server_address));
	_server_address.sin_family = AF_INET;
	_server_address.sin_addr.s_addr = _host;
	_server_address.sin_port = htons(_port);

	if (bind(_listen_fd, (struct sockaddr *) &_server_address, sizeof(_server_address)) == -1)
	{
		close(_listen_fd);
		std::string error_msg = "bind() failed: ";
		error_msg += strerror(errno);
		throw ErrorException(error_msg);
	}

	if (listen(_listen_fd, 128) == -1)
	{
		close(_listen_fd);
		std::string error_msg = "listen() failed: ";
		error_msg += strerror(errno);
		throw ErrorException(error_msg);
	}
}
