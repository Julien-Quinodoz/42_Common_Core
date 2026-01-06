# WebServ - Serveur HTTP en C++

## 📋 Description

**WebServ** est un serveur HTTP complet implémenté en C++98, développé dans le cadre du projet 42. Ce projet permet de comprendre en profondeur le fonctionnement des serveurs web modernes en recréant les mécanismes fondamentaux d'un serveur HTTP.

Le serveur supporte :
- ✅ Méthodes HTTP : GET, POST, DELETE
- ✅ Gestion de plusieurs serveurs virtuels (virtual hosts)
- ✅ Parsing complet des requêtes HTTP (RFC 7230)
- ✅ Support CGI (Common Gateway Interface)
- ✅ Configuration via fichier de configuration (style nginx)
- ✅ Gestion des fichiers statiques et autoindex
- ✅ Gestion des erreurs HTTP personnalisées
- ✅ Multiplexage I/O avec `select()`
- ✅ Sockets non-bloquants

---

## 🚀 Mise en route

### Prérequis

- Compilateur C++ compatible C++98 (g++, clang++)
- Make
- Python3 (pour les scripts CGI de test)
- Bash (pour les scripts CGI de test)

### Compilation

```bash
# Compiler le projet
make

# Nettoyer les fichiers objets
make clean

# Nettoyer tout (objets + binaire)
make fclean

# Recompiler depuis zéro
make re
```

### Exécution

```bash
# Lancer le serveur avec la configuration par défaut
make run

# Ou directement
./webserv config/default.conf

# Avec un fichier de configuration personnalisé
./webserv config/test.conf
```

### Test rapide

Une fois le serveur lancé, ouvrez votre navigateur ou utilisez `curl` :

```bash
# Test simple
curl http://localhost:8080

# Test avec un fichier spécifique
curl http://localhost:8080/index.html

# Test de l'autoindex
curl http://localhost:8080/42-webserv

# Test CGI
curl http://localhost:8080/cgi-bin/time.py
```

---

## 📁 Structure du projet

```
WebServ/
├── network_layer/          # Couche réseau (gestion des sockets)
│   ├── inc/                # Headers
│   │   ├── ServerManager.hpp
│   │   ├── Client.hpp
│   │   ├── FdSetManager.hpp
│   │   └── ...
│   └── src/                # Sources
│       ├── main.cpp        # Point d'entrée
│       ├── ServerManager.cpp
│       ├── Client.cpp
│       └── ...
│
├── http_integration/       # Couche HTTP (parsing et réponse)
│   ├── inc/                # Headers
│   │   ├── HttpRequest.hpp
│   │   ├── Response.hpp
│   │   ├── ServerConfig.hpp
│   │   └── ...
│   └── src/                # Sources
│       ├── HttpRequest.cpp # Parsing HTTP
│       ├── Response.cpp    # Construction réponse
│       ├── CgiHandler.cpp  # Gestion CGI
│       └── ...
│
├── config/                 # Fichiers de configuration
│   ├── default.conf
│   └── test.conf
│
├── docs/                   # Fichiers statiques servis
│   ├── index.html
│   ├── error_pages/
│   └── 42-webserv/
│
├── cgi-bin/                # Scripts CGI de test
│   ├── time.py
│   ├── calc.py
│   └── env.py
│
└── Makefile
```

---

## 🔧 Configuration

Le serveur utilise un fichier de configuration au format similaire à nginx. Voici un exemple minimal :

```nginx
server {
    listen 8080;                    # Port d'écoute
    server_name localhost;          # Nom du serveur (virtual host)
    host 0.0.0.0;                   # Adresse IP (0.0.0.0 = toutes interfaces)
    root docs/;                     # Répertoire racine
    client_max_body_size 2042042;   # Taille max du body (en octets)
    index index.html;               # Fichier index par défaut
    error_page 404 error_pages/404.html;  # Page d'erreur personnalisée

    location / {
        allow_methods GET POST DELETE;  # Méthodes autorisées
        autoindex off;                  # Désactiver l'autoindex
    }

    location /cgi-bin {
        root ./;
        allow_methods GET POST DELETE;
        index time.py;
        cgi_path /usr/bin/python3 /bin/bash;  # Chemins des interpréteurs
        cgi_ext .py .sh;                      # Extensions CGI
    }

    location /42-webserv {
        allow_methods GET POST DELETE;
        autoindex on;  # Activer l'affichage du répertoire
    }
}
```

### Directives principales

- **`listen`** : Port d'écoute du serveur
- **`server_name`** : Nom du serveur virtuel (utilisé pour le matching)
- **`host`** : Adresse IP d'écoute
- **`root`** : Répertoire racine pour servir les fichiers
- **`index`** : Fichier par défaut si le chemin se termine par `/`
- **`client_max_body_size`** : Taille maximale du corps de requête
- **`error_page`** : Mapper un code d'erreur à une page HTML
- **`location`** : Bloc de configuration pour un chemin spécifique
  - **`allow_methods`** : Méthodes HTTP autorisées
  - **`autoindex`** : Activer/désactiver l'affichage du répertoire
  - **`cgi_path`** : Chemins vers les interpréteurs (Python, Bash, etc.)
  - **`cgi_ext`** : Extensions de fichiers qui déclenchent CGI

---

## 🎓 Notions pédagogiques

### 1. Architecture en couches

Le projet est organisé en deux couches principales :

#### **Couche réseau (`network_layer/`)**
Responsable de :
- La gestion des sockets (création, acceptation, fermeture)
- Le multiplexage I/O avec `select()`
- La gestion des connexions clients
- La lecture/écriture non-bloquante

**Concepts clés :**
- **Sockets** : Point de communication réseau (TCP/IP)
- **File descriptors (fd)** : Identifiants numériques des ressources (sockets, fichiers, pipes)
- **Non-blocking I/O** : Opérations qui ne bloquent pas l'exécution
- **Multiplexage** : Surveiller plusieurs sockets simultanément avec `select()`

#### **Couche HTTP (`http_integration/`)**
Responsable de :
- Le parsing des requêtes HTTP
- La construction des réponses HTTP
- La gestion de la configuration
- L'exécution des scripts CGI

**Concepts clés :**
- **Protocole HTTP** : Format des requêtes/réponses (RFC 7230)
- **Machine à états** : Parsing caractère par caractère
- **CGI** : Interface pour exécuter des scripts externes

---

### 2. Flux d'exécution complet

```
1. DÉMARRAGE
   main.cpp → Charge config → Crée sockets d'écoute

2. BOUCLE ÉVÉNEMENTIELLE
   select() → Surveille tous les sockets
   
3. NOUVELLE CONNEXION
   accept() → Crée socket client → Ajoute à la surveillance
   
4. REQUÊTE HTTP
   recv() → Lit données → HttpRequest::feed() → Parse requête
   
5. CONSTRUCTION RÉPONSE
   Response::buildResponse() → Trouve fichier/CGI → Construit réponse
   
6. ENVOI RÉPONSE
   send() → Envoie données → Met à jour offset
   
7. FERMETURE
   close() → Nettoie ressources
```

---

### 3. Multiplexage I/O avec `select()`

**Pourquoi `select()` ?**

Sans multiplexage, un serveur devrait :
- Soit créer un thread/processus par client (coûteux)
- Soit traiter les clients un par un (lent)

Avec `select()`, un seul thread surveille **tous** les sockets simultanément :

```cpp
// Pseudo-code du principe
fd_set read_set, write_set;

while (running) {
    // Copie les sets (select() modifie les sets)
    fd_set read_cpy = read_set;
    fd_set write_cpy = write_set;
    
    // Attend qu'au moins un socket soit prêt
    select(max_fd + 1, &read_cpy, &write_cpy, NULL, &timeout);
    
    // Vérifie quels sockets sont prêts
    for (chaque socket dans read_cpy) {
        if (socket serveur) {
            acceptNewConnection();
        } else {
            handleClientRead();
        }
    }
    
    for (chaque socket dans write_cpy) {
        handleClientWrite();
    }
}
```

**Avantages :**
- ✅ Un seul thread gère tous les clients
- ✅ Efficace pour des milliers de connexions
- ✅ Pas de surcharge de threads/processus

**Limitations :**
- ⚠️ Limité à ~1024 file descriptors (FD_SETSIZE)
- ⚠️ Moins performant que `epoll()` (Linux) ou `kqueue()` (macOS) pour très grand nombre

---

### 4. Parsing HTTP avec machine à états

Le parsing HTTP est complexe car :
- La requête arrive par morceaux (pas tout d'un coup)
- Il faut valider la syntaxe en temps réel
- Plusieurs formats possibles (Content-Length, Chunked, etc.)

**Solution : Machine à états finie (Finite State Machine)**

```cpp
enum ParsingState {
    Request_Line,        // "GET /path HTTP/1.1"
    Request_Line_Method, // Parse "GET"
    Request_Line_URI,    // Parse "/path"
    Headers,            // Parse les headers
    Message_Body,       // Parse le corps
    Parsing_Done        // Terminé
};

void HttpRequest::feed(char *data, size_t size) {
    for (chaque caractère) {
        switch (_state) {
            case Request_Line:
                if (caractère == ' ') {
                    _state = Request_Line_URI;
                }
                break;
            // ... transitions d'état
        }
    }
}
```

**Pourquoi cette approche ?**
- ✅ Parse caractère par caractère (streaming)
- ✅ Détecte les erreurs immédiatement
- ✅ Gère les requêtes incomplètes
- ✅ Respecte la RFC 7230

---

### 5. Common Gateway Interface (CGI)

**Qu'est-ce que CGI ?**

CGI permet d'exécuter des scripts (Python, Bash, etc.) côté serveur pour générer du contenu dynamique.

**Flux CGI :**

```
1. Client envoie : GET /cgi-bin/time.py
2. Serveur détecte : Extension .py → CGI requis
3. Serveur crée :
   - Variables d'environnement (REQUEST_METHOD, QUERY_STRING, etc.)
   - Pipes pour communication (stdin/stdout)
4. Serveur lance : fork() + execve("/usr/bin/python3", ["time.py"])
5. Script CGI :
   - Lit stdin (body POST)
   - Écrit sur stdout (réponse HTTP)
6. Serveur lit la sortie du script
7. Serveur envoie la réponse au client
```

**Implémentation :**

```cpp
// Création des pipes
pipe(pipe_in);   // Pour envoyer données au script
pipe(pipe_out);  // Pour recevoir la réponse

// Fork pour créer un processus enfant
pid_t pid = fork();

if (pid == 0) {
    // Processus enfant (script CGI)
    dup2(pipe_in[0], STDIN_FILENO);
    dup2(pipe_out[1], STDOUT_FILENO);
    execve("/usr/bin/python3", argv, env);
} else {
    // Processus parent (serveur)
    // Écrit dans pipe_in[1] (body POST)
    // Lit depuis pipe_out[0] (réponse)
}
```

**Variables d'environnement CGI importantes :**
- `REQUEST_METHOD` : GET, POST, DELETE
- `QUERY_STRING` : Paramètres de l'URL (?key=value)
- `CONTENT_LENGTH` : Taille du body
- `SCRIPT_NAME` : Chemin du script
- `SERVER_NAME` : Nom du serveur

---

### 6. Gestion des fichiers statiques

Pour une requête `GET /index.html` :

```cpp
1. Trouver la Location correspondante
   - Match du chemin avec les locations configurées
   - Appliquer les règles (root, alias, etc.)

2. Construire le chemin complet
   - root + path = docs/index.html

3. Vérifier l'existence
   - stat() pour vérifier si le fichier existe
   - Vérifier les permissions

4. Lire le fichier
   - open() + read() ou std::ifstream

5. Déterminer le Content-Type
   - Extension → Type MIME (text/html, image/jpeg, etc.)

6. Construire la réponse HTTP
   - Status line : "HTTP/1.1 200 OK\r\n"
   - Headers : Content-Type, Content-Length, etc.
   - Body : Contenu du fichier
```

---

### 7. Virtual Hosts (Serveurs virtuels)

Un même serveur peut écouter sur plusieurs ports et servir différents contenus :

```nginx
server {
    listen 8080;
    server_name localhost;
    root docs/;
}

server {
    listen 8081;
    server_name localhost;
    root autre_dossier/;
}
```

**Matching du serveur :**
1. Par défaut : Premier serveur qui correspond au port
2. Avec header `Host` : Serveur avec `server_name` correspondant

---

### 8. Gestion des erreurs HTTP

Le serveur doit gérer de nombreux cas d'erreur :

| Code | Signification | Exemple |
|------|---------------|---------|
| 400 | Bad Request | Syntaxe HTTP invalide |
| 403 | Forbidden | Fichier sans permission |
| 404 | Not Found | Fichier inexistant |
| 405 | Method Not Allowed | POST non autorisé sur cette location |
| 413 | Payload Too Large | Body trop volumineux |
| 500 | Internal Server Error | Erreur serveur (CGI crash, etc.) |

**Construction d'une réponse d'erreur :**

```cpp
void Response::buildErrorBody(short code) {
    // Cherche une page d'erreur personnalisée
    std::string error_page = _server.getErrorPage(code);
    
    if (fichier existe) {
        // Utilise la page personnalisée
        readFile(error_page);
    } else {
        // Génère une page d'erreur par défaut
        _response_body = "<html><body><h1>404 Not Found</h1></body></html>";
    }
    
    setStatusLine();  // "HTTP/1.1 404 Not Found\r\n"
    setHeaders();
}
```

---

## 🔍 Exemples d'utilisation

### Exemple 1 : Requête GET simple

```bash
# Client
curl http://localhost:8080/index.html

# Requête HTTP envoyée
GET /index.html HTTP/1.1
Host: localhost:8080
Connection: keep-alive

# Réponse HTTP reçue
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 1234
Connection: keep-alive
Server: WebServ

<html>...</html>
```

### Exemple 2 : Requête POST avec CGI

```bash
# Client
curl -X POST http://localhost:8080/cgi-bin/calc.py -d "a=5&b=3"

# Requête HTTP envoyée
POST /cgi-bin/calc.py HTTP/1.1
Host: localhost:8080
Content-Type: application/x-www-form-urlencoded
Content-Length: 7

a=5&b=3

# Le serveur :
# 1. Détecte .py → CGI
# 2. Lance Python avec calc.py
# 3. Envoie "a=5&b=3" dans stdin du script
# 4. Reçoit la réponse du script
# 5. Envoie au client
```

### Exemple 3 : Autoindex

```bash
# Client
curl http://localhost:8080/42-webserv

# Réponse : Page HTML listant les fichiers du répertoire
HTTP/1.1 200 OK
Content-Type: text/html

<html>
<body>
<h1>Index of /42-webserv</h1>
<ul>
  <li><a href="page1.html">page1.html</a></li>
  <li><a href="page2.html">page2.html</a></li>
</ul>
</body>
</html>
```

### Exemple 4 : DELETE

```bash
# Client
curl -X DELETE http://localhost:8080/42-webserv/messages/message_1.txt

# Le serveur supprime le fichier (si autorisé)
# Réponse
HTTP/1.1 200 OK
Content-Length: 0
```

---

## 🧪 Tests

Le projet inclut plusieurs configurations de test dans `config/default.conf` :

- **Port 8080** : Configuration principale avec autoindex
- **Port 8081** : Configuration avec autoindex désactivé
- **Port 8082** : Configuration avec restrictions DELETE
- **Port 8083** : Configuration pour tests de charge (siege)

### Scripts de test CGI

- **`cgi-bin/time.py`** : Affiche l'heure actuelle
- **`cgi-bin/calc.py`** : Calculatrice simple
- **`cgi-bin/env.py`** : Affiche les variables d'environnement CGI

---

## 📚 Ressources pour approfondir

### Standards et RFCs

- **RFC 7230** : HTTP/1.1 Message Syntax and Routing
- **RFC 7231** : HTTP/1.1 Semantics and Content
- **RFC 3875** : The Common Gateway Interface (CGI) Version 1.1

### Concepts système

- **Sockets TCP/IP** : `socket()`, `bind()`, `listen()`, `accept()`, `connect()`
- **I/O multiplexing** : `select()`, `poll()`, `epoll()` (Linux), `kqueue()` (macOS)
- **Processus** : `fork()`, `execve()`, `waitpid()`
- **Pipes** : `pipe()`, `dup2()`

### Livres recommandés

- "Unix Network Programming" - W. Richard Stevens
- "HTTP: The Definitive Guide" - David Gourley

---

## 🐛 Dépannage

### Le serveur ne démarre pas

```bash
# Vérifier que le port n'est pas déjà utilisé
lsof -i :8080

# Vérifier les permissions du fichier de config
ls -l config/default.conf
```

### Erreur "Address already in use"

Le port est déjà occupé. Changez le port dans la config ou tuez le processus :

```bash
# Trouver le processus
lsof -i :8080

# Tuer le processus (remplacer PID)
kill -9 PID
```

### CGI ne fonctionne pas

```bash
# Vérifier que Python est installé
which python3

# Vérifier les permissions des scripts
chmod +x cgi-bin/*.py

# Vérifier le shebang dans les scripts
head -1 cgi-bin/time.py  # Doit être #!/usr/bin/python3
```

### Erreur de compilation

```bash
# Nettoyer et recompiler
make fclean
make

# Vérifier la version du compilateur
g++ --version  # Doit supporter C++98
```

---

## 👥 Auteurs

Projet développé dans le cadre de l'école 42 Lausanne.

---

## 📝 Licence

Ce projet est un projet pédagogique de l'école 42.

---

## 🎯 Objectifs pédagogiques atteints

Ce projet permet de maîtriser :

✅ **Programmation système** : Sockets, processus, pipes  
✅ **Réseau** : Protocole HTTP, TCP/IP  
✅ **Architecture logicielle** : Séparation des couches, design modulaire  
✅ **Parsing** : Machines à états, validation de syntaxe  
✅ **Gestion de configuration** : Parsing de fichiers, validation  
✅ **Gestion d'erreurs** : Codes HTTP, pages d'erreur personnalisées  
✅ **Performance** : I/O non-bloquant, multiplexage  

---

**Bon développement ! 🚀**

