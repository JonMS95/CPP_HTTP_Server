/************************************/
/******** Include statements ********/
/************************************/

#include <netinet/in.h>     // INET_ADDRSTRLEN.
#include <arpa/inet.h>      // sockaddr_in, inet_addr
#include <unistd.h>
#include "HttpServer.hpp"
#include "SeverityLog_api.h"
#include "ServerSocket_api.h"

#include <string>
#include <fstream>          // Read from file to string
#include <vector>
#include <sstream>
#include <filesystem>
#include <map>

/*************************************/

/************************************/
/********* Define statements ********/
/************************************/

#define HTTP_SERVER_MSG_INTANCE_CREATED     "Created HttpServer object instance. Path to HTTP(S) server resources: %s."
#define HTTP_SERVER_MSG_INTANCE_DESTROYED   "Destroyed HttpServer object instance with path to server resources: %s."

/*************************************/

/******************************************/
/******** Class method definitions ********/
/******************************************/

HttpServer::HttpServer(const std::string path_to_resources): path_to_resources(path_to_resources) 
{
    LOG_INF(HTTP_SERVER_MSG_INTANCE_CREATED, this->GetPathToResources().c_str());
}

HttpServer::~HttpServer()
{
    LOG_INF(HTTP_SERVER_MSG_INTANCE_DESTROYED, this->GetPathToResources().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////
// Read data from client

int HttpServer::ReadFromClient(int& client_socket, std::string& read_from_client)
{
    char rx_buffer[HTTP_SERVER_LEN_RX_BUFFER];
    HTTP_READ_FSM http_read_fsm = HTTP_READ_FSM_READ_TRY;
    ssize_t read_from_socket = -1;
    bool keep_trying = true;
    int keep_connected = 0;
    char client_IP_addr[INET_ADDRSTRLEN] = {};

    memset(rx_buffer, 0, sizeof(rx_buffer));
    read_from_client.clear();
    ServerSocketGetClientIPv4(client_socket, client_IP_addr);

    while(keep_trying)
    {
        switch(http_read_fsm)
        {
            case HTTP_READ_FSM_READ_TRY:
            {
                read_from_socket = SERVER_SOCKET_READ(client_socket, rx_buffer);

                if(read_from_socket == 0)
                {
                    LOG_WNG(HTTP_SERVER_MSG_CLIENT_DISCONNECTED, client_IP_addr);
                    keep_connected = -1;
                    http_read_fsm = HTTP_READ_FSM_READ_END;
                }
                else if(read_from_socket > 0)
                {
                    keep_connected = 0;
                    http_read_fsm = HTTP_READ_FSM_ADD_TO_READ_DATA;
                }
                else // read_from_socket < 0
                {
                    switch(errno)
                    {
                        // Firrst, let's fiñter well known errors, such as reset, abort or refusal messages sent by the peer.
                        case ECONNABORTED:
                            LOG_WNG(HTTP_SERVER_MSG_ECONNABORTED, errno);
                        break;

                        case ECONNRESET:
                            LOG_WNG(HTTP_SERVER_MSG_ECONNRESET, errno);
                        break;

                        case ECONNREFUSED:
                            LOG_WNG(HTTP_SERVER_MSG_ECONNREFUSED, errno);
                        break;

                        default:
                        {
                            // Other errors:
                            if(errno != EAGAIN && errno != EWOULDBLOCK && errno != 0)
                                LOG_WNG(HTTP_SERVER_MSG_ERROR_WHILE_READING, errno);
                            else // Timeout expiral
                                LOG_WNG(HTTP_SERVER_MSG_READ_TMT_EXPIRED);
                        }
                        break;
                    }
                    
                    keep_connected = -1;
                    http_read_fsm = HTTP_READ_FSM_READ_END;
                }
            }
            break;

            case HTTP_READ_FSM_ADD_TO_READ_DATA:
            {
                read_from_client += rx_buffer;
                memset(rx_buffer, 0, read_from_socket);

                if(this->CheckRequestEnd(read_from_client))
                {
                    LOG_INF(HTTP_SERVER_MSG_DATA_READ_FROM_CLIENT, read_from_client.data());
                    http_read_fsm = HTTP_READ_FSM_READ_END;
                }
                else
                    http_read_fsm = HTTP_READ_FSM_READ_TRY;
            }
            break;

            case HTTP_READ_FSM_READ_END:
            {
                keep_trying = false;
            }
            break;

            default:
            break;
        }
    }   

    return keep_connected;
}

bool HttpServer::CheckRequestEnd(std::string& request)
{
    // Check first whether or not is the request message long enough.
    if (request.length() < 4)
        return false;
    
    std::string end = request.substr(request.length() - 4); // Extract last 4 characters
    
    return (end == HTTP_SERVER_HTTP_MSG_END);
}

/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// Process request

int HttpServer::ProcessRequest(const std::string&read_from_client, std::map<const std::string, std::string>& request_fields)
{
    // Process every line within the request except for the first one (it has already been analysed).
    std::string line;
    std::istringstream iss(read_from_client);
    
    std::getline(iss, line);
    if(!line.empty() && line[line.size() - 1] == '\n')
        line.erase(line.size() - 1);
    if(!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);

    std::vector<std::string> words_from_req_line = this->ExtractWordsFromReqLine(line);

    LOG_INF(HTTP_SERVER_MSG_RQST_METHOD     , words_from_req_line[0].c_str());
    LOG_INF(HTTP_SERVER_MSG_RQST_RESOURCE   , words_from_req_line[1].c_str());
    LOG_INF(HTTP_SERVER_MSG_RQST_PROTOCOL   , words_from_req_line[2].c_str());

    request_fields.at("Method")             = words_from_req_line[0];
    request_fields.at("Requested resource") = words_from_req_line[1];
    request_fields.at("Protocol")           = words_from_req_line[2];

    if(request_fields.at("Method").empty() || request_fields.at("Requested resource").empty() || request_fields.at("Protocol").empty())
    {
        LOG_ERR(HTTP_SERVER_MSG_BASIC_RQST_FIELD_MISSING);
        return HTTP_SERVER_ERR_BASIC_RQST_FIELDS_FAILED;
    }

    while(std::getline(iss, line))
    {
        size_t pos = line.find(": ");
        std::string key;
        std::string value;
        
        if(!line.empty() && line[line.size() - 1] == '\n')
            line.erase(line.size() - 1);
        if(!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        if(line.empty())
            continue;

        // If ": " found, split the line into two parts
        if (pos != std::string::npos)
        {
            key     = line.substr(0, pos);  // Before ": "
            value   = line.substr(pos + 2); // After ": "
        }

        if(request_fields.count(key) != 0)
            request_fields.at(key) = value;
        else
            LOG_WNG(HTTP_SERVER_MSG_UNKNOWN_RQST_FIELD, value.c_str());
    }

    return 0;
}

std::vector<std::string> HttpServer::ExtractWordsFromReqLine(const std::string& input)
{
    std::vector<std::string> words;
    std::istringstream iss(input);
    std::string word;

    while (iss >> word)
        words.push_back(word);

    return words;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Generate response for client

unsigned long int HttpServer::GenerateResponse(const std::string& requested_resource, std::string& httpResponse)
{
    std::string resource_file;
    std::string content_type ;

    const std::string resource_to_send = this->CheckResourceToSend(requested_resource);

    try
    {
        content_type = std::string((*(this->ptr_extension_to_content)).at(this->GetFileExtension(resource_to_send)));
    }
    catch(const std::out_of_range& e)
    {
        LOG_ERR(HTTP_SERVER_MSG_UNKNOWN_CONTENT_TYPE, this->GetFileExtension(resource_to_send).c_str());
        return HTTP_SERVER_ERR_UNKOWN_RQST_DATA_TYPE;
    }

    int get_html = this->CopyFileToString(resource_to_send, resource_file);
    if(get_html < 0)
        return get_html;

    httpResponse =  "HTTP/1.1 200 OK\r\n"
                    "Content-Type: "        +  content_type                         + "\r\n"
                    "Content-Length: "      +  std::to_string(resource_file.size()) + "\r\n"
                    "Connection: keep-alive\r\n"
                    "\r\n" +
                    resource_file;

    return httpResponse.size();
}

const std::string HttpServer::CheckResourceToSend(const std::string& requested_resource)
{
    std::string requested_resource_aux = std::string(requested_resource);

    // If no resource has been specified, then return the index page by default.
    if(requested_resource == "" || requested_resource == "/")
        requested_resource_aux = HTTP_SERVER_DEFAULT_PAGE;
    
    if(this->FileExists(this->GetPathToResources() + requested_resource_aux))
        requested_resource_aux =  this->GetPathToResources() + requested_resource_aux;
    else
        requested_resource_aux =  this->GetPathToResources() + HTTP_SERVER_DEFAULT_ERROR_404_PAGE;

    return (const std::string)requested_resource_aux;
}

const std::string HttpServer::GetPathToResources(void)
{
    return this->path_to_resources;
}

bool HttpServer::FileExists(const std::string& filePath)
{
    return std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath);
}

std::string HttpServer::GetFileExtension(const std::string& text)
{
    size_t dotPosition = text.find_last_of('.');
    if (dotPosition != std::string::npos && dotPosition < text.length() - 1)
        return text.substr(dotPosition + 1);
    
    // No dot found or dot is the last character
    return "";
}

int HttpServer::CopyFileToString(const std::string& path_to_requested_resource, std::string& dest)
{
    std::ifstream file(path_to_requested_resource); // Open the file

    // If not even the 404 error page could be found, then an error sould be returned.
    if (!file.is_open())
    {
        LOG_ERR(HTTP_SERVER_MSG_OPENING_FILE, path_to_requested_resource.c_str());
        return HTTP_SERVER_ERR_REQUESTED_FILE_NOT_FOUND;
    }

    // Read the file content into an std::string
    dest.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    const std::string mime_data_type = this->GetMIMEDataType((*(this->ptr_extension_to_content)).at(this->GetFileExtension(path_to_requested_resource)));

    if(mime_data_type == "text")
    {
        // Replace '\n' with "\r\n"
        size_t pos = 0;
        while ((pos = dest.find('\n', pos)) != std::string::npos)
        {
            dest.replace(pos, 1, "\r\n");

            // Move past the inserted "\r\n"
            pos += 2;
        }
    }

    // Close the file
    file.close();

    return 0;
}

const std::string HttpServer::GetMIMEDataType(const std::string& content_type)
{
    size_t pos = content_type.find('/');

    if (pos != std::string::npos)
        return content_type.substr(0, pos);
    else
        return content_type;
}

/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// Write to client

int HttpServer::WriteToClient(int& client_socket, const std::string& httpResponse)
{
    unsigned long remaining_data_len    = httpResponse.size();
    unsigned long bytes_already_written = 0;
    
    HTTP_WRITE_FSM http_write_fsm = HTTP_WRITE_FSM_WRITE_TRY;
    int end_connection = 0;
    bool keep_trying = true;

    while(keep_trying)
    {
        switch(http_write_fsm)
        {
            case HTTP_WRITE_FSM_WRITE_TRY:
            {
                long int socket_write = ServerSocketWrite(client_socket, httpResponse.data() + bytes_already_written, remaining_data_len);

                if((socket_write < 0))
                {
                    if((errno != EAGAIN && errno != EWOULDBLOCK))
                    {
                        end_connection = -1;
                        http_write_fsm = HTTP_WRITE_FSM_WRITE_END;
                    }
                }
                else if(socket_write == 0)
                {
                    char client_IP_addr[INET_ADDRSTRLEN] = {};
                    ServerSocketGetClientIPv4(client_socket, client_IP_addr);
                    LOG_WNG(HTTP_SERVER_MSG_CLIENT_DISCONNECTED, client_IP_addr);
                    
                    end_connection = -1;
                    http_write_fsm = HTTP_WRITE_FSM_WRITE_END;
                }
                else // socket_write > 0
                {
                    bytes_already_written   += socket_write;
                    remaining_data_len      -= socket_write;

                    // If no partial write has been detected, then jump to HTTP_WRITE_FSM_WRITE_END state.
                    // Otherwise, keep writing.
                    if(remaining_data_len == 0)
                    {
                        LOG_DBG(HTTP_SERVER_MSG_DATA_WRITTEN_TO_CLIENT, httpResponse.data());
                        http_write_fsm = HTTP_WRITE_FSM_WRITE_END;
                        end_connection = 0;
                    }
                    else
                        LOG_WNG(HTTP_SERVER_MSG_PARTIAL_WRITE, bytes_already_written, remaining_data_len);
                }
            }
            break;

            case HTTP_WRITE_FSM_WRITE_END:
            {
                keep_trying = false;
            }
            break;

            default:
            break;
        }
    }

    return end_connection;
}

/******************************************/