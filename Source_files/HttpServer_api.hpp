#ifndef CPP_HTTP_SERVER_API_HPP
#define CPP_HTTP_SERVER_API_HPP

/************************************/
/******** Include statements ********/
/************************************/

/*************************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

/// @brief Set the path to resources to be provided by the web server.
/// @param path Path to resources, established by the user.
void HttpSetPathToResources(char* path);

/// @brief Reads from client, then sends a response.
/// @param client_socket Client socket.
/// @return < 0 if any error happened, 0 otherwise.
int HttpServerInteractFn(int client_socket);

/*************************************/

#endif