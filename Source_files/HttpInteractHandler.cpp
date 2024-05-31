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

int HttpInteractHandler::HttpServerFSM(HttpServer& http_server, int client_socket)
{
    bool keep_interacting       = true              ;
    HTTP_RUN_FSM http_run_fsm   = HTTP_RUN_FSM_READ ;
    std::string read_from_client                    ;
    std::string httpResponse                        ;

    std::map<const std::string, std::string> request_fields =
    {
        {"Method"                       , ""},
        {"Requested resource"           , ""},
        {"Protocol"                     , ""},
        {"Host"                         , ""},
        {"Connection"                   , ""},
        {"Cache-Control"                , ""},
        {"Upgrade-Insecure-Requests"    , ""},
        {"User-Agent"                   , ""},
        {"Accept"                       , ""},
        {"Referer"                      , ""},
        {"Accept-Encoding"              , ""},
        {"Accept-Language"              , ""},
    };

    while(keep_interacting)
    {
        switch(http_run_fsm)
        {
            // First, try to read something from client.
            // If client got disconnected while trying to read or any error happened, end_connection > 0.
            // In that case, exit and wait for an incoming connection to happen again.
            case HTTP_RUN_FSM_READ:
            {
                int end_connection = http_server.ReadFromClient(client_socket, read_from_client);
                
                if(end_connection < 0)
                    http_run_fsm = HTTP_RUN_FSM_END_CONNECTION;
                else
                    http_run_fsm = HTTP_RUN_FSM_PROCESS_REQUEST;
            }
            break;

            // Once something has been read, process the request. Erase the current request string afterwards,
            // leaving enough space for potential incoming requests.
            case HTTP_RUN_FSM_PROCESS_REQUEST:
            {
                int process_request = http_server.ProcessRequest(read_from_client, request_fields);
                
                // If request could not be processed properly, then stop interacting with the client.
                if(process_request < 0)
                    http_run_fsm = HTTP_RUN_FSM_END_CONNECTION;
                else
                    http_run_fsm = HTTP_RUN_FSM_GENERATE_RESPONSE;
            }
            break;

            // After processing the request, generate a proper response.
            case HTTP_RUN_FSM_GENERATE_RESPONSE:
            {
                unsigned long int response_size = http_server.GenerateResponse(request_fields.at("Requested resource"), httpResponse);

                // If response could not be generated, exit and wait for an incoming connection to happen again.
                if(response_size < 0)
                    http_run_fsm = HTTP_RUN_FSM_END_CONNECTION;
                else
                    http_run_fsm = HTTP_RUN_FSM_WRITE;
            }
            break;

            // Finally, send the generated response back to the client.
            // If client got disconnected or any other kind of error happened while trying to wtite, then exit the process.
            case HTTP_RUN_FSM_WRITE:
            {
                int write_to_client = http_server.WriteToClient(client_socket, httpResponse);

                if(write_to_client < 0)
                    http_run_fsm = HTTP_RUN_FSM_END_CONNECTION;
                else
                    http_run_fsm = HTTP_RUN_FSM_READ;
            }
            break;

            case HTTP_RUN_FSM_END_CONNECTION:
            {
                keep_interacting = false;
            }
            break;

            default:
            break;
        }
    }

    return 0;
}

void HttpInteractHandler::SetPathToResources(const char* path_to_resources)
{
    HttpInteractHandler::path_to_resources = path_to_resources;
}

int HttpInteractHandler::InteractFn(int client_socket)
{
    HttpServer server(HttpInteractHandler::path_to_resources);

    return HttpInteractHandler::HttpServerFSM(server, client_socket);
}

/******************************************/
