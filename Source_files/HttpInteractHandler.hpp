#ifndef CPP_HTTP_INTERACT_HANDLER_HPP
#define CPP_HTTP_INTERACT_HANDLER_HPP

/************************************/
/******** Include statements ********/
/************************************/

#include "HttpServer.hpp"
#include <string>

/*************************************/

/************************************/
/********* Define statements ********/
/************************************/

/************************************/

/************************************/
/********* Type definitions *********/
/************************************/

/************************************/

/*************************************/
/********** Class definition *********/
/*************************************/

class HttpInteractHandler
{
private:
    static int HttpServerFSM(HttpServer& http_server, int client_socket);
    static std::string path_to_resources;

public:
    static void SetPathToResources(const char* path_to_resources);
    static int InteractFn(int client_socket);
};

/*************************************/

#endif