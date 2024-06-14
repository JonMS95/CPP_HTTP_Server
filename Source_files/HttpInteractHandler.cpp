/************************************/
/******** Include statements ********/
/************************************/

#include "HttpServer.hpp"
#include "HttpInteractHandler.hpp"
#include <string>

/*************************************/

/************************************/
/********* Define statements ********/
/************************************/

/*************************************/

/******************************************/
/******** Class method definitions ********/
/******************************************/

std::string HttpInteractHandler::path_to_resources = "";

void HttpInteractHandler::SetPathToResources(const char* path_to_resources)
{
    HttpInteractHandler::path_to_resources = path_to_resources;
}

int HttpInteractHandler::InteractFn(int client_socket)
{
    HttpServer http_server(HttpInteractHandler::path_to_resources);

    return http_server.Run(client_socket);
}

/******************************************/
