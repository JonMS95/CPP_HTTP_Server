/************************************/
/******** Include statements ********/
/************************************/

#include "HttpServer_api.hpp"
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

void HttpInteract::SetPathToResources(const char* path_to_resources)
{
    HttpInteractHandler::SetPathToResources(path_to_resources);
}

int HttpInteract::InteractFn(int client_socket)
{
    return HttpInteractHandler::InteractFn(client_socket);
}

/******************************************/
