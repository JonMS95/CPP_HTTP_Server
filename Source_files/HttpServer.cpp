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
    {"default"  ,   "application/octet-stream"                                                  },
};

const std::map<const std::string, const unsigned int> method_to_uint =
{
    {"GET"      , HTTP_SERVER_METHOD_CODE_GET    },
    {"HEAD"     , HTTP_SERVER_METHOD_CODE_HEAD   },
    {"POST"     , HTTP_SERVER_METHOD_CODE_POST   },
    {"PUT"      , HTTP_SERVER_METHOD_CODE_PUT    },
    {"DELETE"   , HTTP_SERVER_METHOD_CODE_DELETE },
    {"CONNECT"  , HTTP_SERVER_METHOD_CODE_CONNECT},
    {"OPTIONS"  , HTTP_SERVER_METHOD_CODE_OPTIONS},
    {"TRACE"    , HTTP_SERVER_METHOD_CODE_TRACE  },
};

HttpServer::HttpServer(const std::string path_to_resources):
    path_to_resources(path_to_resources)                                                    ,
    ptr_extension_to_content(std::make_shared<ext_to_type_table>(extension_to_content_type)),
    ptr_method_to_uint(std::make_shared<method_to_uint_table>(method_to_uint))              
{
    LOG_INF(HTTP_SERVER_MSG_INTANCE_CREATED, this->GetPathToResources().c_str());
}

HttpServer::~HttpServer()
{
    LOG_INF(HTTP_SERVER_MSG_INTANCE_DESTROYED, this->GetPathToResources().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////
// Read data from client

int HttpServer::ReadFromClient(int& client_socket)
{
    char rx_buffer[HTTP_SERVER_LEN_RX_BUFFER];
    HTTP_READ_FSM http_read_fsm = HTTP_READ_FSM_READ_TRY;
    ssize_t read_from_socket = -1;
    bool keep_trying = true;
    int keep_connected = 0;
    char client_IP_addr[INET_ADDRSTRLEN] = {};

    memset(rx_buffer, 0, sizeof(rx_buffer));
    this->read_from_client.clear();
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
                this->read_from_client += rx_buffer;
                memset(rx_buffer, 0, read_from_socket);

                if(this->CheckRequestEnd())
                {
                    LOG_INF(HTTP_SERVER_MSG_DATA_READ_FROM_CLIENT, this->read_from_client.data());
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

bool HttpServer::CheckRequestEnd(void)
{
    // Check first whether or not is the request message long enough.
    if (this->read_from_client.length() < 4)
        return false;
    
    std::string end = this->read_from_client.substr(this->read_from_client.length() - 4); // Extract last 4 characters
    
    return (end == HTTP_SERVER_HTTP_MSG_END);
}

/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// Process request

int HttpServer::ProcessRequest(void)
{
    // Process every line within the request except for the first one (it has already been analysed).
    std::string line;
    std::istringstream iss(this->read_from_client);
    
    std::getline(iss, line);
    if(!line.empty() && line[line.size() - 1] == '\n')
        line.erase(line.size() - 1);
    if(!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);

    // Clean all values found within request_fields map.
    for(std::pair<const std::string, std::string>& p : this->request_fields)
        p.second = "";

    std::vector<std::string> words_from_req_line = this->ExtractWordsFromReqLine(line);

    LOG_INF(HTTP_SERVER_MSG_RQST_METHOD     , words_from_req_line[0].c_str());
    LOG_INF(HTTP_SERVER_MSG_RQST_RESOURCE   , words_from_req_line[1].c_str());
    LOG_INF(HTTP_SERVER_MSG_RQST_PROTOCOL   , words_from_req_line[2].c_str());

    this->request_fields.at("Method")             = words_from_req_line[0];
    this->request_fields.at("Requested resource") = words_from_req_line[1];
    this->request_fields.at("Protocol")           = words_from_req_line[2];

    if(this->request_fields.at("Method").empty() || this->request_fields.at("Requested resource").empty() || this->request_fields.at("Protocol").empty())
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

        if(this->request_fields.count(key) != 0)
            this->request_fields.at(key) = value;
        else
            LOG_WNG(HTTP_SERVER_MSG_UNKNOWN_RQST_FIELD, key.c_str());
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

long int HttpServer::GenerateResponse(void)
{
    std::string content_type ;
    std::string resource_to_send;
    long int requested_resource_size;
    std::string resource_file;
    bool generating_response = true;
    HTTP_GEN_RESP_FSM http_gen_resp_fsm = HTTP_GEN_RESP_FSM_CHECK_REQUEST_METHOD;
    int gen_resp_error = 0;

    // Clear the response string before starting, as well as request and response fields.
    this->http_response.clear()             ;
    this->http_response_status_code.clear() ;

    while(generating_response)
    {
        switch(http_gen_resp_fsm)
        {
            case HTTP_GEN_RESP_FSM_CHECK_REQUEST_METHOD:
            {
                // NOTE: according to RFC 9110 (HTTP semantics), 
                // "All general-purpose servers MUST support the methods GET and HEAD. All other methods are OPTIONAL."
                switch( this->ptr_method_to_uint->at(this->request_fields.at("Method")) )
                {
                    case HTTP_SERVER_METHOD_CODE_GET :
                    case HTTP_SERVER_METHOD_CODE_HEAD:
                    {
                        http_gen_resp_fsm = HTTP_GEN_RESP_FSM_GET_PATH_TO_RESOURCE;
                    }
                    break;

                    case HTTP_SERVER_METHOD_CODE_TRACE:
                    {
                        http_gen_resp_fsm = HTTP_GEN_RESP_FSM_BUILD_TRACE_RESPONSE;
                    }
                    break;

                    default:
                    {
                        LOG_WNG(HTTP_SERVER_MSG_UNSUPPORTED_METHOD, this->request_fields.at("Method").c_str());
                        http_gen_resp_fsm = HTTP_GEN_RESP_FSM_BUILD_SERVER_UNSUPPORTED_METHOD_RESPONSE;
                    }
                    break;
                }
            }
            break;

            case HTTP_GEN_RESP_FSM_GET_PATH_TO_RESOURCE:
            {
                // Get the path to the requested resource.
                resource_to_send = this->GetPathToRequestedResource();

                http_gen_resp_fsm = HTTP_GEN_RESP_FSM_CHECK_RESOURCE_EXTENSION;
            }
            break;

            // Check whether or not does the requested resource matches a supported type.
            case HTTP_GEN_RESP_FSM_CHECK_RESOURCE_EXTENSION:
            {
                try
                {
                    // Get file extension of the resource to be sent, then get its proper content type.
                    content_type = std::string( this->ptr_extension_to_content->at( this->ParseFileExtension(resource_to_send) ) );
                }
                catch(const std::out_of_range& e)
                {
                    LOG_WNG(HTTP_SERVER_MSG_UNKNOWN_CONTENT_TYPE, this->ParseFileExtension(resource_to_send).c_str());

                    content_type = std::string( this->ptr_extension_to_content->at("default"));
                }

                http_gen_resp_fsm = HTTP_GEN_RESP_FSM_GET_REQUESTED_RESOURCE_SIZE;
            }
            break;

            // If the resource type is well known by the server, then retrieve its size. Abort if file could not be opened.
            case HTTP_GEN_RESP_FSM_GET_REQUESTED_RESOURCE_SIZE:
            {
                requested_resource_size = this->GetRequestedResourceSize(resource_to_send);

                http_gen_resp_fsm = HTTP_GEN_RESP_FSM_BUILD_RESPONSE_HEADER;
            }
            break;

            // Fill the response message string with data. If request method is equal to HEAD, then just build the header.
            case HTTP_GEN_RESP_FSM_BUILD_RESPONSE_HEADER:
            {
                if(this->resource_not_found)
                    this->http_response_status_code = HTTP_SERVER_STATUS_CODE_404;
                else
                    this->http_response_status_code = HTTP_SERVER_STATUS_CODE_200;

                this->http_response =   this->request_fields.at("Protocol") + " " + this->http_response_status_code + "\r\n"
                                        "Content-Type: "        +   content_type                                    + "\r\n" +
                                        "Content-Length: "      +   std::to_string(requested_resource_size)         + "\r\n" +
                                        "Connection: "          +   this->request_fields.at("Connection")           + "\r\n" +
                                        "\r\n";

                // If request method is GET, then add the resource file as string as well.            
                if(this->request_fields.at("Method") == "GET")
                    http_gen_resp_fsm = HTTP_GEN_RESP_FSM_BUILD_ADD_RESOURCE;
                else
                    http_gen_resp_fsm = HTTP_GEN_RESP_FSM_END_GEN_RESP;
            }
            break;

            // Add response body for GET method requests.
            case HTTP_GEN_RESP_FSM_BUILD_ADD_RESOURCE:
            {
                int get_resource = this->CopyFileToString(resource_to_send, resource_file);
                if(get_resource < 0)
                {
                    gen_resp_error = get_resource;

                    http_gen_resp_fsm = HTTP_GEN_RESP_FSM_END_GEN_RESP;
                }
                
                this->http_response += resource_file;

                http_gen_resp_fsm = HTTP_GEN_RESP_FSM_END_GEN_RESP;
            }
            break;

            case HTTP_GEN_RESP_FSM_BUILD_TRACE_RESPONSE:
            {
                this->http_response_status_code = HTTP_SERVER_STATUS_CODE_200;

                this->http_response =   this->request_fields.at("Protocol") + " " + this->http_response_status_code + "\r\n"
                                        "Content-Type: message/http\r\n"
                                        "\r\n" +
                                        this->read_from_client;
                
                http_gen_resp_fsm = HTTP_GEN_RESP_FSM_END_GEN_RESP;
            }
            break;

            case HTTP_GEN_RESP_FSM_BUILD_SERVER_UNSUPPORTED_METHOD_RESPONSE:
            {
                this->http_response_status_code = HTTP_SERVER_STATUS_CODE_405;

                this->http_response =   "HTTP/1.1 405 Method Not Allowed\r\n"
                                        "Content-Type: text/plain\r\n"
                                        "Content-Length: 30\r\n"
                                        "\r\n"
                                        "METHOD NOT SUPPORTED BY SERVER";

                http_gen_resp_fsm = HTTP_GEN_RESP_FSM_END_GEN_RESP;
            }
            break;

            // Return response size.
            case HTTP_GEN_RESP_FSM_END_GEN_RESP:
            {
                generating_response = false;
            }
            break;

            default:
            break;
        }
    }

    if(gen_resp_error < 0)
        return gen_resp_error;
    
    return this->http_response.size();
}

const std::string HttpServer::GetPathToRequestedResource(void)
{
    std::string requested_resource_aux = std::string(this->request_fields.at("Requested resource"));

    // If no resource has been specified, then return the index page by default.
    if(this->request_fields.at("Requested resource") == "" || this->request_fields.at("Requested resource") == "/")
        requested_resource_aux = HTTP_SERVER_DEFAULT_PAGE;
    
    // Clear resource_not_found first.
    this->resource_not_found = false;

    // Then check if the target resource exist.
    if(this->FileExists(this->GetPathToResources() + requested_resource_aux))
        requested_resource_aux = this->GetPathToResources() + requested_resource_aux;
    else
    {
        this->resource_not_found    = true;
        requested_resource_aux      = this->GetPathToResources() + HTTP_SERVER_DEFAULT_ERROR_404_PAGE_PATH;
    }

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

std::string HttpServer::ParseFileExtension(const std::string& text)
{
    size_t dotPosition = text.find_last_of('.');
    if (dotPosition != std::string::npos && dotPosition < text.length() - 1)
        return text.substr(dotPosition + 1);
    
    // No dot found or dot is the last character
    return "";
}

long int HttpServer::GetRequestedResourceSize(const std::string& resource_to_send)
{
    std::ifstream file(resource_to_send, std::ios::binary | std::ios::ate); // Open file in binary mode and move the file pointer to the end
    
    if (!file.is_open())
    {
        LOG_ERR(HTTP_SERVER_MSG_OPENING_FILE, resource_to_send.c_str());
        return HTTP_SERVER_ERR_REQUESTED_FILE_NOT_FOUND;
    }
    
    std::streampos fileSize = file.tellg(); // Get the position of the file pointer, which is at the end
    file.close();
    return fileSize;
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

    // const std::string mime_data_type = this->GetMIMEDataType(this->ptr_extension_to_content->at(this->ParseFileExtension(path_to_requested_resource)));
    // if(mime_data_type == "text")
    // {
    //     // Replace '\n' with "\r\n"
    //     size_t pos = 0;
    //     while ((pos = dest.find('\n', pos)) != std::string::npos)
    //     {
    //         dest.replace(pos, 1, "\r\n");

    //         // Move past the inserted "\r\n"
    //         pos += 2;
    //     }
    // }

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

int HttpServer::WriteToClient(int& client_socket)
{
    unsigned long remaining_data_len    = this->http_response.size();
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
                long int socket_write = ServerSocketWrite(client_socket, this->http_response.data() + bytes_already_written, remaining_data_len);

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
                        LOG_DBG(HTTP_SERVER_MSG_DATA_WRITTEN_TO_CLIENT, this->http_response.data());
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

int HttpServer::Run(int& client_socket)
{
    bool keep_interacting       = true              ;
    HTTP_RUN_FSM http_run_fsm   = HTTP_RUN_FSM_READ ;

    while(keep_interacting)
    {
        switch(http_run_fsm)
        {
            // First, try to read something from client.
            // If client got disconnected while trying to read or any error happened, end_connection > 0.
            // In that case, exit and wait for an incoming connection to happen again.
            case HTTP_RUN_FSM_READ:
            {
                int end_connection = this->ReadFromClient(client_socket);
                
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
                int process_request = this->ProcessRequest();
                
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
                long int response_size = this->GenerateResponse();

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
                int write_to_client = this->WriteToClient(client_socket);

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

/******************************************/