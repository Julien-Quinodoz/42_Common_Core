#pragma once
#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
// # include <fstream>
# include <fcntl.h>
# include <cstring>
# include <cerrno>
# include <string> 
# include <unistd.h>
# include <dirent.h>
# include <sstream>
// # include <bits/stdc++.h>
# include <cstdlib>
# include <fstream>
# include <sstream>
# include <cctype>
# include <ctime>
# include <cstdarg>

/* STL Containers */
# include <map>
# include <set>
# include <vector>
# include <algorithm>
# include <iterator>
# include <list>

/* System */
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <unistd.h>
// # include <machine/types.h>
# include <signal.h>

/* Network */
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/select.h>
# include <arpa/inet.h>

# include "ConfigParser.hpp"
# include "ConfigFile.hpp"
# include "ServerConfig.hpp"
# include "Location.hpp"
# include "HttpRequest.hpp"
# include "CgiHandler.hpp"
# include "Mime.hpp"


#define CONNECTION_TIMEOUT 60
#ifdef TESTER
    #define MESSAGE_BUFFER 40000 
#else
    #define MESSAGE_BUFFER 40000
#endif

#define MAX_URI_LENGTH 4096
#define MAX_CONTENT_LENGTH 30000000

/* conversion très pratique pour construire des strings avec des nombres */
template <typename T>
std::string toString(const T val)
{
    std::stringstream stream;
    stream << val;
    return stream.str();
}

/* Utils.c */

std::string statusCodeString(short); // transforme le code numérique en texte descriptif : 200 -> "ok"
std::string getErrorPage(short); // construit le chemin d'un fichier d'erreur : 404 -> "error_pages/404.html"   
int buildHtmlIndex(std::string &, std::vector<uint8_t> &, size_t &); // construit l'index HTML d'un dossier : "index.html"
int ft_stoi(std::string str); // convertit une chaîne de caractères en entier : "123" -> 123
unsigned int fromHexToDec(const std::string& nb); // convertit une chaîne de caractères en nombre hexadécimal : "1A" -> 26


#endif

