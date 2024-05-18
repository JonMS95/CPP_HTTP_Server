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

#define HTTP_SERVER_LEN_RX_BUFFER                   8192        // RX buffer size.
#define HTTP_SERVER_LEN_TX_BUFFER                   8192        // TX buffer size.
#define HTTP_SERVER_HTTP_MSG_END                    "\r\n\r\n"
#define HTTP_SERVER_DEFAULT_PAGE                    "/index.html"
#define HTTP_SERVER_DEFAULT_ERROR_404_PAGE          "/page_not_found.html"

#define HTTP_SERVER_MSG_DATA_READ_FROM_CLIENT       "Data read from client: <\r\n%s\r\n>"
#define HTTP_SERVER_MSG_DATA_WRITTEN_TO_CLIENT      "Data written to client: <\r\n%s\r\n>"
#define HTTP_SERVER_MSG_CLIENT_DISCONNECTED         "Client with IP <%s> disconnected."
#define HTTP_SERVER_MSG_ERROR_WHILE_READING         "ERROR WHILE READING"
#define HTTP_SERVER_MSG_READ_TMT_EXPIRED            "READ TIMEOUT EXPIRED!"
#define HTTP_SERVER_MSG_UNKNOWN_RQST_FIELD          "Unknown field: "
#define HTTP_SERVER_MSG_OPENING_FILE                "Error opening file \"%s\"."
#define HTTP_SERVER_MSG_UNKNOWN_CONTENT_TYPE        "UNKNOWN CONTENT TYPE (File extension: %s)"
#define HTTP_SERVER_MSG_BASIC_RQST_FIELD_MISSING    "One of the basic request fields (either method, requested resource or protocol) is missing."

#define HTTP_SERVER_ERR_BASIC_RQST_FIELDS_FAILED    -1
#define HTTP_SERVER_ERR_REQUESTED_FILE_NOT_FOUND    -2
#define HTTP_SERVER_ERR_UNKOWN_RQST_DATA_TYPE       -3

/************************************/

/************************************/
/********* Private variables ********/
/************************************/

static std::string path_to_resources;

const std::map<const std::string, const std::string> extension_to_content_type =
{
    {"aac"      ,	"audio/aac"                                                                 },
    {"abw"      ,	"application/x-abiword"                                                     },
    {"apng"     ,   "image/apng"                                                                },
    {"arc"      ,	"application/x-freearc"                                                     },
    {"avif"     ,	"image/avif"                                                                },
    {"avi"      ,	"video/x-msvideo"                                                           },
    {"azw"      ,	"application/vnd.amazon.ebook"                                              },
    {"bin"      ,	"application/octet-stream"                                                  },
    {"bmp"      ,	"image/bmp"                                                                 },
    {"bz"       ,	"application/x-bzip"                                                        },
    {"bz2"      ,	"application/x-bzip2"                                                       },
    {"cda"      ,	"application/x-cdf"                                                         },
    {"csh"      ,	"application/x-csh"                                                         },
    {"css"      ,	"text/css"                                                                  },
    {"csv"      ,	"text/csv"                                                                  },
    {"doc"      ,	"application/msword"                                                        },
    {"docx"     ,	"application/vnd.openxmlformats-officedocument.wordprocessingml.document"   },
    {"eot"      ,	"application/vnd.ms-fontobject"                                             },
    {"epub"     ,	"application/epub+zip"                                                      },
    {"gz"       ,	"application/gzip"                                                          },
    {"gif"      ,	"image/gif"                                                                 },
    {"htm"      ,	"text/html"                                                                 },
    {"html"     ,	"text/html"                                                                 },
    {"ico"      ,	"image/vnd.microsoft.icon"                                                  },
    {"ics"      ,	"text/calendar"                                                             },
    {"jar"      ,	"application/java-archive"                                                  },
    {"jpg"      ,	"image/jpeg"                                                                },
    {"jpeg"     ,	"image/jpeg"                                                                },
    {"js"       ,	"javascript"                                                                },
    {"json"     ,	"application/json"                                                          },
    {"jsonld"   ,	"application/ld+json"                                                       },
    {"mid"      ,	"audio/x-midi"                                                              },
    {"midi"     ,	"audio/x-midi"                                                              },
    {"mjs"      ,	"text/javascript"                                                           },
    {"mp3"      ,	"audio/mpeg"                                                                },
    {"mp4"      ,	"video/mp4"                                                                 },
    {"mpeg"     ,	"video/mpeg"                                                                },
    {"mpkg"     ,	"application/vnd.apple.installer+xml"                                       },
    {"odp"      ,	"application/vnd.oasis.opendocument.presentation"                           },
    {"ods"      ,	"application/vnd.oasis.opendocument.spreadsheet"                            },
    {"odt"      ,	"application/vnd.oasis.opendocument.text"                                   },
    {"oga"      ,	"audio/ogg"                                                                 },
    {"ogv"      ,	"video/ogg"                                                                 },
    {"ogx"      ,	"application/ogg"                                                           },
    {"opus"     ,	"audio/opus"                                                                },
    {"otf"      ,	"font/otf"                                                                  },
    {"png"      ,	"image/png"                                                                 },
    {"pdf"      ,	"application/pdf"                                                           },
    {"php"      ,	"application/x-httpd-php"                                                   },
    {"ppt"      ,	"application/vnd.ms-powerpoint"                                             },
    {"pptx"     ,	"application/vnd.openxmlformats-officedocument.presentationml.presentation" },
    {"rar"      ,	"application/vnd.rar"                                                       },
    {"rtf"      ,	"application/rtf"                                                           },
    {"sh"       ,	"application/x-sh"                                                          },
    {"svg"      ,	"image/svg+xml"                                                             },
    {"tar"      ,	"application/x-tar"                                                         },
    {"tif"      ,	"image/tiff"                                                                },
    {"tiff"     ,	"image/tiff"                                                                },
    {"ts"       ,	"video/mp2t"                                                                },
    {"ttf"      ,	"font/ttf"                                                                  },
    {"txt"      ,	"text/plain"                                                                },
    {"vsd"      ,	"application/vnd.visio"                                                     },
    {"wav"      ,	"audio/wav"                                                                 },
    {"weba"     ,	"audio/webm"                                                                },
    {"webm"     ,	"video/webm"                                                                },
    {"webp"     ,	"image/webp"                                                                },
    {"woff"     ,	"font/woff"                                                                 },
    {"woff2"    ,	"font/woff2"                                                                },
    {"xhtml"    ,	"application/xhtml+xml"                                                     },
    {"xls"      ,	"application/vnd.ms-excel"                                                  },
    {"xlsx"     ,	"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"         },
    {"xml"      ,	"application/xml"                                                           },
    {"xul"      ,	"application/vnd.mozilla.xul+xml"                                           },
    {"zip"      ,	"application/zip"                                                           },
    {"3gp"      ,	"video/3gpp"                                                                },
    {"3g2"      ,	"video/3gpp2"                                                               },
    {"7z"       ,	"application/x-7z-compressed"                                               },
};

/************************************/

/************************************/
/********* Type definitions *********/
/************************************/

typedef enum
{
    HTTP_READ_FSM_READ_TRY              = 0 ,
    HTTP_READ_FSM_CLIENT_DISCONNECTED       ,
    HTTP_READ_FSM_ADD_TO_READ_DATA          ,
    HTTP_READ_FSM_READ_END                  ,
} HTTP_READ_FSM;

typedef enum
{
    HTTP_WRITE_FSM_WRITE_TRY        = 0 ,
    HTTP_WRITE_FSM_WRITE_END            ,
} HTTP_WRITE_FSM;

typedef enum
{
    HTTP_RUN_FSM_READ               = 0 ,
    HTTP_RUN_FSM_PROCESS_REQUEST        ,
    HTTP_RUN_FSM_GENERATE_RESPONSE      ,
    HTTP_RUN_FSM_WRITE                  ,
    HTTP_RUN_FSM_END_CONNECTION         ,
} HTTP_RUN_FSM;

/************************************/

bool HttpCheckRequestEnd(std::string& request)
{
    // Check first whether or not is the request message long enough.
    if (request.length() < 4)
        return false;
    
    std::string end = request.substr(request.length() - 4); // Extract last 4 characters
    
    return (end == HTTP_SERVER_HTTP_MSG_END);
}

/// @brief Tries to read from HTTP client.
/// @param client_socket Target client socket.
/// @param read_from_client String in which data read from client is meant to be stored.
/// @return false if any error happened or client got disconnected, true otherwise.
int HttpReadFromClient(int& client_socket, std::string& read_from_client)
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
                    if(errno != EAGAIN && errno != EWOULDBLOCK && errno != 0)
                        LOG_ERR(HTTP_SERVER_MSG_ERROR_WHILE_READING);
                    else
                        LOG_WNG(HTTP_SERVER_MSG_READ_TMT_EXPIRED);
                    
                    keep_connected = -1;
                    http_read_fsm = HTTP_READ_FSM_READ_END;
                }
            }
            break;

            case HTTP_READ_FSM_ADD_TO_READ_DATA:
            {
                read_from_client += rx_buffer;
                memset(rx_buffer, 0, read_from_socket);

                if(HttpCheckRequestEnd(read_from_client))
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

/// @brief Processes first line of message received from the client.
/// @param input First line of message received from the client.
/// @return String vector including method, requested resource and protocol (in that specific order).
std::vector<std::string> HttpExtractWordsFromReqLine(const std::string& input)
{
    std::vector<std::string> words;
    std::istringstream iss(input);
    std::string word;

    while (iss >> word)
        words.push_back(word);

    return words;
}

/// @brief Processes HTTP request message received from the client.
/// @param read_from_client HTTP request message received from the client.
/// @param request_fields Map to be filled with fields received within client's HTTP request message.
/// @return 0 if everything went OK, < 0 if any of the mandatory field in the first request line is empty.
int HttpProcessRequest(const std::string&read_from_client, std::map<const std::string, std::string>& request_fields)
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

    LOG_INF("METHOD:    %s", words_from_req_line[0].c_str());
    LOG_INF("RESOURCE:  %s", words_from_req_line[1].c_str());
    LOG_INF("PROTOCOL:  %s", words_from_req_line[2].c_str());

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
bool HttpFileExists(const std::string& filePath)
{
    return std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath);
}

const std::string HttpGetMIMEDataType(const std::string& content_type)
{
    size_t pos = content_type.find('/');

    if (pos != std::string::npos)
        return content_type.substr(0, pos);
    else
        return content_type;
}

/// @brief Copies requested resource (if found) to string.
/// @param requested_resource Requested resource path.
/// @param dest String in which requested resource is meant to be copied.
/// @return < 0 if file could not be found nor opened, 0 is no error happened.
int HttpCopyFileToString(const std::string& path_to_requested_resource, std::string& dest)
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

    const std::string mime_data_type = HttpGetMIMEDataType(extension_to_content_type.at(HttpGetFileExtension(path_to_requested_resource)));

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

const std::string HttpCheckResourceToSend(const std::string& requested_resource)
{
    std::string requested_resource_aux = std::string(requested_resource);

    // If no resource has been specified, then return the index page by default.
    if(requested_resource == "" || requested_resource == "/")
        requested_resource_aux = HTTP_SERVER_DEFAULT_PAGE;
    
    if(HttpFileExists(HttpGetPathToResources() + requested_resource_aux))
        requested_resource_aux =  HttpGetPathToResources() + requested_resource_aux;
    else
        requested_resource_aux =  HttpGetPathToResources() + HTTP_SERVER_DEFAULT_ERROR_404_PAGE;

    return (const std::string)requested_resource_aux;
}

/// @brief Generates a response based on the requested resource.
/// @param requested_resource Requested resource path.
/// @param httpResponse String object in which HTTP response is meant to be stored.
/// @return HTTP response size in bytes.
unsigned long int HttpGenerateResponse(const std::string& requested_resource, std::string& httpResponse)
{
    std::string resource_file;
    std::string content_type ;

    const std::string resource_to_send = HttpCheckResourceToSend(requested_resource);

    try
    {
        content_type = std::string(extension_to_content_type.at(HttpGetFileExtension(resource_to_send)));
    }
    catch(const std::out_of_range& e)
    {
        LOG_ERR(HTTP_SERVER_MSG_UNKNOWN_CONTENT_TYPE, HttpGetFileExtension(resource_to_send).c_str());
        return HTTP_SERVER_ERR_UNKOWN_RQST_DATA_TYPE;
    }

    int get_html = HttpCopyFileToString(resource_to_send, resource_file);
    if(get_html < 0)
        return get_html;

    httpResponse =  "HTTP/1.1 200 OK\r\n"
                    "Content-Type: "        +  content_type                         + "\r\n"
                    "Content-Length: "      +  std::to_string(resource_file.size()) + "\r\n"
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
                        end_connection = -104;
                        http_write_fsm = HTTP_WRITE_FSM_WRITE_END;
                    }
                }
                else if(socket_write == 0)
                {
                    char client_IP_addr[INET_ADDRSTRLEN] = {};
                    ServerSocketGetClientIPv4(client_socket, client_IP_addr);
                    LOG_WNG(HTTP_SERVER_MSG_CLIENT_DISCONNECTED, client_IP_addr);
                    
                    end_connection = -105;
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

/// @brief Reads from client, then sends a response.
/// @param client_socket Client socket.
/// @return < 0 if any error happened, > 0 if want to interact again, 0 otherwise.
int HttpServerInteractFn(int client_socket)
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
                int end_connection = HttpReadFromClient(client_socket, read_from_client);
                
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
                int process_request = HttpProcessRequest(read_from_client, request_fields);
                
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
                unsigned long int response_size = HttpGenerateResponse(request_fields.at("Requested resource"), httpResponse);

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
                int write_to_client = HttpWriteToClient(client_socket, httpResponse);

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
