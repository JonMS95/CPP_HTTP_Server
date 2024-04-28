#include "test_api.hpp"


/************************************/
/******** Include statements ********/
/************************************/

#include <netinet/in.h>     // INET_ADDRSTRLEN.
#include <openssl/ssl.h>
#include <arpa/inet.h>      // sockaddr_in, inet_addr
#include <unistd.h>
#include "SeverityLog_api.h"
#include "ServerSocket_api.h"

#include <string>
#include <fstream>          // Read from file to string
#include <vector>
#include <sstream>
#include <filesystem>

/*************************************/

/************************************/
/********* Define statements ********/
/************************************/

#define SERVER_SOCKET_LEN_RX_BUFFER             8192    // RX buffer size.
#define SERVER_SOCKET_LEN_TX_BUFFER             8192    // TX buffer size.

#define SERVER_SOCKET_MSG_DATA_READ_FROM_CLIENT     "Data read from client: <\r\n%s\r\n>"
#define SERVER_SOCKET_MSG_DATA_WRITTEN_TO_CLIENT    "Data written to client: <\r\n%s\r\n>"

#define SERVER_SOCKET_MSG_CLIENT_DISCONNECTED   "Client with IP <%s> disconnected."

/************************************/

/************************************/
/********* Private variables ********/
/************************************/

std::string path_to_resources;

typedef enum
{
    READ_TRY = 0        ,
    CLIENT_DISCONNECTED ,
    ADD_TO_READ_DATA    ,
    // CHECK_STH_READ      ,
    NOTHING_READ        ,
    READ_END            ,

} HTTP_READ_FSM;

/************************************/

int HttpReadFromClient(int& client_socket, std::string& read_from_client)
{
    char rx_buffer[SERVER_SOCKET_LEN_RX_BUFFER];
    HTTP_READ_FSM http_read_fsm = READ_TRY;
    ssize_t read_from_socket = -1;
    bool keep_trying = true;
    int end_connection = 1;
    char client_IP_addr[INET_ADDRSTRLEN] = {};

    memset(rx_buffer, 0, sizeof(rx_buffer));
    ServerSocketGetClientIPv4(client_socket, client_IP_addr);

    while(keep_trying)
    {
        switch(http_read_fsm)
        {
            case READ_TRY:
            {
                read_from_socket = SERVER_SOCKET_READ(client_socket, rx_buffer);

                if(errno != EAGAIN && errno != EWOULDBLOCK)
                    LOG_ERR("errno: %d, LINE: %d", errno, __LINE__);

                if(read_from_socket > 0)
                {
                    http_read_fsm = ADD_TO_READ_DATA;
                }
                else if(read_from_socket < 0)
                {
                    http_read_fsm = NOTHING_READ;
                }
                else
                {
                    http_read_fsm = CLIENT_DISCONNECTED;
                }
            }
            break;

            case ADD_TO_READ_DATA:
            {
                read_from_client += rx_buffer;
                memset(rx_buffer, 0, read_from_socket);

                http_read_fsm = READ_TRY;
            }
            break;

            case NOTHING_READ:
            {
                if(read_from_client.size() > 0)
                    end_connection = 0;

                http_read_fsm = READ_END;
            }
            break;

            case CLIENT_DISCONNECTED:
            {
                LOG_WNG(SERVER_SOCKET_MSG_CLIENT_DISCONNECTED, client_IP_addr);

                http_read_fsm = READ_END;
            }
            break;

            case READ_END:
            {
                keep_trying = false;
            }
            break;

            default:
            break;
        }
    }

    if(end_connection == 0 && read_from_client.size() > 0)
        LOG_INF(SERVER_SOCKET_MSG_DATA_READ_FROM_CLIENT, read_from_client.data());
    
    return end_connection;
}

// int HttpReadFromClient(int& client_socket, std::string& read_from_client)
// {
//     char rx_buffer[SERVER_SOCKET_LEN_RX_BUFFER];
//     HTTP_READ_FSM http_read_fsm = READ_TRY;
//     ssize_t read_from_socket = -1;
//     bool something_read = false;
//     bool keep_trying = true;
//     int end_connection = 0;
//     char client_IP_addr[INET_ADDRSTRLEN] = {};

//     memset(rx_buffer, 0, sizeof(rx_buffer));
//     ServerSocketGetClientIPv4(client_socket, client_IP_addr);

//     while(keep_trying)
//     {
//         switch(http_read_fsm)
//         {
//             case READ_TRY:
//             {
//                 read_from_socket = SERVER_SOCKET_READ(client_socket, rx_buffer);

//                 if(read_from_socket > 0)
//                 {
//                     http_read_fsm = ADD_TO_READ_DATA;
//                 }
//                 else if(read_from_socket < 0)
//                 {
//                     http_read_fsm = CHECK_STH_READ;
//                 }
//                 else
//                 {
//                     http_read_fsm = CLIENT_DISCONNECTED;
//                 }
//             }
//             break;

//             case ADD_TO_READ_DATA:
//             {
//                 something_read = true;
//                 read_from_client += rx_buffer;
//                 memset(rx_buffer, 0, read_from_socket);

//                 http_read_fsm = READ_TRY;
//             }
//             break;

//             case CHECK_STH_READ:
//             {
//                 if(something_read)
//                 {
//                     end_connection = 0;
//                     http_read_fsm = READ_END;
//                 }
//                 else
//                     http_read_fsm = READ_TRY;
//             }
//             break;

//             case CLIENT_DISCONNECTED:
//             {
//                 end_connection = 1;
//                 LOG_WNG(SERVER_SOCKET_MSG_CLIENT_DISCONNECTED, client_IP_addr);

//                 http_read_fsm = READ_END;
//             }
//             break;

//             case READ_END:
//             {
//                 keep_trying = false;
//             }
//             break;

//             default:
//             break;
//         }
//     }

//     if(read_from_client.size() > 0)
//         LOG_INF(SERVER_SOCKET_MSG_DATA_READ_FROM_CLIENT, read_from_client.data());
    
//     return end_connection;
// }

std::string HttpGetRequestLine(const std::string& msg_from_client)
{
    size_t new_line_pos = msg_from_client.find("\r\n");

    return msg_from_client.substr(0, new_line_pos);
}

std::vector<std::string> HttpExtractWordsFromReqLine(const std::string& input)
{
    const std::string request_line = HttpGetRequestLine(input);
    std::vector<std::string> words;
    std::istringstream iss(request_line);
    std::string word;

    while (iss >> word) {
        words.push_back(word);
    }

    return words;
}

std::string HttpProcessRequest(const std::string&read_from_client)
{
    std::vector<std::string> words_from_req_line = HttpExtractWordsFromReqLine(read_from_client);

    if(words_from_req_line[1] == "" || words_from_req_line[1] == "/")
        words_from_req_line[1] = "/index.html";

    LOG_INF("METHOD:    %s", words_from_req_line[0].c_str());
    LOG_INF("RESOURCE:  %s", words_from_req_line[1].c_str());
    LOG_INF("PROTOCOL:  %s", words_from_req_line[2].c_str());

    return words_from_req_line[1];
}

std::string HttpGetFileExtension(const std::string& text)
{
    size_t dotPosition = text.find_last_of('.');
    if (dotPosition != std::string::npos && dotPosition < text.length() - 1)
    {
        return text.substr(dotPosition + 1);
    }
    
    // No dot found or dot is the last character
    return "";
}

void HttpSetPathToResources(char* path)
{
    path_to_resources = path;
}

std::string HttpGetPathToResources(void)
{
    return path_to_resources;
}

bool fileExists(const std::string& filePath)
{
    return std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath);
}

int HttpCopyFileToString(const std::string& requested_resource, std::string& dest)
{
    // Create a modifiable copy of the requested resource
    std::string requested_resource_name = requested_resource;
    
    // If no resource has been specified, then return the index page by default.
    if(requested_resource_name == "" || requested_resource_name == "/")
        requested_resource_name = "/index.html";
    
    std::string path_to_requested_resource;

    if(fileExists(HttpGetPathToResources() + requested_resource_name))
        path_to_requested_resource = HttpGetPathToResources() + requested_resource_name;
    else
        path_to_requested_resource = HttpGetPathToResources() + "/page_not_found.html";

    std::ifstream file(path_to_requested_resource); // Open the file

    // If not even the 404 error page could be found, then an error sould be returned.
    if (!file.is_open())
    {
        LOG_ERR("Error opening file \"%s\".", path_to_requested_resource.c_str());
        return -101;
    }

    // Read the file content into an std::string
    dest.assign((   std::istreambuf_iterator<char>(file))   ,
                    std::istreambuf_iterator<char>())       ;

    // // Replace '\n' with "\r\n"
    // size_t pos = 0;
    // while ((pos = dest.find('\n', pos)) != std::string::npos)
    // {
    //     dest.replace(pos, 1, "\r\n");
    //     pos += 2; // Move past the inserted "\r\n"
    // }

    // Close the file
    file.close();

    return 0;
}

std::string HttpDefineContentType(const std::string& resource_extension)
{
    if(resource_extension == "html")
        return "text/html";
    
    if(resource_extension == "ico")
        return "image/x-icon";
    
    if(resource_extension == "png")
        return "image/png";
    
    return "text/html";
}

unsigned long int HttpGenerateResponse(const std::string& requested_resource, std::string& httpResponse, const std::string& resource_extension)
{
    std::string resource_file;
    int get_html = HttpCopyFileToString(requested_resource, resource_file);
    if(get_html < 0)
        return get_html;

    httpResponse = "HTTP/1.1 200 OK\r\n"
                    "Content-Type: " + HttpDefineContentType(resource_extension) + "\r\n"
                    "Content-Length: " +  std::to_string(resource_file.size()) + "\r\n"
                    "\r\n" +
                    resource_file;

    return httpResponse.size();
}

void HttpWriteToClient(int& client_socket, const std::string& httpResponse)
{
    const char* data_to_send = httpResponse.data();

    ServerSocketWrite(client_socket, (char*)httpResponse.data(), httpResponse.size());

    if(errno != EAGAIN && errno != EWOULDBLOCK)
        LOG_ERR("errno: %d, LINE: %d", errno, __LINE__);

    // // JMS TESTING
    // LOG_DBG(SERVER_SOCKET_MSG_DATA_WRITTEN_TO_CLIENT, data_to_send);
    LOG_DBG(SERVER_SOCKET_MSG_DATA_WRITTEN_TO_CLIENT, "data to send");
}

/// @brief Reads from client, then sends a response.
/// @param client_socket Client socket.
/// @param secure True if TLS security is wanted, false otherwise.
/// @param ssl SSL data.
/// @return < 0 if any error happened, > 0 if want to interact again, 0 otherwise.
int HttpServerDefaultResponse(int client_socket)
{
    // First, try to read something from client.
    std::string read_from_client;

    int end_connection = HttpReadFromClient(client_socket, read_from_client);
    
    // If client got disconnected while trying to read, end_connection > 0.
    // In that case, exit and wait for an incoming connection to happen again.
    if(end_connection)
        return 0;
    
    // Once something has been read, process the request. Erase the current request string afterwards,
    // leaving enough space for potential incoming requests.
    std::string requested_resource = HttpProcessRequest(read_from_client);
    read_from_client.clear();

    std::string resource_extension = HttpGetFileExtension(requested_resource);
    LOG_ERR("resource_extension = %s", resource_extension.c_str());

    // After processing the request, generate a proper response.
    std::string httpResponse;
    unsigned long int response_size = HttpGenerateResponse(requested_resource, httpResponse, resource_extension);

    // If response could not be generated, exit and wait for an incoming connection to happen again.
    if(response_size < 0)
        return 0;

    // Finally, send the generated response back to the client.
    HttpWriteToClient(client_socket, httpResponse);

    // JMS TESTING. 1 makes the function keep interacting, whereas 0 send the TCP socket back to the ACCEPT state.
    return 1;
    // return 0;
}
