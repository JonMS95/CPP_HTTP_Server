#include "test_api.hpp"


/************************************/
/******** Include statements ********/
/************************************/

#include <netinet/in.h>     // INET_ADDRSTRLEN.
#include <arpa/inet.h>      // sockaddr_in, inet_addr
#include <unistd.h>
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

#define HTTP_SERVER_LEN_RX_BUFFER             8192    // RX buffer size.
#define HTTP_SERVER_LEN_TX_BUFFER             8192    // TX buffer size.

#define HTTP_SERVER_MSG_DATA_READ_FROM_CLIENT     "Data read from client: <\r\n%s\r\n>"
#define HTTP_SERVER_MSG_DATA_WRITTEN_TO_CLIENT    "Data written to client: <\r\n%s\r\n>"
#define HTTP_SERVER_MSG_CLIENT_DISCONNECTED       "Client with IP <%s> disconnected."

/************************************/

/************************************/
/********* Private variables ********/
/************************************/

static std::string path_to_resources;

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

bool HttpCheckRequestEnd(std::string& request)
{
    // Check first whether or not is the request message long enough.
    if (request.length() < 4)
        return false;
    
    std::string end = request.substr(request.length() - 4); // Extract last 4 characters
    
    return (end == "\r\n\r\n");
}

/// @brief WIP
/// @param client_socket 
/// @param read_from_client 
/// @return 
int HttpReadFromClient(int& client_socket, std::string& read_from_client)
{
    char rx_buffer[HTTP_SERVER_LEN_RX_BUFFER];
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

                if(read_from_socket == 0)
                {
                    LOG_WNG(HTTP_SERVER_MSG_CLIENT_DISCONNECTED, client_IP_addr);
                    end_connection = 1;
                    http_read_fsm = READ_END;
                }
                else if(read_from_socket > 0)
                {
                    end_connection = 0;
                    http_read_fsm = ADD_TO_READ_DATA;
                }
                else // read_from_socket < 0
                {
                    if(errno != EAGAIN && errno != EWOULDBLOCK && errno != 0)
                        LOG_ERR("ERROR WHILE READING");
                    
                    LOG_WNG("TIMEOUT EXPIRED!");
                    end_connection = 1;
                    http_read_fsm = READ_END;
                }
            }
            break;

            case ADD_TO_READ_DATA:
            {
                read_from_client += rx_buffer;
                memset(rx_buffer, 0, read_from_socket);

                if(HttpCheckRequestEnd(read_from_client))
                {
                    LOG_INF(HTTP_SERVER_MSG_DATA_READ_FROM_CLIENT, read_from_client.data());
                    http_read_fsm = READ_END;
                }
                else
                    http_read_fsm = READ_TRY;
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

    return end_connection;
}

/// @brief Processes first line of message received from the client.
/// @param input First line of message received from the client.
/// @return String vector including method, requested resource and protocol (in that specific order).
std::vector<std::string> HttpExtractWordsFromReqLine(const std::string& input)
{
    std::vector<std::string> words;
    std::istringstream iss(input);
    std::string word;

    while (iss >> word) {
        words.push_back(word);
    }

    return words;
}

/// @brief Processes HTTP request message received from the client.
/// @param read_from_client HTTP request message received from the client.
/// @param request_fields Map to be filled with fields received within client's HTTP request message.
void HttpProcessRequest(const std::string&read_from_client, std::map<std::string, std::string>& request_fields)
{
    // Process every line within the request except for the first one (it has already been analysed).
    std::string line;
    std::istringstream iss(read_from_client);
    
    std::getline(iss, line);
    if(!line.empty() && line[line.size() - 1] == '\n')
        line.erase(line.size() - 1);
    if(!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);

    std::vector<std::string> words_from_req_line = HttpExtractWordsFromReqLine(line);

    if(words_from_req_line[1] == "" || words_from_req_line[1] == "/")
        words_from_req_line[1] = "/index.html";

    LOG_INF("METHOD:    %s", words_from_req_line[0].c_str());
    LOG_INF("RESOURCE:  %s", words_from_req_line[1].c_str());
    LOG_INF("PROTOCOL:  %s", words_from_req_line[2].c_str());

    request_fields["Method"]                = words_from_req_line[0];
    request_fields["Requested resource"]    = words_from_req_line[1];
    request_fields["Protocol"]              = words_from_req_line[2];

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
            request_fields[key] = value;
        else
            LOG_WNG("Unknown field: ", value.c_str());
    }
}

/// @brief Returns file's extension.
/// @param text Requested resource name.
/// @return Empty string if no extension could be found, file extension if everything is okay.
std::string HttpGetFileExtension(const std::string& text)
{
    size_t dotPosition = text.find_last_of('.');
    if (dotPosition != std::string::npos && dotPosition < text.length() - 1)
        return text.substr(dotPosition + 1);
    
    // No dot found or dot is the last character
    return "";
}

/// @brief Sets path to directory in which resources are stored.
/// @param path Path to directory in which resources are stored.
void HttpSetPathToResources(char* path)
{
    path_to_resources = path;
}

/// @brief Gets path to directory in which resources are stored.
/// @return Path to directory in which resources are stored.
std::string HttpGetPathToResources(void)
{
    return path_to_resources;
}

/// @brief Checks whether or not does the file exist.
/// @param filePath File to be checked.
/// @return True if file exists, false otherwise.
bool fileExists(const std::string& filePath)
{
    return std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath);
}

/// @brief Copies requested resource (if found) to string.
/// @param requested_resource Requested resource path.
/// @param dest String in which requested resource is meant to be copied.
/// @return < 0 if file could not be found nor opened, 0 is no error happened.
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

/// @brief Gets content type string based on requested resource's file extension.
/// @param resource_extension Requested resource's file extension.
/// @return Content type string.
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

/// @brief Generates a response based on the requested resource.
/// @param requested_resource Requested resource path.
/// @param httpResponse String object in which HTTP response is meant to be stored.
/// @param resource_extension File extension of resource to be sent.
/// @return HTTP response size in bytes.
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

/// @brief Writes response to client.
/// @param client_socket Client socket.
/// @param httpResponse HTTP response to be sent.
/// @return 0 if no error happened, < 0 otherwise (client disconnected or error while writing).
int HttpWriteToClient(int& client_socket, const std::string& httpResponse)
{
    unsigned long remaining_data_len    = httpResponse.size();
    unsigned long bytes_already_written = 0;

    while(remaining_data_len > 0)
    {
        long int socket_write = ServerSocketWrite(client_socket, httpResponse.data() + bytes_already_written, remaining_data_len);

        if((socket_write < 0))
        {
            if((errno != EAGAIN && errno != EWOULDBLOCK))
                return -104;
            
            continue;
        }

        if(socket_write == 0)
        {
            char client_IP_addr[INET_ADDRSTRLEN] = {};
            ServerSocketGetClientIPv4(client_socket, client_IP_addr);
            LOG_WNG(HTTP_SERVER_MSG_CLIENT_DISCONNECTED, client_IP_addr);
            return -105;
        }

        bytes_already_written   += socket_write;
        remaining_data_len      -= socket_write;
    }

    LOG_DBG(HTTP_SERVER_MSG_DATA_WRITTEN_TO_CLIENT, httpResponse.data());
    
    return 0;
}

/// @brief Reads from client, then sends a response.
/// @param client_socket Client socket.
/// @return < 0 if any error happened, > 0 if want to interact again, 0 otherwise.
int HttpServerRun(int client_socket)
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
    std::map<std::string, std::string> request_fields =
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

    HttpProcessRequest(read_from_client, request_fields);
    if(request_fields["Method"].empty() || request_fields["Requested resource"].empty() || request_fields["Protocol"].empty())
        return 0;
    read_from_client.clear();

    std::string resource_extension = HttpGetFileExtension(request_fields["Requested resource"]);
    LOG_ERR("resource_extension = %s", resource_extension.c_str());

    // After processing the request, generate a proper response.
    std::string httpResponse;
    unsigned long int response_size = HttpGenerateResponse(request_fields["Requested resource"], httpResponse, resource_extension);

    // If response could not be generated, exit and wait for an incoming connection to happen again.
    if(response_size < 0)
        return 0;

    // Finally, send the generated response back to the client.
    int write_to_client = HttpWriteToClient(client_socket, httpResponse);
    // If client got disconnected or any other kind of error happened while trying to wtite, then exit the process.
    if(write_to_client < 0)
        return 0;

    // JMS TESTING. 1 makes the function keep interacting, whereas 0 send the TCP socket back to the ACCEPT state.
    return 1;
    // return 0;
}
