#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "Webserv.hpp"

/* Méthodes HTTP supportées (enum pour numerotation)*/
enum HttpMethod
{
    GET,
    POST,
    DELETE,
    NONE
};

/* Machine à états pour le parsing HTTP caractère par caractère
   Suit la RFC 7230 pour analyser Request-Line, Headers et Body */
enum ParsingState
{
    Request_Line,
    Request_Line_Post,
    Request_Line_Method,
    Request_Line_First_Space,
    Request_Line_URI_Path_Slash,
    Request_Line_URI_Path,
    Request_Line_URI_Query,
    Request_Line_URI_Fragment,
    Request_Line_Ver,
    Request_Line_HT,
    Request_Line_HTT,
    Request_Line_HTTP,
    Request_Line_HTTP_Slash,
    Request_Line_Major,
    Request_Line_Dot,
    Request_Line_Minor,
    Request_Line_CR,
    Request_Line_LF,
    Field_Name_Start,
    Fields_End,
    Field_Name,
    Field_Value,
    Field_Value_End,
    Chunked_Length_Begin,
    Chunked_Length,
    Chunked_Ignore,
    Chunked_Length_CR,
    Chunked_Length_LF,
    Chunked_Data,
    Chunked_Data_CR,
    Chunked_Data_LF,
    Chunked_End_CR,
    Chunked_End_LF,
    Message_Body,
    Parsing_Done
};

/*
  Classe HttpRequest : Parse et stocke les requêtes HTTP
  - Reçoit la requête caractère par caractère (feed)
  - Déclenche un flag quand le parsing est terminé
  - En cas d'erreur, _error_code contient le code HTTP approprié (400, 404, etc.)
*/
class HttpRequest
{
    public:
        HttpRequest();
        ~HttpRequest();

        HttpMethod                                  &getMethod(); // méthode HTTP (GET, POST, DELETE, etc.) 
        std::string                                 &getPath(); // chemin de la requête (URI)
        std::string                                 &getQuery(); // query string de la requête
        std::string                                 &getFragment(); // fragment de la requête
        std::string                                 getHeader(std::string const &); // header de la requête
		const std::map<std::string, std::string>    &getHeaders() const; // headers de la requête
		std::string                                 getMethodStr(); // méthode HTTP en string (GET, POST, DELETE, etc.)
        std::string                                 &getBody(); // body de la requête
        std::string                                 getServerName(); // nom du serveur
        std::string                                 &getBoundary(); // boundary de la requête
        bool                                        getMultiformFlag(); // flag pour le multipart/form-data
        
        /* setters */
        void        setMethod(HttpMethod &);    
        void        setHeader(std::string &, std::string &);
        void        setMaxBodySize(size_t);
        void        setBody(std::string name);

        /* méthodes pour le parsing */
        void        feed(char *data, size_t size);  // reçoit la requête caractère par caractère
        bool        parsingCompleted(); 
        void        printMessage(); 
        void        clear();        
        short       errorCode();    
        bool        keepAlive();    
        void        cutReqBody(int bytes);  

    private:
        std::string                         _path;
        std::string                         _query;
        std::string                         _fragment;
        std::map<std::string, std::string>  _request_headers;
        std::vector<u_int8_t>               _body; 
        std::string                         _boundary;
        HttpMethod                          _method;
        std::map<u_int8_t, std::string>     _method_str;
        ParsingState                        _state;
        size_t                              _max_body_size;
        size_t                              _body_length;
        short                               _error_code;
        size_t                              _chunk_length;
        std::string                         _storage;
        std::string                         _key_storage;
        short                               _method_index;
        u_int8_t                            _ver_major;
        u_int8_t                            _ver_minor;
        std::string                         _server_name;
        std::string                         _body_str;
        /* flags */
        bool                                _fields_done_flag;
        bool                                _body_flag;
        bool                                _body_done_flag;
        bool                                _complete_flag;
        bool                                _chunked_flag;
        bool                                _multiform_flag;

        void            _handle_headers();

};

#endif

