#ifndef CPP_HTTP_SERVER_API_HPP
#define CPP_HTTP_SERVER_API_HPP

/************************************/
/******** Include statements ********/
/************************************/

#include <string>
#include <vector>
#include <map>
#include <functional>

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
#define HTTP_SERVER_MSG_RQST_METHOD                 "METHOD:    %s"
#define HTTP_SERVER_MSG_RQST_RESOURCE               "RESOURCE:  %s"
#define HTTP_SERVER_MSG_RQST_PROTOCOL               "PROTOCOL:  %s"
#define HTTP_SERVER_MSG_BASIC_RQST_FIELD_MISSING    "One of the basic request fields (either method, requested resource or protocol) is missing."

#define HTTP_SERVER_ERR_BASIC_RQST_FIELDS_FAILED    -1
#define HTTP_SERVER_ERR_REQUESTED_FILE_NOT_FOUND    -2
#define HTTP_SERVER_ERR_UNKOWN_RQST_DATA_TYPE       -3

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

/*************************************/
/********** Class definition *********/
/*************************************/

class HttpServer
{
private:
    const unsigned int server_port              ;
    const unsigned int max_clients_num          ;
    const bool concurrency_enabled              ;
    const bool non_blocking             = false ;
    const bool reuse_address            = true  ;
    const bool reuse_port               = true  ;
    const unsigned long rx_timeout              ;  
    const bool secure_connection                ;
    const std::string path_cert                 ;
    const std::string path_pkey                 ;
    const std::string path_to_resources         ;

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

    // Read data from client
    int     ReadFromClient(int& client_socket, std::string& read_from_client)   ;
    bool    CheckRequestEnd(std::string& request)                               ;

    // Process request
    int                         ProcessRequest(const std::string&read_from_client, std::map<const std::string, std::string>& request_fields);
    std::vector<std::string>    ExtractWordsFromReqLine(const std::string& input)                                                           ;

    // Generate response for client
    unsigned long int   GenerateResponse(const std::string& requested_resource, std::string& httpResponse)  ;
    const std::string   CheckResourceToSend(const std::string& requested_resource)                          ;
    std::string         GetPathToResources(void)                                                            ;
    bool                FileExists(const std::string& filePath)                                             ;
    std::string         GetFileExtension(const std::string& text)                                           ;
    int                 CopyFileToString(const std::string& path_to_requested_resource, std::string& dest)  ;
    const std::string   GetMIMEDataType(const std::string& content_type)                                    ;

    // Write to client
    int WriteToClient(int& client_socket, const std::string& httpResponse);

    // Interact function to be binded
    int         InteractFn(int client_socket)       ;
    static int  StaticInteractFn(int client_socket) ;
    static std::function<int(int)> interact_callback;

public:
    HttpServer( unsigned int server_port        ,
                unsigned int max_clients_num    ,
                bool concurrency_enabled        ,
                unsigned long rx_timeout        ,
                bool secure_connection          ,
                std::string path_cert           ,
                std::string path_pkey           ,
                std::string path_to_resources   );
    virtual ~HttpServer();

    // Constructor with no parameters as well as copy constructor will not be allowed
    // as undefined/repeated parameters can lead to potential conflicts during runtime.
    HttpServer() = delete                       ;
    HttpServer(const HttpServer& obj) = delete  ;
    
    int Run();
};

/*************************************/

#endif