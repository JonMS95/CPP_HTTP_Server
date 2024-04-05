#ifndef CPP_HTTP_SERVER
#define CPP_HTTP_SERVER

/************************************/
/******** Include statements ********/
/************************************/

#include <stdbool.h>
#include <openssl/ssl.h>

/*************************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

/// @brief Set the path to resources to be provided by the web server.
/// @param path Path to resources, established by the user.
void HttpSetPathToResources(char* path);

/// @brief Reads from client, then sends a response.
/// @param client_socket Client socket.
/// @param secure True if TLS security is wanted, false otherwise.
/// @param ssl SSL data.
/// @return < 0 if any error happened, 0 otherwise.
int HttpServerDefaultResponse(int client_socket, bool secure, SSL** ssl);

/*************************************/

#endif