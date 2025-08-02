#ifndef CPP_HTTP_SERVER_HPP
#define CPP_HTTP_SERVER_HPP

/************************************/
/******** Include statements ********/
/************************************/

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <MutexGuard_api.h>

/*************************************/

/************************************/
/********* Define statements ********/
/************************************/

#define HTTP_SERVER_LEN_RX_BUFFER                   8192        // RX buffer size.
#define HTTP_SERVER_LEN_TX_BUFFER                   8192        // TX buffer size.
#define HTTP_SERVER_HTTP_MSG_END                    "\r\n\r\n"
#define HTTP_SERVER_DEFAULT_PAGE                    "/index.html"
#define HTTP_SERVER_DEFAULT_ERROR_404_PAGE_PATH     "/page_not_found.html"
#define HTTP_SERVER_DEFAULT_ERROR_404_PAGE          "<!DOCTYPE html>\n" \
                                                    "<html lang=\"en\">\n" \
                                                    "<head>\n" \
                                                    "    <meta charset=\"UTF-8\">\n" \
                                                    "    <title>404 Not Found</title>\n" \
                                                    "</head>\n" \
                                                    "<body>\n" \
                                                    "    <h1>404 - Page Not Found</h1>\n" \
                                                    "    <p>Sorry, the page you are looking for does not exist.</p>\n" \
                                                    "    <p><a href=\"/\">Return to Homepage</a></p>\n" \
                                                    "</body>\n" \
                                                    "</html>"        

#define HTTP_SERVER_MSG_DATA_READ_FROM_CLIENT       "Data read from client: <\r\n%s\r\n>"
#define HTTP_SERVER_MSG_DATA_WRITTEN_TO_CLIENT      "Data written to client: <\r\n%s\r\n>"
#define HTTP_SERVER_MSG_CLIENT_DISCONNECTED         "Client with IP <%s> disconnected."
#define HTTP_SERVER_MSG_ECONNABORTED                "Connection aborted by peer, errno: %d"
#define HTTP_SERVER_MSG_ECONNRESET                  "Connection reset by peer, errno: %d"
#define HTTP_SERVER_MSG_ECONNREFUSED                "Connection refused by peer, errno: %d"
#define HTTP_SERVER_MSG_ERROR_WHILE_READING         "ERROR WHILE READING, errno: %d"
#define HTTP_SERVER_MSG_READ_TMT_EXPIRED            "READ TIMEOUT EXPIRED!"
#define HTTP_SERVER_MSG_UNKNOWN_RQST_FIELD          "Unknown field: %s"
#define HTTP_SERVER_MSG_OPENING_FILE                "Error opening file \"%s\"."
#define HTTP_SERVER_MSG_UNKNOWN_CONTENT_TYPE        "UNKNOWN CONTENT TYPE (File extension: %s)"
#define HTTP_SERVER_MSG_RQST_METHOD                 "METHOD:    %s"
#define HTTP_SERVER_MSG_RQST_RESOURCE               "RESOURCE:  %s"
#define HTTP_SERVER_MSG_RQST_PROTOCOL               "PROTOCOL:  %s"
#define HTTP_SERVER_MSG_BASIC_RQST_FIELD_MISSING    "One of the basic request fields (either method, requested resource or protocol) is missing."
#define HTTP_SERVER_MSG_PARTIAL_WRITE               "Partial write detected. Already written: %d. Remaining bytes amount: %d."
#define HTTP_SERVER_MSG_UNSUPPORTED_METHOD          "%s method is unsupported by the server."

#define HTTP_SERVER_ERR_BASIC_RQST_FIELDS_FAILED    -1
#define HTTP_SERVER_ERR_REQUESTED_FILE_NOT_FOUND    -2

#define HTTP_SERVER_METHOD_CODE_GET     0
#define HTTP_SERVER_METHOD_CODE_HEAD    1
#define HTTP_SERVER_METHOD_CODE_POST    2
#define HTTP_SERVER_METHOD_CODE_PUT     3
#define HTTP_SERVER_METHOD_CODE_DELETE  4
#define HTTP_SERVER_METHOD_CODE_CONNECT 5
#define HTTP_SERVER_METHOD_CODE_OPTIONS 6
#define HTTP_SERVER_METHOD_CODE_TRACE   7

#define HTTP_SERVER_STATUS_CODE_200     "200 OK"
#define HTTP_SERVER_STATUS_CODE_404     "404 Not found"
#define HTTP_SERVER_STATUS_CODE_405     "405 Method Not Allowed"

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
    HTTP_GEN_RESP_FSM_CHECK_REQUEST_METHOD                  = 0 ,
    HTTP_GEN_RESP_FSM_GET_PATH_TO_RESOURCE                      ,
    HTTP_GEN_RESP_FSM_CHECK_RESOURCE_EXTENSION                  ,
    HTTP_GEN_RESP_FSM_GET_REQUESTED_RESOURCE_SIZE               ,
    HTTP_GEN_RESP_FSM_BUILD_RESPONSE_HEADER                     ,
    HTTP_GEN_RESP_FSM_BUILD_ADD_RESOURCE                        ,
    HTTP_GEN_RESP_FSM_BUILD_TRACE_RESPONSE                      ,
    HTTP_GEN_RESP_FSM_BUILD_SERVER_UNSUPPORTED_METHOD_RESPONSE  ,
    HTTP_GEN_RESP_FSM_END_GEN_RESP                              ,
} HTTP_GEN_RESP_FSM;

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
    const std::string path_to_resources;

    using ext_to_type_table     = const std::map<const std::string, const std::string>  ;
    using method_to_uint_table  = const std::map<const std::string, const unsigned int> ;

    const std::shared_ptr<ext_to_type_table> ptr_extension_to_content   ;
    const std::shared_ptr<method_to_uint_table> ptr_method_to_uint      ;
    const std::shared_ptr<MTX_GRD> ptr_to_resources_read_mutex          ;

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
        {"Content-Length"               , ""},
        {"Referer"                      , ""},
        {"Accept-Encoding"              , ""},
        {"Accept-Language"              , ""},
        {"Purpose"                      , ""},
        {"Sec-Purpose"                  , ""},
        {"sec-ch-ua"                    , ""},
        {"sec-ch-ua-mobile"             , ""},
        {"sec-ch-ua-platform"           , ""},
        {"Sec-Fetch-Site"               , ""},
        {"Sec-Fetch-Mode"               , ""},
        {"Sec-Fetch-User"               , ""},
        {"Sec-Fetch-Dest"               , ""},
    };

    bool resource_not_found;

    std::string read_from_client            ;
    std::string http_response               ;
    std::string http_response_status_code   ;

    // Check resource sharing mutex status
    void InitResourcesMutex(void);

    // Read data from client
    int ReadFromClient(int& client_socket);
    // Used by ReadFromClient
    bool CheckRequestEnd(void);
    
    // Process request
    int ProcessRequest(void);
    // Used by ProcessRequest
    std::vector<std::string> ExtractWordsFromReqLine(const std::string& input);

    // Generate response for client
    long int GenerateResponse(void);
    // Used by GenerateResponse
    const std::string   GetPathToRequestedResource(void)                                                    ;
    const std::string   GetPathToResources(void)                                                            ;
    bool                FileExists(const std::string& filePath)                                             ;
    std::string         ParseFileExtension(const std::string& text)                                         ;
    long int            GetRequestedResourceSize(const std::string& resource_to_send)                       ;
    int                 CopyFileToString(const std::string& path_to_requested_resource, std::string& dest)  ;
    const std::string   GetMIMEDataType(const std::string& content_type)                                    ;

    // Write to client
    int WriteToClient(int& client_socket);

public:
    HttpServer(const std::string path_to_resources) ;
    virtual ~HttpServer(void)                       ;

    // Copy constructor will not be allowed as undefined/repeated parameters can lead to potential
    // conflicts during runtime.
    HttpServer(const HttpServer& obj) = delete;

    // Run Http Server FSM
    int Run(int& client_socket);
};

/************************************/

#endif