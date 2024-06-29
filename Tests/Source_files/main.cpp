/************************************/
/******** Include statements ********/
/************************************/

#include "ServerSocket_api.h"
#include "GetOptions_api.h"
#include "SeverityLog_api.h"
#include "HttpServer_api.hpp"
#include <cstdlib>

/************************************/

/***************************************/
/********** Private constants **********/
/***************************************/

/************ Port settings ************/

#define PORT_OPT_CHAR                       'r'
#define PORT_OPT_LONG                       "Port_range"
#define PORT_OPT_DETAIL                     "Range of acceptable port numbers."
#define PORT_MIN_VALUE                      49152
#define PORT_MAX_VALUE                      65535
#define PORT_DEFAULT_VALUE                  50000

/********* Connection settings *********/

#define CLIENTS_OPT_CHAR                    'm'
#define CLIENTS_OPT_LONG                    "Clients"
#define CLIENTS_OPT_DETAIL                  "Maximum number of clients."
#define CLIENTS_MIN_VALUE                   1
#define CLIENTS_MAX_VALUE                   100
#define CLIENTS_DEFAULT_VALUE               3

/********* Enable concurrency *********/

#define SIMULTANEOUS_CONNS_CHAR             'p'
#define SIMULTANEOUS_CONNS_LONG             "Parallel"
#define SIMULTANEOUS_CONNS_DETAIL           "Enable concurrency."
#define SIMULTANEOUS_CONNS_DEFAULT_VALUE    false

/********** Receive timeout (s) ******/
#define RX_TIMEOUT_SECS_CHAR                't'
#define RX_TIMEOUT_SECS_OPT_LONG            "RXTimeoutSecs"
#define RX_TIMEOUT_SECS_OPT_DETAIL          "Receive Timeout in seconds."
#define RX_TIMEOUT_SECS_MIN_VALUE           0
#define RX_TIMEOUT_SECS_MAX_VALUE           3600    // 1 hour
#define RX_TIMEOUT_SECS_DEFAULT_VALUE       600     // 10 minutes

/********** Receive timeout (us) *****/
#define RX_TIMEOUT_USECS_CHAR               'u'
#define RX_TIMEOUT_USECS_OPT_LONG           "RXTimeoutUsecs"
#define RX_TIMEOUT_USECS_OPT_DETAIL         "Receive Timeout in microseconds."
#define RX_TIMEOUT_USECS_MIN_VALUE          0
#define RX_TIMEOUT_USECS_MAX_VALUE          1000000 // 1 second
#define RX_TIMEOUT_USECS_DEFAULT_VALUE      0

/********** Send timeout (s) ******/
#define TX_TIMEOUT_SECS_CHAR                'y'
#define TX_TIMEOUT_SECS_OPT_LONG            "TXTimeoutSecs"
#define TX_TIMEOUT_SECS_OPT_DETAIL          "Send Timeout in seconds."
#define TX_TIMEOUT_SECS_MIN_VALUE           0
#define TX_TIMEOUT_SECS_MAX_VALUE           3600    // 1 hour
#define TX_TIMEOUT_SECS_DEFAULT_VALUE       600     // 10 minutes

/********** Send timeout (us) *****/
#define TX_TIMEOUT_USECS_CHAR               'i'
#define TX_TIMEOUT_USECS_OPT_LONG           "TXTimeoutUsecs"
#define TX_TIMEOUT_USECS_OPT_DETAIL         "Send Timeout in microseconds."
#define TX_TIMEOUT_USECS_MIN_VALUE          0
#define TX_TIMEOUT_USECS_MAX_VALUE          1000000 // 1 second
#define TX_TIMEOUT_USECS_DEFAULT_VALUE      0

/********* Secure connection *********/

#define SECURE_CONN_CHAR                    's'
#define SECURE_CONN_LONG                    "Secure"
#define SECURE_CONN_DETAIL                  "Secure connection."
#define SECURE_CONN_DEFAULT_VALUE           false

/********* Certificate and private key path *********/

// Certificate as well as private keys should not be stored within dependency files directory
// (as it may lead to malicious activity if any execution permission is accepted).
/************ Server certificate ************/

#define CERT_OPT_CHAR                       'c'
#define CERT_OPT_LONG                       "Certificate"
#define CERT_OPT_DETAIL                     "Server certificate."
#define CERT_DEFAULT_VALUE                  "~/Desktop/scripts/certificate_test/certificate.crt"

/************ Server private key ************/

#define PKEY_OPT_CHAR                       'k'
#define PKEY_OPT_LONG                       "Key"
#define PKEY_OPT_DETAIL                     "Server private key."
#define PKEY_DEFAULT_VALUE                  "~/Desktop/scripts/certificate_test/private.key"

/***************************************/

/************ Web resources path ************/

#define RESOURCES_OPT_CHAR                  'e'
#define RESOURCES_OPT_LONG                  "Resources"
#define RESOURCES_OPT_DETAIL                "Path to web resources"
#define RESOURCES_DEFAULT_VALUE             "~/Desktop/scripts/C++/CPP_HTTP_Server/Tests/Dependency_files/Data/HTML_tutorial"

/***************************************/

/*
@brief Main function. Program's entry point.
*/
int main(int argc, char** argv)
{
    SetSeverityLogMask(SVRTY_LOG_MASK_ALL);
    SetSeverityLogPrintTimeStatus(true);

    int server_port         ;
    int max_clients_num     ;
    bool concurrency_enabled;
    int rx_timeout_s        ;
    int rx_timeout_us       ;
    int tx_timeout          ;
    int tx_timeout_us       ;
    bool secure_connection  ;
    char* path_cert = (char*)calloc(1024, 1);
    char* path_pkey = (char*)calloc(1024, 1);
    char* path_to_resources = (char*)calloc(1024, 1);

    SetOptionDefinitionInt(     PORT_OPT_CHAR                   ,
                                PORT_OPT_LONG                   ,
                                PORT_OPT_DETAIL                 ,
                                PORT_MIN_VALUE                  ,
                                PORT_MAX_VALUE                  ,
                                PORT_DEFAULT_VALUE              ,
                                &server_port                    );

    SetOptionDefinitionInt(     CLIENTS_OPT_CHAR                ,
                                CLIENTS_OPT_LONG                ,
                                CLIENTS_OPT_DETAIL              ,
                                CLIENTS_MIN_VALUE               ,
                                CLIENTS_MAX_VALUE               ,
                                CLIENTS_DEFAULT_VALUE           ,
                                &max_clients_num                );

    SetOptionDefinitionBool(    SIMULTANEOUS_CONNS_CHAR         ,
                                SIMULTANEOUS_CONNS_LONG         ,
                                SIMULTANEOUS_CONNS_DETAIL       ,
                                SIMULTANEOUS_CONNS_DEFAULT_VALUE,
                                &concurrency_enabled            );

    SetOptionDefinitionInt(     RX_TIMEOUT_SECS_CHAR            ,
                                RX_TIMEOUT_SECS_OPT_LONG        ,
                                RX_TIMEOUT_SECS_OPT_DETAIL      ,
                                RX_TIMEOUT_SECS_MIN_VALUE       ,
                                RX_TIMEOUT_SECS_MAX_VALUE       ,
                                RX_TIMEOUT_SECS_DEFAULT_VALUE   ,
                                &rx_timeout_s                   );

    SetOptionDefinitionInt(     RX_TIMEOUT_USECS_CHAR           ,
                                RX_TIMEOUT_USECS_OPT_LONG       ,
                                RX_TIMEOUT_USECS_OPT_DETAIL     ,
                                RX_TIMEOUT_USECS_MIN_VALUE      ,
                                RX_TIMEOUT_USECS_MAX_VALUE      ,
                                RX_TIMEOUT_USECS_DEFAULT_VALUE  ,
                                &rx_timeout_us                  );

    SetOptionDefinitionInt(     TX_TIMEOUT_SECS_CHAR            ,
                                TX_TIMEOUT_SECS_OPT_LONG        ,
                                TX_TIMEOUT_SECS_OPT_DETAIL      ,
                                TX_TIMEOUT_SECS_MIN_VALUE       ,
                                TX_TIMEOUT_SECS_MAX_VALUE       ,
                                TX_TIMEOUT_SECS_DEFAULT_VALUE   ,
                                &tx_timeout                     );

    SetOptionDefinitionInt(     TX_TIMEOUT_USECS_CHAR           ,
                                TX_TIMEOUT_USECS_OPT_LONG       ,
                                TX_TIMEOUT_USECS_OPT_DETAIL     ,
                                TX_TIMEOUT_USECS_MIN_VALUE      ,
                                TX_TIMEOUT_USECS_MAX_VALUE      ,
                                TX_TIMEOUT_USECS_DEFAULT_VALUE  ,
                                &tx_timeout_us                  );

    SetOptionDefinitionBool(    SECURE_CONN_CHAR                ,
                                SECURE_CONN_LONG                ,
                                SECURE_CONN_DETAIL              ,
                                SECURE_CONN_DEFAULT_VALUE       ,
                                &secure_connection              );

    SetOptionDefinitionStringNL(CERT_OPT_CHAR                   ,
                                CERT_OPT_LONG                   ,
                                CERT_OPT_DETAIL                 ,
                                CERT_DEFAULT_VALUE              ,
                                path_cert                       );

    SetOptionDefinitionStringNL(PKEY_OPT_CHAR                   ,
                                PKEY_OPT_LONG                   ,
                                PKEY_OPT_DETAIL                 ,
                                PKEY_DEFAULT_VALUE              ,
                                path_pkey                       );

    SetOptionDefinitionStringNL(RESOURCES_OPT_CHAR              ,
                                RESOURCES_OPT_LONG              ,
                                RESOURCES_OPT_DETAIL            ,
                                RESOURCES_DEFAULT_VALUE         ,
                                path_to_resources               );

    int parse_arguments = ParseOptions(argc, argv);
    if(parse_arguments < 0)
    {
        LOG_ERR("Arguments parsing failed!");
        return parse_arguments;
    }

    LOG_INF("Arguments successfully parsed!");

    HttpInteract::SetPathToResources(path_to_resources);

    ServerSocketRun(server_port             ,
                    max_clients_num         ,
                    concurrency_enabled     ,
                    false                   ,
                    true                    ,
                    true                    ,
                    rx_timeout_s            ,
                    rx_timeout_us           ,
                    tx_timeout              ,
                    tx_timeout_us           ,
                    secure_connection       ,
                    path_cert               ,
                    path_pkey               ,
                    HttpInteract::InteractFn);

    return 0;
}
