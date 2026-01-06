/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jquinodo <jquinodo@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/03 13:43:17 by jquinodo          #+#    #+#             */
/*   Updated: 2025/11/19 10:45:54 by jquinodo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Mime Response::mime;

Response::Response()
{
	_target_file = "";
	_body.clear();
	_body_length = 0;
	response_content = "";
	_response_body = "";
	_location = "";
	_code = 0;
	_cgi = 0;
	_cgi_response_length = 0;
	_auto_index = 0;
}

Response::~Response() {}

Response::Response(HttpRequest &req) : request(req)
{
	_target_file = "";
	_body.clear();
	_body_length = 0;
	response_content = "";
	_response_body = "";
	_location = "";
	_code = 0;
	_cgi = 0;
	_cgi_response_length = 0;
	_auto_index = 0;
}

/* Construit le type de contenu de la réponse 
  	Extrait extension et trouve le type MIME correspondant
	Si pas d'extension, type MIME par défaut */
void	Response::contentType()
{
	response_content.append("Content-Type: ");
	if(_target_file.rfind(".", std::string::npos) != std::string::npos && _code == 200)
		response_content.append(mime.getMimeType(_target_file.substr(_target_file.rfind(".", std::string::npos))) );
	else
		response_content.append(mime.getMimeType("default"));
	response_content.append("\r\n");
}

/* Construit la longueur du contenu  */
void	Response::contentLength()
{
	std::stringstream ss;
	ss << _response_body.length();
	response_content.append("Content-Length: ");
	response_content.append(ss.str());
	response_content.append("\r\n");
}

/* Construit le header Connection */
void	Response::connection()
{
	std::string conn = request.getHeader("connection");
	if (conn == "close")
		response_content.append("Connection: close\r\n");
	else
		response_content.append("Connection: keep-alive\r\n");
}

void	Response::server()
{
		response_content.append("Server: LETSGO\r\n");
}

void    Response::location()
{
	if (_location.length())
		response_content.append("Location: "+ _location +"\r\n");
}

void	Response::date()
{
	char date[1000];
	time_t now = time(0);
	struct tm *tm = gmtime(&now);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", tm);
	response_content.append("Date: ");
	response_content.append(date);
	response_content.append("\r\n");
}


/* Construit les headers de la réponse 
	Content-Type: type/sous-type
   Content-Length: longueur
   Connection: keep-alive ou close
   Server: LETSGO
   Location: redirection
   Allow: GET, POST, DELETE
   Date: date
*/
void	Response::setHeaders()
{
	contentType();
	contentLength();
	connection();
	server();
	location();

	/* Ajoute le header Allow si le code d'état est 405 (Method Not Allowed) */
	if (_code == 405)
	{
		/* Trouve la location la plus correspondante pour le chemin de la requête
		   et ajoute le header Allow en fonction des méthodes autorisées */
		const std::vector<Location> &locs = _server.getLocations();
		const Location *best = NULL;
		size_t biggest_match = 0;
		const std::string &path = request.getPath();
		for (std::vector<Location>::const_iterator it = locs.begin(); it != locs.end(); ++it)
		{
			const std::string &lp = it->getPath();
			if (path.find(lp) == 0)
			{
				if (lp == "/" || path.length() == lp.length() || path[lp.length()] == '/')
				{
					if (lp.length() > biggest_match)
					{
						biggest_match = lp.length();
						best = &(*it);
					}
				}
			}
		}
		if (best)  // Si on a trouvé une location correspondante ; Récupère les méthodes autorisées de cette location
		{
			const std::vector<short> methods = best->getMethods();
			std::string allow = "Allow: ";
			bool first = true;
			// Order: GET, POST, DELETE
			if (methods.size() >= 3)
			{
				if (methods[0]) { allow += "GET"; first = false; }
				if (methods[1]) { if (!first) allow += ", "; allow += "POST"; first = false; }
				if (methods[2]) { if (!first) allow += ", "; allow += "DELETE"; }
			}
			allow += "\r\n";
			response_content.append(allow);
		}
	}
	date();
	response_content.append("\r\n");
}

static bool fileExists (const std::string& f)
{
	std::ifstream file(f.c_str());
	return (file.good());
}

static bool isDirectory(std::string path)
{
	struct stat file_stat;
	if (stat(path.c_str(), &file_stat) != 0)
		return (false);

	return (S_ISDIR(file_stat.st_mode));
}

static bool isAllowedMethod(HttpMethod &method, Location &location, short &code)
{
	std::vector<short> methods = location.getMethods();
	if ((method == GET && !methods[0]) || (method == POST && !methods[1]) ||
		(method == DELETE && !methods[2]))
	{
		code = 405;
		return (1);
	}
	return (0);
}

/* Vérifie si la location a une redirection configurée */
static bool	checkReturn(Location &loc, short &code, std::string &location)
{
	if (!loc.getReturn().empty())
	{
		code = 301;
		location = loc.getReturn();
		if (location[0] != '/')
			location.insert(location.begin(), '/');
		return (1);
	}
	return (0);
}

/* Combine les chemins de la requête */
static std::string combinePaths(std::string p1, std::string p2, std::string p3)
{
	std::string res;
	int	len1;
	int	len2;

	len1 = p1.length();
	len2 = p2.length();
	if (p1[len1 - 1] == '/' && (!p2.empty() && p2[0] == '/') )	// Pour éviter une double barre
		p2.erase(0, 1);
	if (p1[len1 - 1] != '/' && (!p2.empty() && p2[0] != '/'))	// Pour éviter une fusion sans séparateur
		p1.insert(p1.end(), '/');
	if (p2[len2 - 1] == '/' && (!p3.empty() && p3[0] == '/') )	// Même logique entre p2 et p3
		p3.erase(0, 1);
	if (p2[len2 - 1] != '/' && (!p3.empty() && p3[0] != '/'))
		p2.insert(p2.end(), '/');
	res = p1 + p2 + p3;
	return (res);

}

/* Remplace le chemin par l'alias */
static void replaceAlias(Location &location, HttpRequest &request, std::string &target_file) {
	target_file = combinePaths(location.getAlias(), request.getPath().substr(location.getPath().length()), "");
}

/* Ajoute le chemin racine */
static void appendRoot(Location &location, HttpRequest &request, std::string &target_file) {
	target_file = combinePaths(location.getRootLocation(), request.getPath(), "");
}

/* Prépare et lance l'exécution d'un script CGI */
int Response::handleCgiTemp(std::string &location_key)
{
	std::string path;
	path = _target_file;					// Récupère le chemin du fichier script CGI
	cgi_obj.clear();
	cgi_obj.setCgiPath(path);				// Définit le chemin du script CGI à exécuter
	_cgi = 1;								// Flag indiquant qu'une requête CGI est en cours
	if (pipe(_cgi_fd) < 0)					// Crée un pipe pour la communication entre processus
	{										// _cgi_fd[0] = lecture, _cgi_fd[1] = écriture
		_code = 500;
		return (1);
	}
	cgi_obj.initEnvCgi(request, _server.getLocationKey(location_key)); // Initialise les variables d'environnement CGI (REQUEST_METHOD, QUERY_STRING, etc.)
	cgi_obj.execute(this->_code);			// Execute le script CGI et stocke le code de statut dans _code
	return (0);
}

/* Vérifie si c'est un fichier pour CGI
--> (l'extension est prise en charge, le fichier existe et est exécutable) et exécuter le CGI
Étape	Vérification						Code erreur
1		Nettoyage du chemin
2		Ajout de l'index si nécessaire
3		Présence d'une extension		
4		Extension supportée (.py ou .sh)	
5		Fichier existe						
6		Fichier lisible ET exécutable		
7		Méthode HTTP autorisée				
8		Création du pipe					
9		Exécution du CGI-						*/
int	Response::handleCgi(std::string &location_key)
{
	std::string path;
	std::string exten;
	size_t		pos;

	path = this->request.getPath();
	if (!path.empty() && path[0] == '/')		// Supprime le '/' initial si présent
		path.erase(0, 1);						// "/cgi-bin/script.py" → "cgi-bin/script.py"
	if (path == "cgi-bin")						// Si le chemin est exactement "cgi-bin", ajoute le fichier index
		path += "/" + _server.getLocationKey(location_key)->getIndexLocation();
	else if (path == "cgi-bin/")				// Si le chemin est "cgi-bin/", ajoute juste le fichier index
		path.append(_server.getLocationKey(location_key)->getIndexLocation()); // Ex: "cgi-bin/" → "cgi-bin/index.py"

	pos = path.find(".");						// Cherche le point de l'extension error 501 si pas trouvé
	if (pos == std::string::npos)
	{
		_code = 501;
		return (1);
	}
	exten = path.substr(pos);					// Extrait l'extension: ".py" ou ".sh"
	if (exten != ".py" && exten != ".sh")
	{
		_code = 501;
		return (1);
	}
	if (ConfigFile::getTypePath(path) != 1)
	{
		_code = 404;
		return (1);
	}
	if (ConfigFile::checkFile(path, 1) == -1 || ConfigFile::checkFile(path, 3) == -1) // 1 lecture (read) 3 permission d'Exécution
	{
		_code = 403;
		return (1);
	}
	if (isAllowedMethod(request.getMethod(), *_server.getLocationKey(location_key), _code)) 	// Vérifie si la méthode (GET, POST, etc.) est autorisée
		return (1);
	cgi_obj.clear();						// Nettoie l'objet CGI
	cgi_obj.setCgiPath(path);				// Définit le script à exécuter
	_cgi = 1;								// Active le flag CGI
	if (pipe(_cgi_fd) < 0)					// CRÉATION DU PIPE POUR LA COMMUNICATION
	{
		_code = 500;
		return (1);
	}
	cgi_obj.initEnv(request, _server.getLocationKey(location_key)); // INITIALISATION DES VARIABLES D'ENVIRONNEMENT CGI
	cgi_obj.execute(this->_code);			// EXÉCUTION DU SCRIPT CGI
	return (0);
}

/*
Compare l'URI avec les emplacements du fichier de configuration et tente de trouver la meilleure correspondance.
Si une correspondance est trouvée, location_key est défini sur cet emplacement ; sinon, location_key est une chaîne vide.	*/
static void	getLocationMatch(std::string &path, std::vector<Location> locations, std::string &location_key)
{
	size_t biggest_match = 0;									// Garde la longueur de la meilleure correspondance

	for(std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) 	// Parcourt toutes les locations définies dans config
	{
		if(path.find(it->getPath()) == 0) {
			if( it->getPath() == "/" ||						// Vérifie si le chemin commence par le chemin de la location
			 path.length() == it->getPath().length() ||		// Match exact
			 path[it->getPath().length()] == '/')			// Prochain caractère est '/'
			{
				if(it->getPath().length() > biggest_match){ // Si cette location est plus spécifique que la précédente
					biggest_match = it->getPath().length();	// Met à jour la longueur
					location_key = it->getPath();			// Sauvegarde la clé de location
				}
			}
		}
	}
}
/*
	Cherche quelle location correspond à l'URL
	Vérifie les permissions (méthode, taille body)
	Gère les redirections configurées
	Détecte si c'est du CGI
	Construit le chemin complet du fichier
	Gère les répertoires (index, autoindex, redirections)
	Retourne 0 si tout OK, 1 si erreur/redirection */
int	Response::handleTarget()
{
	std::string location_key;
	getLocationMatch(request.getPath(), _server.getLocations(), location_key);
	if (location_key.length() > 0)
	{
		Location target_location = *_server.getLocationKey(location_key);

	if (isAllowedMethod(request.getMethod(), target_location, _code))
	{
		return (1);
	}
		if (request.getBody().length() > target_location.getMaxBodySize())
		{
			_code = 413;
			return (1);
		}
		if (checkReturn(target_location, _code, _location))
			return (1);

		if (target_location.getPath().find("cgi-bin") != std::string::npos){
			return (handleCgi(location_key));
		}

		if (!target_location.getAlias().empty()){
			replaceAlias(target_location, request, _target_file);
		}
		else
			appendRoot(target_location, request, _target_file);

		if (!target_location.getCgiExtension().empty())
		{
			if (_target_file.rfind(target_location.getCgiExtension()[0]) != std::string::npos) {
				return (handleCgiTemp(location_key));
			}
		}

		if (isDirectory(_target_file))
		{
			if (_target_file[_target_file.length() - 1] != '/')
			{
				_code = 301;
				_location = request.getPath() + "/";
				return (1);
			}
			if (!target_location.getIndexLocation().empty())
					_target_file += target_location.getIndexLocation();
			else
				_target_file += _server.getIndex();
			if (!fileExists(_target_file))
			{
				if (target_location.getAutoindex())
				{
					_target_file.erase(_target_file.find_last_of('/') + 1);
					_auto_index = true;
					return (0);
				}
				else
				{
					_code = 403;
					return (1);
				}
			}
			if (isDirectory(_target_file))
			{
				_code = 301;
				if (!target_location.getIndexLocation().empty())
					_location = combinePaths(request.getPath(), target_location.getIndexLocation(), "");
				else
					_location = combinePaths(request.getPath(), _server.getIndex(), "");
				if (_location[_location.length() - 1] != '/')
					_location.insert(_location.end(), '/');
				return (1);
			}
		}
	}
	else
	{
		_target_file = combinePaths(_server.getRoot(), request.getPath(), "");
		if (isDirectory(_target_file))
		{
			if (_target_file[_target_file.length() - 1] != '/')
			{
				_code = 301;
				_location = request.getPath() + "/";
				return (1);
			}
			_target_file += _server.getIndex();
			if (!fileExists(_target_file))
			{
				_code = 403;
				return (1);
			}
			if (isDirectory(_target_file))
			{
				_code = 301;
				_location = combinePaths(request.getPath(), _server.getIndex(), "");
				if(_location[_location.length() - 1] != '/')
				{
					_location.insert(_location.end(), '/');
					return (1);
				}
			}
		}
	}
	return (0);
}

bool Response::reqError()
{
	if(request.errorCode())
	{
		_code = request.errorCode();
		return (1);
	}
	return (0);
}

void Response::setServerDefaultErrorPages(){
	_response_body = getErrorPage(_code);
}

/* Construit le corps de réponse pour les pages d'erreur */
void Response::buildErrorBody()
{
	short original_code = _code;

	if (!_server.getErrorPages().count(original_code) ||
		_server.getErrorPages().at(original_code).empty() ||
		request.getMethod() == DELETE ||
		request.getMethod() == POST)
	{
		/* Retour à la page d'erreur par défaut */
		setServerDefaultErrorPages();
		_code = original_code;
		return;
	}

	std::string error_page_path = _server.getErrorPages().at(original_code);
	_target_file = _server.getRoot() + error_page_path;

	/* Essayer de lire la page d'erreur personnalisée */
	if (readFile() == 0)
	{
		/* Succès: garder le code original, _response_body déjà rempli */
		_code = original_code;
		return;
	}
	/* Lecture échouée: retour à la page d'erreur par défaut avec le code original */
	_code = original_code;
	_response_body = getErrorPage(_code);
}

/* Génére de réponse HTTP
 Elle coordonne toutes les étapes de construction d'une réponse HTTP complète */
void	 Response::buildResponse()
{
	if (reqError() || buildBody())
			buildErrorBody();
	if (_cgi)
		return ;
	else if (_auto_index)
	{
		if (buildHtmlIndex(_target_file, _body, _body_length))
		{
			_code = 500;
			buildErrorBody();
		}
		else
			_code = 200;
		_response_body.insert(_response_body.begin(), _body.begin(), _body.end());
	}
	setStatusLine();
	setHeaders();
	if (request.getMethod() == GET || _code != 200)
			response_content.append(_response_body);
}
/*
┌─────────────────────────────────────┐
│ buildResponse()                     │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│ 1. Vérification erreurs             │
│    reqError() || buildBody()        │
└─────────────────────────────────────┘
              ↓
         ┌────┴────┐
      Erreur ?    Succès
         ↓          ↓
┌──────────────┐   │
│buildErrorBody│   │
└──────────────┘   │
         ↓          ↓
         └────┬─────┘
              ↓
┌─────────────────────────────────────┐
│ 2. Type de réponse ?                │
└─────────────────────────────────────┘
       ↓         ↓         ↓
     CGI    Auto-index  Normal
       ↓         ↓         ↓
    return  buildHtml...  [rien]
              ↓
       ┌──────┴──────┐
    Succès        Échec
       ↓             ↓
   code=200    code=500
       ↓        buildError
       └──────┬──────┘
              ↓
┌─────────────────────────────────────┐
│ 3. Assemblage final                 │
│    setStatusLine()                  │
│    setHeaders()                     │
│    append body si nécessaire        │
└─────────────────────────────────────┘
              ↓
	[Réponse HTTP complète] */

void	Response::setErrorResponse(short code)
{
	response_content = "";
	_code = code;
	_response_body = "";
	buildErrorBody();
	setStatusLine();
	setHeaders();
	response_content.append(_response_body);
}

/* Renvoie la réponse entière (en-têtes + corps) */
std::string Response::getRes()	{
	return (response_content);
}

/* taille (en-têtes + corps) */
size_t Response::getLen() const	{
	return (response_content.length());
}

/* Construit la ligne d'état en fonction du code d'état. */
void	Response::setStatusLine()
{
	response_content.append("HTTP/1.1 " + toString(_code) + " ");
	response_content.append(statusCodeString(_code));
	response_content.append("\r\n");
}

/* Construit le corps de la réponse */
int	Response::buildBody()
{
	if (request.getBody().length() > _server.getClientMaxBodySize())
	{
		_code = 413;
		return (1);
	}
	if ( handleTarget() )
		return (1);
	if (_cgi || _auto_index)
		return (0);
	if (_code)
		return (0);
	if (request.getMethod() == GET)
	{
		if (readFile())
			return (1);
	}
	else if (request.getMethod() == POST)
	{
		bool existed = fileExists(_target_file);
		std::ofstream file(_target_file.c_str(), std::ios::binary);
		if (file.fail())
		{
			_code = 403;
			return (1);
		}

		if (request.getMultiformFlag())
		{
			std::string body = request.getBody();
			body = removeBoundary(body, request.getBoundary());
			file.write(body.c_str(), body.length());
		}
		else
		{
			file.write(request.getBody().c_str(), request.getBody().length());
		}
		/* Définit les codes d'état appropriés pour POST */
		if (existed)
		{
			_code = 204; /* Pas de contenu lors de la surécriture */
		}
		else
		{
			_code = 201; /* Créé pour une nouvelle ressource */
			_location = request.getPath(); /* Fournit l'URI de la ressource */
		}
	}
	else if (request.getMethod() == DELETE)
	{
		if (!fileExists(_target_file))
		{
			_code = 404;
			return (1);
		}
		if (remove( _target_file.c_str() ) != 0 )
		{
			_code = 403;
			return (1);
		}
	}
	/* Si aucun code d'état spécifique n'a été défini par les gestionnaires ci-dessus,
	   définit le code d'état par défaut à 200 OK */
	if (_code == 0)
		_code = 200;
	return (0);
}

/* Lit le fichier et le stocke dans _response_body */
int Response::readFile()
{
	std::ifstream file(_target_file.c_str());

	if (file.fail())
	{
		_code = 404;
		return (1);
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	_response_body = ss.str();
	return (0);
}

void	Response::setServer(ServerConfig &server)	{
	_server = server;
}

void	Response::setRequest(HttpRequest &req)	{
	request = req;
}

/* Supprime le début de la réponse */
void	Response::cutRes(size_t i)	{
	response_content = response_content.substr(i);
}

void	Response::clear()
{
	_target_file.clear();
	_body.clear();
	_body_length = 0;
	response_content.clear();
	_response_body.clear();
	_location.clear();
	_code = 0;
	_cgi = 0;
	_cgi_response_length = 0;
	_auto_index = 0;
}

int	Response::getCode() const	{
	return (_code);
}

int	Response::getCgiState()	{
	return (_cgi);
}

/* Supprime le boundary (Séparateur de contenu --boundary--) de la réponse */
std::string Response::removeBoundary(std::string &body, std::string &boundary)
{
	std::string buffer;
	std::string new_body;
	std::string filename;
	bool is_boundary = false;
	bool is_content = false;

	if (body.find("--" + boundary) != std::string::npos && body.find("--" + boundary + "--") != std::string::npos)
	{
		for (size_t i = 0; i < body.size(); i++)
		{
			buffer.clear();
			while(body[i] != '\n')
			{
				 buffer += body[i];
				i++;
			}
			if (!buffer.compare(("--" + boundary + "--\r")))
			{
				is_content = true;
				is_boundary = false;
			}
			if (!buffer.compare(("--" + boundary + "\r")))
			{
				is_boundary = true;
			}
			if (is_boundary)
			{
				if (!buffer.compare(0, 31, "Content-Disposition: form-data;"))
				{
					size_t start = buffer.find("filename=\"");
					if (start != std::string::npos)
					{
						size_t end = buffer.find("\"", start + 10);
						if (end != std::string::npos)
						filename = buffer.substr(start + 10, end);
					}
				}
				else if (!buffer.compare(0, 1, "\r") && !filename.empty())
				{
					is_boundary = false;
					is_content = true;
				}
			}
			else if (is_content)
			{
				if (!buffer.compare(("--" + boundary + "\r")))
				{
					is_boundary = true;
				}
				else if (!buffer.compare(("--" + boundary + "--\r")))
				{
				new_body.erase(new_body.end() - 1);
					break ;
				}
				else
					new_body += (buffer + "\n");
			}
		}
	}

	body.clear();
	return (new_body);
}

void	Response::setCgiState(int state)	{
	_cgi = state;
}
