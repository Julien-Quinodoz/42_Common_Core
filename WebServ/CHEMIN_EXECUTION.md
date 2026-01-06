# Chemin d'exécution du projet WebServ

## Vue d'ensemble du flux

```
1. DÉMARRAGE (main.cpp)
   ↓
2. CONFIGURATION (ServerManager::loadConfig)
   ↓
3. BOUCLE ÉVÉNEMENTS (ServerManager::run → processEvents)
   ↓
4. ACCEPTATION CONNEXION (handleServerSocket → acceptNewConnection)
   ↓
5. LECTURE REQUÊTE (handleClientRead)
   ↓
6. PARSING HTTP (HttpRequest::feed)
   ↓
7. CONSTRUCTION RÉPONSE (Response::buildResponse)
   ↓
8. ENVOI RÉPONSE (handleClientWrite)
   ↓
9. FERMETURE (closeClient)
```

---

## 1. POINT D'ENTRÉE

**Fichier:** `network_layer/src/main.cpp`

```cpp
main()
  ├─> ServerManager::loadConfig(config_file)  // Charge la configuration
  └─> ServerManager::run()                     // Démarre la boucle événementielle
```

**Responsabilités:**
- Initialisation du serveur
- Gestion des signaux (SIGINT, SIGTERM)
- Chargement du fichier de configuration

---

## 2. CONFIGURATION ET INITIALISATION

**Fichier:** `network_layer/src/ServerManager.cpp`

```cpp
ServerManager::loadConfig()
  ├─> ConfigParser::createCluster()            // Parse le fichier .conf
  ├─> ServerConfig::setupServer()              // Crée les sockets d'écoute
  └─> initSets()                               // Ajoute les sockets au fd_set

ServerManager::run()
  ├─> initSets()                               // Initialise les sets pour select()
  └─> while (_running) {
        processEvents()                        // Boucle principale
        checkTimeouts()                        // Vérifie les timeouts
      }
```

**Fichiers associés:**
- `http_integration/src/ConfigParser.cpp` - Parsing de la configuration
- `http_integration/src/ServerConfig.cpp` - Configuration des serveurs
- `network_layer/src/SocketOps.cpp` - Opérations sur les sockets

---

## 3. BOUCLE ÉVÉNEMENTIELLE (SELECT)

**Fichier:** `network_layer/src/ServerManager.cpp`

```cpp
ServerManager::processEvents()
  ├─> select(_max_fd + 1, &read_cpy, &write_cpy, NULL, &timeout)
  │
  ├─> Pour chaque fd dans read_cpy:
  │   ├─> Si socket serveur → handleServerSocket()
  │   ├─> Si client → handleClientRead()
  │   └─> Si pipe CGI → handleCgiRead()
  │
  └─> Pour chaque fd dans write_cpy:
      ├─> Si client → handleClientWrite()
      └─> Si pipe CGI → handleCgiWrite()
```

**Responsabilités:**
- Surveille les sockets avec `select()`
- Dispatch les événements vers les handlers appropriés
- Gère les timeouts

---

## 4. ACCEPTATION D'UNE NOUVELLE CONNEXION

**Fichier:** `network_layer/src/ServerManager_handlers.cpp`

```cpp
ServerManager::handleServerSocket(ServerConfig& server)
  └─> acceptNewConnection(server)

ServerManager::acceptNewConnection(ServerConfig& server)
  ├─> SocketOps::acceptConnection()           // accept() système
  ├─> SocketOps::setNonBlocking()              // Mode non-bloquant
  ├─> Client client(fd, addr)                 // Crée objet Client
  │   └─> Client contient:
  │       ├─> HttpRequest request              // Pour parser la requête
  │       └─> Response response                // Pour construire la réponse
  ├─> _clients[fd] = client                    // Stocke le client
  └─> _fd_manager.add(fd, _read_set)           // Surveille pour lecture
```

**Fichiers associés:**
- `network_layer/inc/Client.hpp` - Structure du client
- `network_layer/src/Client.cpp` - Implémentation du client

---

## 5. LECTURE DE LA REQUÊTE HTTP

**Fichier:** `network_layer/src/ServerManager_handlers.cpp`

```cpp
ServerManager::handleClientRead(int fd)
  ├─> readFromSocket(fd, client.read_buffer)  // recv() les données
  │   └─> Stocke dans client.read_buffer
  │
  ├─> client.request.feed(data, size)         // ⭐ PARSING HTTP
  │   └─> HttpRequest::feed()                  // Parse caractère par caractère
  │
  └─> Si requestComplete():
      ├─> Sélection du serveur (virtual hosts)
      ├─> client.response.setRequest(request)  // ⭐ Passe la requête à Response
      ├─> client.response.setServer(server)    // ⭐ Configure le serveur
      └─> client.response.buildResponse()       // ⭐ CONSTRUIT LA RÉPONSE
```

**Fichiers associés:**
- `http_integration/src/HttpRequest.cpp` - Parsing HTTP
- `http_integration/inc/HttpRequest.hpp` - Interface HttpRequest

---

## 6. PARSING HTTP (HttpRequest)

**Fichier:** `http_integration/src/HttpRequest.cpp`

```cpp
HttpRequest::feed(char *data, size_t size)
  ├─> Parse caractère par caractère avec machine à états
  │   ├─> Request_Line          // "GET /path HTTP/1.1"
  │   ├─> Request_Line_Method    // GET, POST, DELETE
  │   ├─> Request_Line_URI       // /path?query#fragment
  │   ├─> Request_Line_HTTP      // HTTP/1.1
  │   ├─> Headers                // Headers HTTP
  │   ├─> Message_Body           // Corps de la requête
  │   └─> Parsing_Done          // Requête complète
  │
  └─> Stocke dans:
      ├─> _method                // GET, POST, DELETE
      ├─> _path                  // Chemin de la requête
      ├─> _query                 // Query string
      ├─> _request_headers        // Map des headers
      └─> _body_str              // Corps de la requête
```

**États de parsing:**
- `Request_Line` → Parse la ligne de requête
- `Headers` → Parse les headers HTTP
- `Message_Body` → Parse le corps (Content-Length ou Chunked)
- `Parsing_Done` → Requête complète

**Méthodes importantes:**
- `getMethod()` - Retourne la méthode HTTP
- `getPath()` - Retourne le chemin
- `getHeader(name)` - Retourne un header
- `getBody()` - Retourne le corps
- `parsingCompleted()` - Vérifie si parsing terminé

---

## 7. CONSTRUCTION DE LA RÉPONSE (Response)

**Fichier:** `http_integration/src/Response.cpp`

```cpp
Response::buildResponse()
  ├─> reqError()                                // Vérifie erreurs de requête
  │   └─> Si erreur → buildErrorBody()
  │
  ├─> Trouve la Location correspondante         // Match du path avec config
  │
  ├─> Vérifie méthode autorisée                // GET, POST, DELETE
  │   └─> Si non autorisée → code 405
  │
  ├─> Vérifie redirection (return)             // Location avec return
  │   └─> Si redirection → code 301
  │
  ├─> Construit le chemin cible (_target_file)
  │   ├─> Si alias → replaceAlias()
  │   └─> Sinon → appendRoot()
  │
  ├─> Vérifie si fichier/dossier existe
  │
  ├─> Si CGI requis:
  │   └─> handleCgi() / handleCgiTemp()
  │       ├─> CgiHandler::initEnvCgi()         // Variables d'environnement
  │       ├─> pipe()                           // Crée pipes
  │       ├─> fork() + execve()                 // Lance script CGI
  │       └─> _cgi = 1                         // Flag CGI actif
  │
  ├─> Sinon (fichier statique):
  │   ├─> readFile()                           // Lit le fichier
  │   ├─> buildBody()                          // Construit le corps
  │   └─> setStatusLine()                      // "HTTP/1.1 200 OK"
  │
  ├─> setHeaders()                             // Construit les headers
  │   ├─> contentType()                        // Content-Type
  │   ├─> contentLength()                      // Content-Length
  │   ├─> connection()                         // Connection: keep-alive/close
  │   ├─> server()                             // Server: LETSGO
  │   ├─> location()                           // Location (redirection)
  │   └─> date()                               // Date
  │
  └─> Assemble response_content:
      ├─> Status line                          // "HTTP/1.1 200 OK\r\n"
      ├─> Headers                              // "Content-Type: ...\r\n..."
      └─> Body                                 // Contenu du fichier
```

**Fichiers associés:**
- `http_integration/inc/Response.hpp` - Interface Response
- `http_integration/src/CgiHandler.cpp` - Gestion CGI
- `http_integration/src/Mime.cpp` - Types MIME

**Méthodes importantes:**
- `buildResponse()` - Point d'entrée principal
- `getRes()` - Retourne la réponse complète (headers + body)
- `getCode()` - Code de statut HTTP
- `getCgiState()` - État CGI (0=pas CGI, 1=actif, 2=terminé)

---

## 8. ENVOI DE LA RÉPONSE

**Fichier:** `network_layer/src/ServerManager_handlers.cpp`

```cpp
ServerManager::handleClientWrite(int fd)
  ├─> writeToSocket(fd, client.write_buffer, client.write_offset)
  │   └─> send(fd, buffer + offset, size - offset, 0)
  │       └─> Met à jour write_offset
  │
  └─> Si write_offset >= write_buffer.size():
      ├─> _fd_manager.remove(fd, _write_set)   // Retire de write_set
      └─> closeClient(fd)                       // Ferme la connexion
```

**Cas CGI:**
```cpp
// Si CGI actif, la réponse est construite progressivement
ServerManager::readCgiResponse(int client_fd)
  ├─> read(pipe_out[0], buffer)                // Lit sortie CGI
  ├─> response_content.append(buffer)          // Ajoute à la réponse
  ├─> Si headers complets (\r\n\r\n):
  │   └─> client.write_buffer = response.getRes()
  │       └─> Ajoute client_fd à _write_set
  └─> Quand EOF:
      ├─> waitpid()                            // Attend fin du processus
      └─> setCgiState(2)                       // CGI terminé
```

**Fichiers associés:**
- `network_layer/src/ServerManager_io.cpp` - Fonctions I/O

---

## 9. FERMETURE DE LA CONNEXION

**Fichier:** `network_layer/src/ServerManager_io.cpp`

```cpp
ServerManager::closeClient(int fd)
  ├─> _fd_manager.remove(fd, _read_set)        // Retire de read_set
  ├─> _fd_manager.remove(fd, _write_set)       // Retire de write_set
  ├─> SocketOps::closeSocket(fd)                // close(fd)
  ├─> _clients.erase(fd)                       // Supprime du map
  └─> _active_connections--                     // Décrémente compteur
```

---

## RÉSUMÉ DU FLUX COMPLET

### Exemple: Requête `GET /index.html`

1. **main.cpp** → Démarre le serveur
2. **ServerManager::loadConfig()** → Charge config, crée socket d'écoute
3. **ServerManager::run()** → Entre dans la boucle `select()`
4. **acceptNewConnection()** → Accepte connexion, crée `Client` avec `HttpRequest` et `Response`
5. **handleClientRead()** → Lit données avec `recv()`
6. **HttpRequest::feed()** → Parse "GET /index.html HTTP/1.1\r\n..."
7. **Response::buildResponse()** → 
   - Trouve location
   - Lit fichier `index.html`
   - Construit "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n..."
8. **handleClientWrite()** → Envoie réponse avec `send()`
9. **closeClient()** → Ferme connexion

---

## FICHIERS CLÉS PAR ORDRE D'EXÉCUTION

### 1. Point d'entrée
- `network_layer/src/main.cpp`

### 2. Gestion réseau
- `network_layer/src/ServerManager.cpp` - Boucle événementielle
- `network_layer/src/ServerManager_handlers.cpp` - Handlers (read/write)
- `network_layer/src/ServerManager_io.cpp` - I/O (readFromSocket, writeToSocket)
- `network_layer/inc/ServerManager.hpp`
- `network_layer/inc/Client.hpp`

### 3. Parsing HTTP
- `http_integration/src/HttpRequest.cpp` ⭐ **PARSING REQUÊTE**
- `http_integration/inc/HttpRequest.hpp`

### 4. Construction réponse
- `http_integration/src/Response.cpp` ⭐ **CONSTRUCTION RÉPONSE**
- `http_integration/inc/Response.hpp`
- `http_integration/src/CgiHandler.cpp` - Gestion CGI
- `http_integration/src/Mime.cpp` - Types MIME

### 5. Configuration
- `http_integration/src/ConfigParser.cpp` - Parse .conf
- `http_integration/src/ServerConfig.cpp` - Config serveur
- `http_integration/src/Location.cpp` - Config location

### 6. Utilitaires
- `http_integration/src/Utils.cpp` - Fonctions utilitaires
- `network_layer/src/SocketOps.cpp` - Opérations sockets
- `network_layer/src/Logger.cpp` - Logging

---

## POINTS D'ENTRÉE PRINCIPAUX

### Pour comprendre HttpRequest:
1. `http_integration/inc/HttpRequest.hpp` - Interface
2. `http_integration/src/HttpRequest.cpp` - Ligne 86: `feed()` - Parsing
3. `network_layer/src/ServerManager_handlers.cpp` - Ligne 90: Utilisation

### Pour comprendre Response:
1. `http_integration/inc/Response.hpp` - Interface
2. `http_integration/src/Response.cpp` - Ligne 227: `buildResponse()` - Construction
3. `network_layer/src/ServerManager_handlers.cpp` - Ligne 121: Utilisation

### Pour comprendre le flux complet:
1. `network_layer/src/main.cpp` - Point d'entrée
2. `network_layer/src/ServerManager.cpp` - Ligne 140: `processEvents()` - Boucle
3. `network_layer/src/ServerManager_handlers.cpp` - Handlers

---

## NOTES IMPORTANTES

- **Non-bloquant:** Tous les sockets sont en mode non-bloquant
- **Select:** Utilise `select()` pour multiplexer les I/O
- **État machine:** HttpRequest utilise une machine à états pour parser
- **CGI:** Géré avec pipes et fork/execve
- **Virtual hosts:** Support via matching du header Host
- **Keep-alive:** Géré via header Connection
