/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jquinodo <jquinodo@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 14:14:02 by jquinodo          #+#    #+#             */
/*   Updated: 2025/11/19 10:46:53 by jquinodo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

ConfigParser::ConfigParser() : _nb_server(0) {}

ConfigParser::~ConfigParser() {}


/* impression des paramètres des serveurs à partir du fichier de configuration */
int ConfigParser::print() {

	std::cout << "------------- Config -------------" << std::endl;
	for (size_t i = 0; i < _servers.size(); i++)
	{
		std::cout << "Server #" << i + 1 << std::endl;
		std::cout << "Server name: " << _servers[i].getServerName() << std::endl;
		std::cout << "Host: " << _servers[i].getHost() << std::endl;
		std::cout << "Root: " << _servers[i].getRoot() << std::endl;
		std::cout << "Index: " << _servers[i].getIndex() << std::endl;
		std::cout << "Port: " << _servers[i].getPort() << std::endl;
		std::cout << "Max BSize: " << _servers[i].getClientMaxBodySize() << std::endl;
		std::cout << "Error pages: " << _servers[i].getErrorPages().size() << std::endl;
		std::map<short, std::string>::const_iterator it = _servers[i].getErrorPages().begin();
		while (it != _servers[i].getErrorPages().end())
		{
			std::cout << (*it).first << " - " << it->second << std::endl;
			++it;
		}
		std::cout << "Locations: " << _servers[i].getLocations().size() << std::endl;
		std::vector<Location>::const_iterator itl = _servers[i].getLocations().begin();
		while (itl != _servers[i].getLocations().end())
		{
			std::cout << "name location: " << itl->getPath() << std::endl;
			std::cout << "methods: " << itl->getPrintMethods() << std::endl;
			std::cout << "index: " << itl->getIndexLocation() << std::endl;
			if (itl->getCgiPath().empty())
			{
				std::cout << "root: " << itl->getRootLocation() << std::endl;
				if (!itl->getReturn().empty())
					std::cout << "return: " << itl->getReturn() << std::endl;
				if (!itl->getAlias().empty())
					std::cout << "alias: " << itl->getAlias() << std::endl;
			}
			else
			{
				std::cout << "cgi root: " << itl->getRootLocation() << std::endl;
				std::cout << "sgi_path: " << itl->getCgiPath().size() << std::endl;
				std::cout << "sgi_ext: " << itl->getCgiExtension().size() << std::endl;
			}
			++itl;
		}
		itl = _servers[i].getLocations().begin();

		std::cout << "-----------------------------" << std::endl;
	}
	return (0);
}

/* vérification et lecture du fichier de configuration,
		division des serveurs en chaînes et création d'un vecteur de serveurs
	--> lit un fichier .conf, le valide, le nettoie, le découpe en blocs serveurs,
			puis crée des objets ServerConfig pour chaque serveur trouvé */
int ConfigParser::createCluster(const std::string &config_file)
{
	std::string		content;
	ConfigFile		file(config_file);

	if (file.getTypePath(file.getPath()) != 1)			// Vérifie le type 1 ichier 2 dossier 3 autre
		throw ErrorException("File is invalid");
	if (file.checkFile(file.getPath(), 4) == -1)		// vérifie si le fichier existe et est accessible (4 = read)
		throw ErrorException("File is not accessible");

	content = file.readFile(config_file);				// lecture du fichier vers une string

	if (content.empty())
		throw ErrorException("File is empty");

	removeComments(content);							// supprimer les commentaires du caractère # à \n
	removeWhiteSpace(content);							// supprimer les WhiteSpace
	splitServers(content);								// fractionnement des serveurs sur des chaînes séparées dans le vecteur

	if (this->_server_config.size() != this->_nb_server)// comparaison serveurs dans vector / _nb_server
		throw ErrorException("Server parsing error: count mismatch");

	for (size_t i = 0; i < this->_nb_server; i++)		//  transforme chaque bloc serveur (string) en objet ServerConfig configuré.
	{
		ServerConfig server;
		createServer(this->_server_config[i], server);
		this->_servers.push_back(server);				// sauvegarde dans --> _servers
	}
	if (this->_nb_server > 1)
		checkServers();
	return (0);
}

/* supprimer les commentaires du caractère # à \n */
void ConfigParser::removeComments(std::string &content)
{
	size_t pos;

	pos = content.find('#');
	while (pos != std::string::npos)
	{
		size_t pos_end;
		pos_end = content.find('\n', pos);
		content.erase(pos, pos_end - pos);
		pos = content.find('#');
	}
}

/* suppression des whitespaces au début, à la fin et dans le contenu s'il y en a plusieurs */
void ConfigParser::removeWhiteSpace(std::string &content)
{
	size_t	i = 0;

	while (content[i] && isspace(content[i]))
		i++;
	content = content.substr(i);
	i = content.length() - 1;
	while (i > 0 && isspace(content[i]))
		i--;
	content = content.substr(0, i + 1);
}

/* fractionnement des serveurs sur des chaînes séparées dans le vecteur */
void ConfigParser::splitServers(std::string &content)
{
	size_t start = 0;
	size_t end = 1;

	if (content.find("server", 0) == std::string::npos)
		throw ErrorException("Server did not find");
	while (start != end && start < content.length())
	{
		start = findStartServer(start, content);
		end = findEndServer(start, content);
		if (start == end)
			throw ErrorException("Unmatched braces in server block");
		this->_server_config.push_back(content.substr(start, end - start + 1));
		this->_nb_server++;
		start = end + 1;
	}
}

/* trouver un début de serveur et renvoyer l'index de --> {  début du serveur */
size_t ConfigParser::findStartServer (size_t start, std::string &content)
{
	size_t i;

	for (i = start; content[i]; i++)
	{
		if (content[i] == 's')
			break ;
		if (!isspace(content[i]))
			throw  ErrorException("Wrong character out of server scope{}");
	}
	if (!content[i])
		return (start);
	if (content.compare(i, 6, "server") != 0)
		throw ErrorException("Wrong character out of server scope{}");
	i += 6;
	while (content[i] && isspace(content[i]))
		i++;
	if (content[i] == '{')
		return (i);
	else
		throw  ErrorException("Wrong character out of server scope{}");
}

/* trouver la fin du serveur et renvoyer l'index de } fin du serveur */
size_t ConfigParser::findEndServer (size_t start, std::string &content)
{
	size_t	i;
	size_t	scope;

	scope = 0;
	for (i = start + 1; content[i]; i++)
	{
		if (content[i] == '{')
			scope++;
		if (content[i] == '}')
		{
			if (!scope)
				return (i);
			scope--;
		}
	}
	return (start);
}

/* division de la ligne par séparateur pour *creatServer* */
std::vector<std::string> splitParametrs(std::string line, std::string sep)
{
	std::vector<std::string>	str;
	std::string::size_type		start, end;

	start = end = 0;
	while (1)
	{
		end = line.find_first_of(sep, start);
		if (end == std::string::npos)
			break;
		std::string tmp = line.substr(start, end - start);
		str.push_back(tmp);
		start = line.find_first_not_of(sep, end);
		if (start == std::string::npos)
			break;
	}
	return (str);
}

/*** La méthode parse une chaîne de configuration pour extraire des directives comme listen, host, root, location, etc.
		--> valide la syntaxe et l'absence de duplications.
		--> remplit un objet ServerConfig avec les valeurs extraites.
		--> gère les sections location comme des blocs distincts.
		--> applique des valeurs par défaut si nécessaire et effectue des validations finales.*/
void ConfigParser::createServer(std::string &config, ServerConfig &server)
{
	std::vector<std::string>	parametrs;
	std::vector<std::string>	error_codes;
	int		flag_loc = 1;						// : Indique si les directives doivent être lues avant une section location (1 = avant, 0 = après).
	bool	flag_autoindex = false;
	bool	flag_max_size = false;

	parametrs = splitParametrs(config += ' ', std::string(" \n\t"));	// Split en tocken dans parametrs , un espace est ajouté à config pour s'assurer que le dernier token est bien traité.
	if (parametrs.size() < 3)
		throw  ErrorException("Failed server validation");

	for (size_t i = 0; i < parametrs.size(); i++)						// parcourt chaque token de parametrs pour identifier et traiter les directives.
	{
		if (parametrs[i] == "listen" && (i + 1) < parametrs.size() && flag_loc)	// Vérifie listen est suivi d'une valeur (le port) et doit apparaître avant une section location
		{
			if (server.getPort())												// Si un port est déjà défini  exception pour éviter les duplications.
				throw  ErrorException("Port is duplicated");
			server.setPort(parametrs[++i]);										// Assigne le port à l'objet server avec la valeur suivante ++i
		}
		else if (parametrs[i] == "location" && (i + 1) < parametrs.size())
		{
			std::string	path;
			i++;
			if (parametrs[i] == "{" || parametrs[i] == "}")						// Vérifie que le token suivant n'est pas { ou } (car ils délimitent un bloc)
				throw  ErrorException("Wrong character in server scope { } ");

			path = parametrs[i];												// Récpération du path

			std::vector<std::string> codes;
			if (parametrs[++i] != "{")
			{											// Le suivant doit'être {
				throw  ErrorException("Wrong character in server scope{}");
			}
			++i;														// saute l'accolade ouvrante pour ne garder que les directives internes
			while (i < parametrs.size() && parametrs[i] != "}")					// Récupère tous les tokens jusqu'à } dans codes.
				codes.push_back(parametrs[i++]);

			if (i >= parametrs.size() || parametrs[i] != "}")						// vérifie qu'on a bien trouvé la fermeture
				throw  ErrorException("Wrong character in server scope{}");
			server.setLocation(path, codes);									// Appelle server.setLocation(path, codes) pour configurer la location.
			flag_loc = 0;														// flag_loc à 0 pour indiquer que les directives suivantes ne peuvent plus apparaître au niveau du serveur.
		}
		else if (parametrs[i] == "host" && (i + 1) < parametrs.size() && flag_loc) // similaires à listen
		{
			if (server.getHost())
				throw  ErrorException("Host is duplicated");
			server.setHost(parametrs[++i]);
		}
		else if (parametrs[i] == "root" && (i + 1) < parametrs.size() && flag_loc)	// similaires à listen
		{
			if (!server.getRoot().empty())
				throw  ErrorException("Root is duplicated");
			server.setRoot(parametrs[++i]);
		}
		else if (parametrs[i] == "error_page" && (i + 1) < parametrs.size() && flag_loc)
		{
			while (++i < parametrs.size())													// Récupère une liste de codes d'erreur jusqu'à rencontrer un point-virgule (;).
			{																				// Stocke les codes dans error_codes pour une validation ultérieure.
				error_codes.push_back(parametrs[i]);
				if (parametrs[i].find(';') != std::string::npos)
					break ;
				if (i + 1 >= parametrs.size())
					throw ErrorException("Wrong character out of server scope{}");
			}
		}
		else if (parametrs[i] == "client_max_body_size" && (i + 1) < parametrs.size() && flag_loc) // similaires à listen
		{
			if (flag_max_size)
				throw  ErrorException("Client_max_body_size is duplicated");
			server.setClientMaxBodySize(parametrs[++i]);
			flag_max_size = true;
		}
		else if (parametrs[i] == "server_name" && (i + 1) < parametrs.size() && flag_loc) // similaires à listen
		{
			if (!server.getServerName().empty())
				throw  ErrorException("Server_name is duplicated");
			server.setServerName(parametrs[++i]);
		}
		else if (parametrs[i] == "index" && (i + 1) < parametrs.size() && flag_loc)		// similaires à listen
		{
			if (!server.getIndex().empty())
				throw  ErrorException("Index is duplicated");
			server.setIndex(parametrs[++i]);
		}
		else if (parametrs[i] == "autoindex" && (i + 1) < parametrs.size() && flag_loc)	// similaires à listen
		{
			if (flag_autoindex)
				throw ErrorException("Autoindex of server is duplicated");
			server.setAutoindex(parametrs[++i]);
			flag_autoindex = true;
		}
		else if (parametrs[i] != "}" && parametrs[i] != "{")					// Si un token n'est ni une directive reconnue, ni { ou }, une exception est levée.
		{
			if (!flag_loc)														// Si flag_loc est à 0, cela signifie qu'un paramètre apparaît après une section location, ce qui est interdit.
				throw  ErrorException("Parametrs after location");
			else
				throw  ErrorException("Unsupported directive");
		}
	}
/*** Définition des valeurs par défaut ***/
	if (server.getRoot().empty())
		server.setRoot("/;");
	if (server.getHost() == 0)
		server.setHost("localhost;");
	if (server.getIndex().empty())
		server.setIndex("index.html;");

/*** Validations finales ***
	--> Vérifie que le fichier index est accessible.
	-->	Vérifie qu'il n'y a pas de locations dupliquées.
	--> Vérifie qu'un port est défini.
	--> Configure les pages d'erreur avec error_codes et valide leur format. */
	if (ConfigFile::isFileExistAndReadable(server.getRoot(), server.getIndex()))
		throw ErrorException("Index from config file not found or unreadable");
	if (server.checkLocations())
		throw ErrorException("Locaition is duplicated");
	if (!server.getPort())
		throw ErrorException("Port not found");
	server.setErrorPages(error_codes);
	if (!server.isValidErrorPages())
		throw ErrorException("Incorrect path for error page or number of error");
}

/* comparaison de chaînes à partir de la position */
int	ConfigParser::stringCompare(std::string str1, std::string str2, size_t pos)
{
	size_t	i;

	i = 0;
	while (pos < str1.length() && i < str2.length() && str1[pos] == str2[i])
	{
		pos++;
		i++;
	}
	if (i == str2.length() && pos <= str1.length() && (str1.length() == pos || isspace(str1[pos])))
		return (0);
	return (1);
}

/* vérification des paramètres répétés et obligatoires
	assure qu'aucun serveur dans _servers n'a la même combinaison de port, host et server_name. (2 identiques = ok)*/
void ConfigParser::checkServers()
{
	std::vector<ServerConfig>::iterator it1;
	std::vector<ServerConfig>::iterator it2;

	for (it1 = this->_servers.begin(); it1 != this->_servers.end() - 1; it1++)
	{
		for (it2 = it1 + 1; it2 != this->_servers.end(); it2++)
		{
			if (it1->getPort() == it2->getPort() && it1->getHost() == it2->getHost() && it1->getServerName() == it2->getServerName())
				throw ErrorException("Failed server validation");
		}
	}
}

std::vector<ServerConfig>	ConfigParser::getServers() {
	return (this->_servers);
}
